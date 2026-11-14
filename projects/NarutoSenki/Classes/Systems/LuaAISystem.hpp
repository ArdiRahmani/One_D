#pragma once
#include "Core/Hero.hpp"
#include "Utils/Cocos2dxHelper.hpp"

#include <string>
#include <unordered_map>
#include <unordered_set>

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "tolua++.h"
}

/**
 * Jembatan dua arah untuk AI hero yang ditulis di Lua  (C++ -> Lua  +  registry).
 *
 * Bagian Lua -> C++ (memanggil fungsi/field C++ dari Lua) ada di Systems/HeroBinding.hpp.
 * File ini HANYA mengurus: "siapa yang menangani virtual method hero ini di Lua".
 *
 * Tabel global `LuaAI` (diisi dari lua/Hook/AI.lua):
 *   LuaAI.set(ai, handlers)  -> boolean  (handlers = {perform=fn, changeAction=fn, ...})
 *   LuaAI.set(ai, nil)       -> boolean  (hapus, hero kembali memakai class C++ asli)
 *   LuaAI.remove(ai)         -> boolean
 *   LuaAI.clear()
 *   LuaAI.get(ai)            -> table|nil
 *   LuaAI.has(ai)            -> boolean
 *   LuaAI.hasMethod(ai, fn)  -> boolean
 *   LuaAI.count()            -> integer
 *
 * `ai` = nama class AI native (sama dengan yang dipakai Provider.hpp / CloneAI), mis. "Tsunade".
 *
 * Method yang dikenal (semuanya opsional, signature di sisi Lua):
 *   perform(self)                -> Hero::perform()
 *   changeAction(self)           -> CharacterBase::changeAction()
 *   resumeAction(self, dt)       -> CharacterBase::resumeAction(float)
 *   createClone(self, cloneTime) -> CharacterBase::createClone(int)   (return hero | nil)
 * `self` adalah userdata tolua bertipe "CharacterBase".
 *
 * Dipanggil sekali dari Internal::initAllSystems(), sebelum main.lua dijalankan.
 */
namespace LuaAISystem
{
	inline std::unordered_map<std::string, int> &refs()
	{
		static std::unordered_map<std::string, int> map; // ai -> luaL_ref di LUA_REGISTRYINDEX
		return map;
	}

	inline bool has(const std::string &ai)
	{
		return refs().find(ai) != refs().end();
	}

	// ---------------------------------------------------------------- C++ -> Lua

	inline int onError(lua_State *L)
	{
		const char *msg = lua_tostring(L, 1);
		lua_getglobal(L, "debug");
		if (!lua_istable(L, -1))
		{
			lua_pop(L, 1);
			lua_pushstring(L, msg ? msg : "<non-string error>");
			return 1;
		}
		lua_getfield(L, -1, "traceback");
		lua_remove(L, -2);
		lua_pushstring(L, msg ? msg : "<non-string error>");
		lua_pushinteger(L, 2);
		lua_call(L, 2, 1);
		return 1;
	}

	// Push ai.method ke stack. false (stack tidak berubah) kalau tidak ada / bukan function.
	inline bool pushMethod(lua_State *L, const std::string &ai, const char *method)
	{
		auto it = refs().find(ai);
		if (it == refs().end())
			return false;

		lua_rawgeti(L, LUA_REGISTRYINDEX, it->second);
		if (!lua_istable(L, -1))
		{
			lua_pop(L, 1);
			return false;
		}
		lua_getfield(L, -1, method);
		if (!lua_isfunction(L, -1))
		{
			lua_pop(L, 2);
			return false;
		}
		lua_remove(L, -2); // buang tabel handler
		return true;
	}

	inline void reportOnce(const std::string &ai, const char *method, const char *what, const char *detail)
	{
		static std::unordered_set<std::string> reported;
		if (reported.insert(ai + "." + method + "/" + what).second)
		{
			CCLOG("[LuaAI] %s.%s %s%s%s", ai.c_str(), method, what, detail ? ": " : "", detail ? detail : "");
		}
	}

