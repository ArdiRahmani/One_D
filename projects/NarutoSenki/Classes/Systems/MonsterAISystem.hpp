#pragma once
#include "Data/UnitDefines.h"
#include "Utils/Cocos2dxHelper.hpp"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

/**
 * Jembatan Lua -> C++ untuk Monster::setAI (CloneMonAI: AI per-tick monster/summon).
 *
 * Mendaftarkan tabel global `CloneMonAI` ke Lua:
 *   CloneMonAI.set(name, ai)   -> boolean  (monster `name` memakai Monster::setAI milik `ai`)
 *   CloneMonAI.set(name, nil)  -> boolean  (hapus alias, sama dengan CloneMonAI.remove)
 *   CloneMonAI.setAll(tbl)     -> integer  ({Shakuton = 'Rocket', ...}; return jumlah yang masuk)
 *   CloneMonAI.remove(name)    -> boolean
 *   CloneMonAI.clear()
 *   CloneMonAI.get(name)       -> string|nil  (AI yang dipakai, nil kalau tidak ada alias)
 *   CloneMonAI.resolve(name)   -> string      (AI efektif; = name kalau tidak ada alias)
 *   CloneMonAI.has(name)       -> boolean
 *   CloneMonAI.count()         -> integer
 *
 * Contoh di Lua:
 *   CloneMonAI.set('Shakuton', 'Rocket')
 *   CloneMonAI.set('PaperBom', 'Mine')
 *   -- Monster "Shakuton" berperilaku seperti Rocket, "PaperBom" seperti Mine,
 *   -- tapi nama unit (sprite/data/resource) tetap Shakuton / PaperBom.
 *
 * Dipanggil sekali dari Internal::initAllSystems(), sebelum main.lua dijalankan.
 */
namespace MonsterAISystem
{
	static int aiSet(lua_State *L)
	{
		const char *name = luaL_checkstring(L, 1);
		if (lua_isnoneornil(L, 2))
		{
			MonAIRegistry::remove(name);
			lua_pushboolean(L, 1);
			return 1;
		}
		const char *ai = luaL_checkstring(L, 2);
		lua_pushboolean(L, MonAIRegistry::set(name, ai) ? 1 : 0);
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
				if (MonAIRegistry::set(lua_tostring(L, -2), lua_tostring(L, -1)))
					added++;
			}
			lua_pop(L, 1);
		}
		lua_pushinteger(L, (lua_Integer)added);
		return 1;
	}

	static int aiRemove(lua_State *L)
	{
		lua_pushboolean(L, MonAIRegistry::remove(luaL_checkstring(L, 1)) ? 1 : 0);
		return 1;
	}

	static int aiClear(lua_State *)
	{
		MonAIRegistry::clear();
		return 0;
	}

	static int aiGet(lua_State *L)
	{
		const char *name = luaL_checkstring(L, 1);
		if (MonAIRegistry::has(name))
			lua_pushstring(L, MonAIRegistry::resolve(name).c_str());
		else
			lua_pushnil(L);
		return 1;
	}

	static int aiResolve(lua_State *L)
	{
		const std::string name = luaL_checkstring(L, 1);
		lua_pushstring(L, MonAIRegistry::resolve(name).c_str());
		return 1;
	}

	static int aiHas(lua_State *L)
	{
		lua_pushboolean(L, MonAIRegistry::has(luaL_checkstring(L, 1)) ? 1 : 0);
		return 1;
	}

	static int aiCount(lua_State *L)
	{
		lua_pushinteger(L, (lua_Integer)MonAIRegistry::count());
		return 1;
	}

	static void registerLua()
	{
		if (!LuaBridge::isReady())
		{
			CCLOG("[MonsterAISystem] Lua engine not ready, CloneMonAI not registered");
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
			{"resolve", aiResolve},
			{"has", aiHas},
			{"count", aiCount},
		};

		lua_newtable(L);
		for (const auto &f : kFuncs)
		{
			lua_pushcfunction(L, f.func);
			lua_setfield(L, -2, f.name);
		}
		lua_setglobal(L, "CloneMonAI");
	}
} // namespace MonsterAISystem
