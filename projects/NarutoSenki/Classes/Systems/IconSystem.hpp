#pragma once
#include "Data/UnitDefines.h"
#include "Utils/Cocos2dxHelper.hpp"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

/**
 * Jembatan Lua -> C++ untuk binding ikon/gambar unit (IconRegistry).
 *
 * Mendaftarkan tabel global `Icons` ke Lua:
 *   Icons.setAll(tbl)              -> integer  ({rpf = {Guy = 'Guy8Gates'}, rp = {...}, ...})
 *   Icons.set(kind, spec)          -> integer  (ganti seluruh binding jenis `kind`)
 *   Icons.add(kind, source, unit)  -> boolean  (unit memakai gambar milik `source`)
 *   Icons.remove(kind, unit)       -> boolean
 *   Icons.clear([kind])
 *   Icons.resolve(kind, unit)      -> string   (hero sumber; = unit kalau tidak ada binding)
 *   Icons.frame(kind, unit)        -> string   (nama frame lengkap, mis. 'Guy_rpf.png')
 *   Icons.has(kind, unit)          -> boolean
 *   Icons.count()                  -> integer
 *
 * kind (tidak peka huruf besar/kecil): rp, rpf, CutIn, Avator, small
 * spec = { [hero_sumber] = 'Unit' atau {'UnitA', 'UnitB'} }
 *
 * Dipanggil sekali dari Internal::initAllSystems(), sebelum main.lua dijalankan.
 */
namespace IconSystem
{
	static bool readKind(lua_State *L, int idx, IconRegistry::Kind &out)
	{
		const char *key = luaL_checkstring(L, idx);
		if (IconRegistry::findKind(key, out))
			return true;
		CCLOG("[IconSystem] jenis gambar tidak dikenal: %s (rp, rpf, CutIn, Avator, small)", key);
		return false;
	}

	static inline int absIndex(lua_State *L, int idx)
	{
		return (idx > 0 || idx <= LUA_REGISTRYINDEX) ? idx : lua_gettop(L) + idx + 1;
	}

	// Isi satu jenis dari tabel spec di indeks `idx`. Return jumlah binding yang masuk.
	static int applySpec(lua_State *L, int idx, IconRegistry::Kind kind)
	{
		idx = absIndex(L, idx);
		int added = 0;
		lua_pushnil(L);
		while (lua_next(L, idx) != 0)
		{
			// key harus string (jangan lua_tostring ke key angka: mengubah key asli)
			if (lua_type(L, -2) == LUA_TSTRING)
			{
				std::string source = lua_tostring(L, -2);
				if (lua_type(L, -1) == LUA_TSTRING)
				{
					if (IconRegistry::add(kind, source, lua_tostring(L, -1)))
						added++;
				}
				else if (lua_type(L, -1) == LUA_TTABLE)
				{
					int tbl = lua_gettop(L);
					int n = (int)lua_objlen(L, tbl);
					for (int i = 1; i <= n; i++)
					{
						lua_rawgeti(L, tbl, i);
						if (lua_type(L, -1) == LUA_TSTRING && IconRegistry::add(kind, source, lua_tostring(L, -1)))
							added++;
						lua_pop(L, 1);
					}
				}
			}
			lua_pop(L, 1);
		}
		return added;
	}

	static int icSetAll(lua_State *L)
	{
		luaL_checktype(L, 1, LUA_TTABLE);
		int added = 0;
		lua_pushnil(L);
		while (lua_next(L, 1) != 0)
		{
			IconRegistry::Kind kind;
			if (lua_type(L, -2) == LUA_TSTRING && lua_type(L, -1) == LUA_TTABLE &&
				IconRegistry::findKind(lua_tostring(L, -2), kind))
			{
				IconRegistry::clear(kind);
				added += applySpec(L, lua_gettop(L), kind);
			}
			else if (lua_type(L, -2) == LUA_TSTRING)
			{
				CCLOG("[IconSystem] jenis gambar dilewati: %s", lua_tostring(L, -2));
			}
			lua_pop(L, 1);
		}
		lua_pushinteger(L, (lua_Integer)added);
		return 1;
	}

	static int icSet(lua_State *L)
	{
		IconRegistry::Kind kind;
		luaL_checktype(L, 2, LUA_TTABLE);
		if (!readKind(L, 1, kind))
		{
			lua_pushinteger(L, 0);
			return 1;
		}
		IconRegistry::clear(kind);
		lua_pushinteger(L, (lua_Integer)applySpec(L, 2, kind));
		return 1;
	}

	static int icAdd(lua_State *L)
	{
		IconRegistry::Kind kind;
		const char *source = luaL_checkstring(L, 2);
		const char *unit = luaL_checkstring(L, 3);
		lua_pushboolean(L, readKind(L, 1, kind) && IconRegistry::add(kind, source, unit) ? 1 : 0);
		return 1;
	}

	static int icRemove(lua_State *L)
	{
		IconRegistry::Kind kind;
		lua_pushboolean(L, readKind(L, 1, kind) && IconRegistry::remove(kind, luaL_checkstring(L, 2)) ? 1 : 0);
		return 1;
	}

	static int icClear(lua_State *L)
	{
		IconRegistry::Kind kind;
		if (lua_isnoneornil(L, 1))
			IconRegistry::clearAll();
		else if (readKind(L, 1, kind))
			IconRegistry::clear(kind);
		return 0;
	}

	static int icResolve(lua_State *L)
	{
		IconRegistry::Kind kind;
		const char *unit = luaL_checkstring(L, 2);
		if (readKind(L, 1, kind))
			lua_pushstring(L, IconRegistry::resolve(kind, unit).c_str());
		else
			lua_pushstring(L, unit);
		return 1;
	}

	static int icFrame(lua_State *L)
	{
		IconRegistry::Kind kind;
		const char *unit = luaL_checkstring(L, 2);
		if (readKind(L, 1, kind))
			lua_pushstring(L, iconFrameName(kind, unit).c_str());
		else
			lua_pushstring(L, unit);
		return 1;
	}

	static int icHas(lua_State *L)
	{
		IconRegistry::Kind kind;
		lua_pushboolean(L, readKind(L, 1, kind) && IconRegistry::has(kind, luaL_checkstring(L, 2)) ? 1 : 0);
		return 1;
	}

	static int icCount(lua_State *L)
	{
		lua_pushinteger(L, (lua_Integer)IconRegistry::count());
		return 1;
	}

	static void registerLua()
	{
		if (!LuaBridge::isReady())
		{
			CCLOG("[IconSystem] Lua engine not ready, Icons not registered");
			return;
		}

		lua_getL;
		static const struct
		{
			const char *name;
			lua_CFunction func;
		} kFuncs[] = {
			{"setAll", icSetAll},
			{"set", icSet},
			{"add", icAdd},
			{"remove", icRemove},
			{"clear", icClear},
			{"resolve", icResolve},
			{"frame", icFrame},
			{"has", icHas},
			{"count", icCount},
		};

		lua_newtable(L);
		for (const auto &f : kFuncs)
		{
			lua_pushcfunction(L, f.func);
			lua_setfield(L, -2, f.name);
		}
		lua_setglobal(L, "Icons");
	}
} // namespace IconSystem