	/**
	 * Panggil ai.method(self [, num]).
	 * @param nres  0 = tanpa return, 1 = ambil 1 return (dikembalikan lewat `outHero` bila tidak null).
	 * @return true kalau fungsi Lua ada DAN berjalan tanpa error.
	 *         Error Lua tidak pernah merambat ke engine: dicatat sekali per method.
	 */
	inline bool call(const std::string &ai, const char *method, CharacterBase *self,
					 bool hasNum, double num, Hero **outHero = nullptr)
	{
		if (!LuaBridge::isReady() || !self || !has(ai))
			return false;

		lua_getL;
		const int base = lua_gettop(L);

		lua_pushcfunction(L, onError);
		const int errFunc = lua_gettop(L);

		if (!pushMethod(L, ai, method))
		{
			lua_settop(L, base);
			reportOnce(ai, method, "tidak ada di tabel handler Lua", nullptr);
			return false;
		}

		get_luastack->pushCCObject(static_cast<CCObject *>(self), "CharacterBase");
		int nargs = 1;
		if (hasNum)
		{
			lua_pushnumber(L, num);
			nargs++;
		}

		const int nres = outHero ? 1 : 0;
		if (lua_pcall(L, nargs, nres, errFunc) != 0)
		{
			reportOnce(ai, method, "error", lua_tostring(L, -1));
			lua_settop(L, base);
			return false;
		}

		if (outHero)
		{
			*outHero = nullptr;
			tolua_Error err;
			if (!lua_isnil(L, -1) && tolua_isusertype(L, -1, "CharacterBase", 0, &err))
			{
				CCObject *obj = static_cast<CCObject *>(tolua_tousertype(L, -1, nullptr));
				*outHero = dynamic_cast<Hero *>(obj);
			}
		}

		lua_settop(L, base);
		return true;
	}

	inline bool callVoid(const std::string &ai, const char *method, CharacterBase *self)
	{
		return call(ai, method, self, false, 0);
	}

	inline bool callVoid(const std::string &ai, const char *method, CharacterBase *self, double num)
	{
		return call(ai, method, self, true, num);
	}

	// Untuk createClone: return nullptr kalau Lua tidak ada / error / return bukan hero.
	inline Hero *callHero(const std::string &ai, const char *method, CharacterBase *self, double num)
	{
		Hero *result = nullptr;
		call(ai, method, self, true, num, &result);
		return result;
	}

	// ---------------------------------------------------------------- Lua -> registry

	static void unrefAi(lua_State *L, const std::string &ai)
	{
		auto it = refs().find(ai);
		if (it != refs().end())
		{
			luaL_unref(L, LUA_REGISTRYINDEX, it->second);
			refs().erase(it);
		}
	}

	static int aiSet(lua_State *L)
	{
		const char *ai = luaL_checkstring(L, 1);
		if (lua_isnoneornil(L, 2))
		{
			unrefAi(L, ai);
			lua_pushboolean(L, 1);
			return 1;
		}
		luaL_checktype(L, 2, LUA_TTABLE);
		unrefAi(L, ai);
		lua_pushvalue(L, 2);
		refs()[ai] = luaL_ref(L, LUA_REGISTRYINDEX);
		lua_pushboolean(L, 1);
		return 1;
	}

	static int aiRemove(lua_State *L)
	{
		const std::string ai = luaL_checkstring(L, 1);
		const bool existed = has(ai);
		unrefAi(L, ai);
		lua_pushboolean(L, existed ? 1 : 0);
		return 1;
	}

	static int aiClear(lua_State *L)
	{
		for (auto &kv : refs())
			luaL_unref(L, LUA_REGISTRYINDEX, kv.second);
		refs().clear();
		return 0;
	}

	static int aiGet(lua_State *L)
	{
		auto it = refs().find(luaL_checkstring(L, 1));
		if (it == refs().end())
			lua_pushnil(L);
		else
			lua_rawgeti(L, LUA_REGISTRYINDEX, it->second);
		return 1;
	}

	static int aiHas(lua_State *L)
	{
		lua_pushboolean(L, has(luaL_checkstring(L, 1)) ? 1 : 0);
		return 1;
	}

	static int aiHasMethod(lua_State *L)
	{
		const std::string ai = luaL_checkstring(L, 1);
		const char *method = luaL_checkstring(L, 2);
		const bool ok = pushMethod(L, ai, method);
		if (ok)
			lua_pop(L, 1);
		lua_pushboolean(L, ok ? 1 : 0);
		return 1;
	}

	static int aiCount(lua_State *L)
	{
		lua_pushinteger(L, (lua_Integer)refs().size());
		return 1;
	}

	static void registerLua()
	{
		if (!LuaBridge::isReady())
		{
			CCLOG("[LuaAISystem] Lua engine not ready, LuaAI not registered");
			return;
		}

		lua_getL;
		refs().clear(); // referensi lama milik lua_State sebelumnya tidak berlaku lagi

		static const struct
		{
			const char *name;
			lua_CFunction func;
		} kFuncs[] = {
			{"set", aiSet},
			{"remove", aiRemove},
			{"clear", aiClear},
			{"get", aiGet},
			{"has", aiHas},
			{"hasMethod", aiHasMethod},
			{"count", aiCount},
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
