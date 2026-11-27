#pragma once
#include "Data/UnitDefines.h"
#include "Utils/Cocos2dxHelper.hpp"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

/**
 * Jembatan Lua -> C++ untuk CharacterBase::setTransform (transformasi karakter).
 *
 * Mendaftarkan tabel global `Transform` ke Lua:
 *   Transform.set(name, to)   -> boolean  (karakter `name` berubah menjadi `to`)
 *   Transform.set(name, nil)  -> boolean  (hapus transformasi, sama dengan Transform.remove)
 *   Transform.setAll(tbl)     -> integer  ({Guy = 'Guy8Gates', ...}; return jumlah yang masuk)
 *   Transform.remove(name)    -> boolean
 *   Transform.clear()
 *   Transform.get(name)       -> string|nil  (hasil transformasi, nil kalau tidak terdaftar)
 *   Transform.has(name)       -> boolean
 *   Transform.count()         -> integer
 *
 * Contoh di Lua:
 *   Transform.set('Guy', 'Guy8Gates')
 *   -- setTransform() pada unit "Guy" => setID("Guy8Gates", _role, _group), setara dengan
 *   --   if (getName() == "Guy") setID("Guy8Gates", _role, _group);
 *
 * Harus dipanggil sebelum pertandingan dimulai (sebelum CharacterBase::setTransform dijalankan).
 * Dipanggil sekali dari Internal::initAllSystems(), sebelum main.lua dijalankan.
 */
namespace TransformSystem
{
	static int trSet(lua_State *L)
	{
		const char *name = luaL_checkstring(L, 1);
		if (lua_isnoneornil(L, 2))
		{
			TransformRegistry::remove(name);
			lua_pushboolean(L, 1);
			return 1;
		}
		const char *to = luaL_checkstring(L, 2);
		lua_pushboolean(L, TransformRegistry::set(name, to) ? 1 : 0);
		return 1;
	}

	static int trSetAll(lua_State *L)
	{
		luaL_checktype(L, 1, LUA_TTABLE);
		int added = 0;
		lua_pushnil(L);
		while (lua_next(L, 1) != 0)
		{
			// Hanya pasangan string -> string (jangan lua_tostring ke key angka: mengubah key asli)
			if (lua_type(L, -2) == LUA_TSTRING && lua_type(L, -1) == LUA_TSTRING)
			{
				if (TransformRegistry::set(lua_tostring(L, -2), lua_tostring(L, -1)))
					added++;
			}
			lua_pop(L, 1);
		}
		lua_pushinteger(L, (lua_Integer)added);
		return 1;
	}

	static int trRemove(lua_State *L)
	{
		lua_pushboolean(L, TransformRegistry::remove(luaL_checkstring(L, 1)) ? 1 : 0);
		return 1;
	}

	static int trClear(lua_State *)
	{
		TransformRegistry::clear();
		return 0;
	}

	static int trGet(lua_State *L)
	{
		const char *name = luaL_checkstring(L, 1);
		if (TransformRegistry::has(name))
			lua_pushstring(L, TransformRegistry::resolve(name).c_str());
		else
			lua_pushnil(L);
		return 1;
	}

	static int trHas(lua_State *L)
	{
		lua_pushboolean(L, TransformRegistry::has(luaL_checkstring(L, 1)) ? 1 : 0);
		return 1;
	}

	static int trCount(lua_State *L)
	{
		lua_pushinteger(L, (lua_Integer)TransformRegistry::count());
		return 1;
	}

	static void registerLua()
	{
		if (!LuaBridge::isReady())
		{
			CCLOG("[TransformSystem] Lua engine not ready, Transform not registered");
			return;
		}

		lua_getL;
		static const struct
		{
			const char *name;
			lua_CFunction func;
		} kFuncs[] = {
			{"set", trSet},
			{"setAll", trSetAll},
			{"remove", trRemove},
			{"clear", trClear},
			{"get", trGet},
			{"has", trHas},
			{"count", trCount},
		};

		lua_newtable(L);
		for (const auto &f : kFuncs)
		{
			lua_pushcfunction(L, f.func);
			lua_setfield(L, -2, f.name);
		}
		lua_setglobal(L, "Transform");
	}
} // namespace TransformSystem
