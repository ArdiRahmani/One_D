#pragma once
#include "Data/UnitDefines.h"
#include "Utils/Cocos2dxHelper.hpp"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

/**
 * Jembatan Lua -> C++ untuk AI karakter yang DITULIS di Lua (LuaAI).
 *
 * Beda dengan CloneAI (alias ke class C++ yang sudah ada), LuaAI menjalankan logika AI
 * dari script lua/AI/<Script>.lua lewat kelas LuaHero (Core/LuaHero.hpp).
 *
 * Mendaftarkan tabel global `LuaAI` ke Lua:
 *   LuaAI.set(unit, script)   -> boolean  (unit memakai lua/AI/<script>.lua)
 *   LuaAI.set(unit, nil)      -> boolean  (hapus binding, sama dengan LuaAI.remove)
 *   LuaAI.setAll(tbl)         -> integer  ({Yagura = 'Tsunade', ...}; return jumlah yang masuk)
 *   LuaAI.remove(unit)        -> boolean
 *   LuaAI.clear()
 *   LuaAI.get(unit)           -> string|nil  (nama script, nil kalau tidak ada)
 *   LuaAI.has(unit)           -> boolean
 *   LuaAI.count()             -> integer
 *   LuaAI.define(script, tbl) -> boolean  (daftarkan script langsung dari tabel, tanpa file)
 *   LuaAI.reload([script])    -> nil      (buang cache script; unit yang DIBUAT sesudahnya
 *                                          memakai versi baru. Tanpa argumen = semua script)
 *
 * Script AI (lua/AI/<Script>.lua) mengembalikan tabel berisi fungsi:
 *   perform(self)              WAJIB. Dipanggil tiap tick AI (setara Hero::perform()).
 *   changeAction(self)         opsional (setara override changeAction()).
 *   resumeAction(self, dt)     opsional (setara override resumeAction(dt)).
 *   createClone(self, time)    opsional, return unit hasil self:createSummon('Nama') atau nil.
 * `self` adalah proxy hero; API-nya ada di Core/LuaHero.hpp dan lua/AI/README.md.
 *
 * Harus dipanggil sebelum pertandingan dimulai (sebelum Provider::create dijalankan).
 * Dipanggil sekali dari Internal::initAllSystems(), sebelum main.lua dijalankan.
 */
namespace LuaAISystem
{
	// nama script -> registry ref tabel script (cache hasil require 'AI.<script>')
	inline std::unordered_map<std::string, int> &cache()
	{
		static std::unordered_map<std::string, int> map;
		return map;
	}

