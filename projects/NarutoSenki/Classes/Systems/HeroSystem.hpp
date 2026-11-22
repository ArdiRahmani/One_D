#pragma once
#include "Data/UnitDefines.h"
#include "Utils/Cocos2dxHelper.hpp"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

/**
 * Jembatan Lua -> C++ untuk daftar hero.
 *
 * Mendaftarkan tabel global `HeroRegistry` ke Lua:
 *   HeroRegistry.add(name)    -> boolean  (true kalau baru ditambahkan)
 *   HeroRegistry.set(tbl)     -> integer  (ganti seluruh daftar, return jumlah)
 *   HeroRegistry.clear()
 *   HeroRegistry.count()      -> integer
 *   HeroRegistry.get(i)       -> string|nil  (index mulai dari 1)
 *   HeroRegistry.has(name)    -> boolean
 *
 * Dipanggil sekali dari Internal::initAllSystems(), sebelum main.lua dijalankan.
 */
namespace HeroSystem
{
	static int heroAdd(lua_State *L)
	{
		const char *name = luaL_checkstring(L, 1);
		lua_pushboolean(L, HeroRegistry::add(name) ? 1 : 0);
		return 1;
	}

	static int heroSet(lua_State *L)
	{
		luaL_checktype(L, 1, LUA_TTABLE);
		HeroRegistry::clear();
		const int n = (int)lua_objlen(L, 1);
		for (int i = 1; i <= n; i++)
		{
			lua_rawgeti(L, 1, i);
			if (lua_isstring(L, -1))
				HeroRegistry::add(lua_tostring(L, -1));
			lua_pop(L, 1);
		}
		lua_pushinteger(L, (lua_Integer)HeroRegistry::count());
		return 1;
	}

	static int heroClear(lua_State *)
	{
		HeroRegistry::clear();
		return 0;
	}

	static int heroCount(lua_State *L)
	{
		lua_pushinteger(L, (lua_Integer)HeroRegistry::count());
		return 1;
	}

	static int heroGet(lua_State *L)
	{
		const lua_Integer index = luaL_checkinteger(L, 1);
		const char *name = index >= 1 ? HeroRegistry::at((size_t)(index - 1)) : nullptr;
		if (name)
			lua_pushstring(L, name);
		else
			lua_pushnil(L);
		return 1;
	}

	static int heroHas(lua_State *L)
	{
		lua_pushboolean(L, HeroRegistry::contains(luaL_checkstring(L, 1)) ? 1 : 0);
		return 1;
	}

	static void registerLua()
	{
		if (!LuaBridge::isReady())
		{
			CCLOG("[HeroSystem] Lua engine not ready, HeroRegistry not registered");
			return;
		}

		lua_getL;
		static const struct
		{
			const char *name;
			lua_CFunction func;
		} kFuncs[] = {
			{"add", heroAdd},
			{"set", heroSet},
			{"clear", heroClear},
			{"count", heroCount},
			{"get", heroGet},
			{"has", heroHas},
		};

		lua_newtable(L);
		for (const auto &f : kFuncs)
		{
			lua_pushcfunction(L, f.func);
			lua_setfield(L, -2, f.name);
		}
		lua_setglobal(L, "HeroRegistry");
	}
} // namespace HeroSystem
