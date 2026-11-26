#pragma once
#include "Data/UnitDefines.h"
#include "Utils/Cocos2dxHelper.hpp"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

/**
 * Jembatan Lua -> C++ generik untuk sebuah AliasMap.
 *
 * Mendaftarkan sebuah tabel global (mis. `CloneBullet`, `CloneCatch`) ke Lua dengan API
 * yang sama seperti CloneAI / CloneMonAI:
 *   T.set(name, alias)   -> boolean  (name memakai alias)
 *   T.set(name, nil)     -> boolean  (hapus alias, sama dengan T.remove)
 *   T.setAll(tbl)        -> integer  ({Sanpu = 'TentenSRK', ...}; return jumlah yang masuk)
 *   T.remove(name)       -> boolean
 *   T.clear()
 *   T.get(name)          -> string|nil  (alias, nil kalau tidak ada)
 *   T.resolve(name)      -> string      (alias efektif; = name kalau tidak ada alias)
 *   T.has(name)          -> boolean
 *   T.count()            -> integer
 *
 * Pemakaian:  AliasBridge<BulletRegistry::map>::registerLua("CloneBullet");
 */
template <AliasMap &(*Get)()>
struct AliasBridge
{
	static int aliasSet(lua_State *L)
	{
		const char *name = luaL_checkstring(L, 1);
		if (lua_isnoneornil(L, 2))
		{
			Get().remove(name);
			lua_pushboolean(L, 1);
			return 1;
		}
		const char *alias = luaL_checkstring(L, 2);
		lua_pushboolean(L, Get().set(name, alias) ? 1 : 0);
		return 1;
	}

	static int aliasSetAll(lua_State *L)
	{
		luaL_checktype(L, 1, LUA_TTABLE);
		int added = 0;
		lua_pushnil(L);
		while (lua_next(L, 1) != 0)
		{
			// Hanya pasangan string -> string (jangan lua_tostring ke key angka: mengubah key asli)
			if (lua_type(L, -2) == LUA_TSTRING && lua_type(L, -1) == LUA_TSTRING)
			{
				if (Get().set(lua_tostring(L, -2), lua_tostring(L, -1)))
					added++;
			}
			lua_pop(L, 1);
		}
		lua_pushinteger(L, (lua_Integer)added);
		return 1;
	}

	static int aliasRemove(lua_State *L)
	{
		lua_pushboolean(L, Get().remove(luaL_checkstring(L, 1)) ? 1 : 0);
		return 1;
	}

	static int aliasClear(lua_State *)
	{
		Get().clear();
		return 0;
	}

	static int aliasGet(lua_State *L)
	{
		const std::string name = luaL_checkstring(L, 1);
		if (Get().has(name))
			lua_pushstring(L, Get().resolve(name).c_str());
		else
			lua_pushnil(L);
		return 1;
	}

	static int aliasResolve(lua_State *L)
	{
		const std::string name = luaL_checkstring(L, 1);
		lua_pushstring(L, Get().resolve(name).c_str());
		return 1;
	}

	static int aliasHas(lua_State *L)
	{
		lua_pushboolean(L, Get().has(luaL_checkstring(L, 1)) ? 1 : 0);
		return 1;
	}

	static int aliasCount(lua_State *L)
	{
		lua_pushinteger(L, (lua_Integer)Get().count());
		return 1;
	}

	static void registerLua(const char *globalName)
	{
		if (!LuaBridge::isReady())
		{
			CCLOG("[AliasBridge] Lua engine not ready, %s not registered", globalName);
			return;
		}

		lua_getL;
		static const struct
		{
			const char *name;
			lua_CFunction func;
		} kFuncs[] = {
			{"set", aliasSet},
			{"setAll", aliasSetAll},
			{"remove", aliasRemove},
			{"clear", aliasClear},
			{"get", aliasGet},
			{"resolve", aliasResolve},
			{"has", aliasHas},
			{"count", aliasCount},
		};

		lua_newtable(L);
		for (const auto &f : kFuncs)
		{
			lua_pushcfunction(L, f.func);
			lua_setfield(L, -2, f.name);
		}
		lua_setglobal(L, globalName);
	}
};