	inline bool isValidScriptName(const char *s)
	{
		if (!s || !*s)
			return false;
		for (const char *p = s; *p; ++p)
		{
			const char c = *p;
			if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '.'))
				return false;
		}
		return true;
	}

	inline void dropCache(lua_State *L, const std::string &script)
	{
		auto it = cache().find(script);
		if (it == cache().end())
			return;
		luaL_unref(L, LUA_REGISTRYINDEX, it->second);
		cache().erase(it);
	}

	// package.loaded['AI.<script>'] = nil
	inline void forgetModule(lua_State *L, const std::string &script)
	{
		lua_getglobal(L, "package");
		if (lua_istable(L, -1))
		{
			lua_getfield(L, -1, "loaded");
			if (lua_istable(L, -1))
			{
				lua_pushnil(L);
				lua_setfield(L, -2, ("AI." + script).c_str());
			}
			lua_pop(L, 1);
		}
		lua_pop(L, 1);
	}

	/**
	 * Taruh tabel script di puncak stack. Return false (stack bersih) kalau gagal dimuat.
	 * Script dimuat lazy: `require 'AI.<script>'` saat pertama kali ada unit yang memakainya.
	 */
	inline bool pushScript(lua_State *L, const std::string &script)
	{
		auto &c = cache();
		auto it = c.find(script);
		if (it != c.end())
		{
			lua_rawgeti(L, LUA_REGISTRYINDEX, it->second);
			return true;
		}

		lua_getglobal(L, "require");
		if (!lua_isfunction(L, -1))
		{
			lua_pop(L, 1);
			CCLOG("[LuaAI] fungsi require tidak ada, script [ %s ] tidak bisa dimuat", script.c_str());
			return false;
		}
		lua_pushstring(L, ("AI." + script).c_str());
		if (lua_pcall(L, 1, 1, 0) != LUA_OK)
		{
			const char *err = lua_tostring(L, -1);
			CCLOG("[LuaAI] gagal memuat lua/AI/%s.lua: %s", script.c_str(), err ? err : "<no error>");
			lua_pop(L, 1);
			return false;
		}
		if (!lua_istable(L, -1))
		{
			CCLOG("[LuaAI] lua/AI/%s.lua harus 'return { perform = function(self) ... end }'", script.c_str());
			lua_pop(L, 1);
			return false;
		}

		lua_getfield(L, -1, "perform");
		const bool hasPerform = lua_isfunction(L, -1);
		lua_pop(L, 1);
		if (!hasPerform)
		{
			CCLOG("[LuaAI] lua/AI/%s.lua tidak punya fungsi perform(self)", script.c_str());
			lua_pop(L, 1);
			return false;
		}

		lua_pushvalue(L, -1);
		c[script] = luaL_ref(L, LUA_REGISTRYINDEX);
		return true;
	}

	static int aiSet(lua_State *L)
	{
		const char *unit = luaL_checkstring(L, 1);
		if (lua_isnoneornil(L, 2))
		{
			LuaAIRegistry::remove(unit);
			lua_pushboolean(L, 1);
			return 1;
		}
		const char *script = luaL_checkstring(L, 2);
		if (!isValidScriptName(script))
		{
			CCLOG("[LuaAI] nama script tidak valid: %s", script);
			lua_pushboolean(L, 0);
			return 1;
		}
		lua_pushboolean(L, LuaAIRegistry::set(unit, script) ? 1 : 0);
		return 1;
	}

	static int aiSetAll(lua_State *L)
	{
		luaL_checktype(L, 1, LUA_TTABLE);
		int added = 0;
		lua_pushnil(L);
		while (lua_next(L, 1) != 0)
		{
			// Hanya pasangan string -> string (jangan lua_tostring ke key angka: mengubah key asli)
			if (lua_type(L, -2) == LUA_TSTRING && lua_type(L, -1) == LUA_TSTRING)
			{
				const char *script = lua_tostring(L, -1);
				if (isValidScriptName(script) && LuaAIRegistry::set(lua_tostring(L, -2), script))
					added++;
			}
			lua_pop(L, 1);
		}
		lua_pushinteger(L, (lua_Integer)added);
		return 1;
	}

	static int aiRemove(lua_State *L)
	{
		lua_pushboolean(L, LuaAIRegistry::remove(luaL_checkstring(L, 1)) ? 1 : 0);
		return 1;
	}

	static int aiClear(lua_State *)
	{
		LuaAIRegistry::clear();
		return 0;
	}

	static int aiGet(lua_State *L)
	{
		const std::string unit = luaL_checkstring(L, 1);
		if (LuaAIRegistry::has(unit))
			lua_pushstring(L, LuaAIRegistry::get(unit).c_str());
		else
			lua_pushnil(L);
		return 1;
	}

	static int aiHas(lua_State *L)
	{
		lua_pushboolean(L, LuaAIRegistry::has(luaL_checkstring(L, 1)) ? 1 : 0);
		return 1;
	}

	static int aiCount(lua_State *L)
	{
		lua_pushinteger(L, (lua_Integer)LuaAIRegistry::count());
		return 1;
	}

	// LuaAI.define('Nama', { perform = function(self) ... end })
	static int aiDefine(lua_State *L)
	{
		const char *script = luaL_checkstring(L, 1);
		luaL_checktype(L, 2, LUA_TTABLE);
		if (!isValidScriptName(script))
		{
			CCLOG("[LuaAI] nama script tidak valid: %s", script);
			lua_pushboolean(L, 0);
			return 1;
		}

		lua_getfield(L, 2, "perform");
		const bool hasPerform = lua_isfunction(L, -1);
		lua_pop(L, 1);
		if (!hasPerform)
		{
			CCLOG("[LuaAI] define('%s'): tabel harus punya fungsi perform(self)", script);
			lua_pushboolean(L, 0);
			return 1;
		}

		dropCache(L, script);
		lua_pushvalue(L, 2);
		cache()[script] = luaL_ref(L, LUA_REGISTRYINDEX);
		lua_pushboolean(L, 1);
		return 1;
	}

	// LuaAI.reload('Nama') / LuaAI.reload()
	static int aiReload(lua_State *L)
	{
		if (lua_isnoneornil(L, 1))
		{
			std::vector<std::string> names;
			for (const auto &kv : cache())
				names.push_back(kv.first);
			for (const auto &n : names)
			{
				dropCache(L, n);
				forgetModule(L, n);
			}
			return 0;
		}
		const std::string script = luaL_checkstring(L, 1);
		dropCache(L, script);
		forgetModule(L, script);
		return 0;
	}

	static void registerLua()
	{
		if (!LuaBridge::isReady())
		{
			CCLOG("[LuaAISystem] Lua engine not ready, LuaAI not registered");
			return;
		}

		lua_getL;
		static const struct
		{
			const char *name;
			lua_CFunction func;
		} kFuncs[] = {
			{"set", aiSet},
			{"setAll", aiSetAll},
			{"remove", aiRemove},
			{"clear", aiClear},
			{"get", aiGet},
			{"has", aiHas},
			{"count", aiCount},
			{"define", aiDefine},
			{"reload", aiReload},
		};

		lua_newtable(L);
		for (const auto &f : kFuncs)
		{
			lua_pushcfunction(L, f.func);
			lua_setfield(L, -2, f.name);
		}
		lua_setglobal(L, "LuaAI");
	}
} // namespace LuaAISystem
