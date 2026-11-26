#pragma once
#include "Data/UnitDefines.h"
#include "Utils/Cocos2dxHelper.hpp"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

/**
 * Jembatan Lua -> C++ untuk layar hasil akhir (GameOver::listResult).
 *
 * Mendaftarkan tabel global `GameOver` ke Lua:
 *   GameOver.setAll(tbl)            -> integer  ({flipX = {...}, half = {...}, scale = {...}, record = {...}}; return jumlah entri)
 *   GameOver.setFlipX(name[, bool]) -> boolean  (bool default true; false = hapus dari daftar flipX)
 *   GameOver.setFlipXAll(arr)       -> integer  (ganti seluruh daftar flipX)
 *   GameOver.isFlipX(name)          -> boolean
 *   GameOver.setPath(template)      -> boolean  (templat path half, "{}" = nama hero sumber; default 'Img/Half/{}_half.png')
 *   GameOver.getPath()              -> string
 *   GameOver.setFile(name, path)    -> boolean  (path half LENGKAP untuk `name`; nil = hapus)
 *   GameOver.resolvePath(name)      -> string   (path half yang dipakai `name`, belum dicek ada/tidaknya)
 *   GameOver.setDefaultScale(n)     -> boolean  (skala half umum untuk unit tanpa entri scale; default 1.0)
 *   GameOver.getDefaultScale()      -> number
 *   GameOver.setScale(name, scale)  -> boolean  (skala half->setScale untuk `name`; nil = kembali 1.0)
 *   GameOver.getScale(name)         -> number   (default 1.0)
 *   GameOver.setHalf(name, source)  -> boolean  (Img/Half/<source>_half.png dipakai untuk `name`; nil = hapus)
 *   GameOver.resolveHalf(name)      -> string   (hero sumber gambar half; = name kalau tidak ada binding)
 *   GameOver.setRecord(name, alias) -> boolean  (kemenangan `name` dicatat di baris CharRecord `alias`; nil = hapus)
 *   GameOver.resolveRecord(name)    -> string   (nama baris catatan; = name kalau tidak ada binding)
 *   GameOver.clear()
 *   GameOver.count()                -> integer
 *
 * Dipanggil sekali dari Internal::initAllSystems(), sebelum main.lua dijalankan.
 */
namespace GameOverSystem
{
	static bool isStr(lua_State *L, int idx) { return lua_type(L, idx) == LUA_TSTRING; }

	static inline int absIndex(lua_State *L, int idx)
	{
		return (idx > 0 || idx <= LUA_REGISTRYINDEX) ? idx : lua_gettop(L) + idx + 1;
	}

	// Array string di indeks `idx` -> daftar flipX (daftar lama TIDAK dikosongkan di sini).
	static int addFlipArray(lua_State *L, int idx)
	{
		idx = absIndex(L, idx);
		int added = 0;
		const int n = (int)lua_objlen(L, idx);
		for (int i = 1; i <= n; i++)
		{
			lua_rawgeti(L, idx, i);
			if (isStr(L, -1) && *lua_tostring(L, -1))
			{
				GameOverRegistry::flipSet().insert(lua_tostring(L, -1));
				added++;
			}
			lua_pop(L, 1);
		}
		return added;
	}

	// Tabel { unit = 'sumber' } di indeks `idx` -> AliasMap (tidak dikosongkan di sini).
	static int addAliasTable(lua_State *L, int idx, AliasMap &map)
	{
		idx = absIndex(L, idx);
		int added = 0;
		lua_pushnil(L);
		while (lua_next(L, idx) != 0)
		{
			// Hanya string -> string (jangan lua_tostring ke key angka: mengubah key asli)
			if (isStr(L, -2) && isStr(L, -1) && map.set(lua_tostring(L, -2), lua_tostring(L, -1)))
				added++;
			lua_pop(L, 1);
		}
		return added;
	}

	static int goSetAll(lua_State *L)
	{
		luaL_checktype(L, 1, LUA_TTABLE);
		int added = 0;

		lua_getfield(L, 1, "flipX");
		if (lua_istable(L, -1))
		{
			GameOverRegistry::flipSet().clear();
			added += addFlipArray(L, -1);
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "half");
		if (lua_istable(L, -1))
		{
			GameOverRegistry::halfMap().clear();
			added += addAliasTable(L, -1, GameOverRegistry::halfMap());
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "path");
		if (isStr(L, -1) && *lua_tostring(L, -1))
		{
			GameOverRegistry::pathTemplate() = lua_tostring(L, -1);
			added++;
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "file");
		if (lua_istable(L, -1))
		{
			GameOverRegistry::fileMap().clear();
			int idx = lua_gettop(L);
			lua_pushnil(L);
			while (lua_next(L, idx) != 0)
			{
				if (isStr(L, -2) && isStr(L, -1) && *lua_tostring(L, -2) && *lua_tostring(L, -1))
				{
					GameOverRegistry::fileMap()[lua_tostring(L, -2)] = lua_tostring(L, -1);
					added++;
				}
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "defaultScale");
		if (lua_type(L, -1) == LUA_TNUMBER && lua_tonumber(L, -1) > 0)
		{
			GameOverRegistry::defaultScale() = (float)lua_tonumber(L, -1);
			added++;
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "scale");
		if (lua_istable(L, -1))
		{
			GameOverRegistry::scaleMap().clear();
			int idx = lua_gettop(L);
			lua_pushnil(L);
			while (lua_next(L, idx) != 0)
			{
				if (isStr(L, -2) && lua_type(L, -1) == LUA_TNUMBER && lua_tonumber(L, -1) > 0)
				{
					GameOverRegistry::scaleMap()[lua_tostring(L, -2)] = (float)lua_tonumber(L, -1);
					added++;
				}
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, 1, "record");
		if (lua_istable(L, -1))
		{
			GameOverRegistry::recordMap().clear();
			added += addAliasTable(L, -1, GameOverRegistry::recordMap());
		}
		lua_pop(L, 1);

		lua_pushinteger(L, (lua_Integer)added);
		return 1;
	}

	static int goSetFlipX(lua_State *L)
	{
		const char *name = luaL_checkstring(L, 1);
		const bool on = lua_isnoneornil(L, 2) ? true : lua_toboolean(L, 2) != 0;
		if (!*name)
		{
			lua_pushboolean(L, 0);
			return 1;
		}
		if (on)
			GameOverRegistry::flipSet().insert(name);
		else
			GameOverRegistry::flipSet().erase(name);
		lua_pushboolean(L, 1);
		return 1;
	}

	static int goSetFlipXAll(lua_State *L)
	{
		luaL_checktype(L, 1, LUA_TTABLE);
		GameOverRegistry::flipSet().clear();
		lua_pushinteger(L, (lua_Integer)addFlipArray(L, 1));
		return 1;
	}

	static int goIsFlipX(lua_State *L)
	{
		lua_pushboolean(L, GameOverRegistry::isFlipX(luaL_checkstring(L, 1)) ? 1 : 0);
		return 1;
	}

	static int setAlias(lua_State *L, AliasMap &map)
	{
		const char *name = luaL_checkstring(L, 1);
		if (lua_isnoneornil(L, 2))
		{
			map.remove(name);
			lua_pushboolean(L, 1);
			return 1;
		}
		lua_pushboolean(L, map.set(name, luaL_checkstring(L, 2)) ? 1 : 0);
		return 1;
	}

	static int goSetPath(lua_State *L)
	{
		const char *tpl = luaL_checkstring(L, 1);
		if (!*tpl)
		{
			lua_pushboolean(L, 0);
			return 1;
		}
		GameOverRegistry::pathTemplate() = tpl;
		lua_pushboolean(L, 1);
		return 1;
	}

	static int goGetPath(lua_State *L)
	{
		lua_pushstring(L, GameOverRegistry::pathTemplate().c_str());
		return 1;
	}

	static int goSetFile(lua_State *L)
	{
		const char *name = luaL_checkstring(L, 1);
		if (lua_isnoneornil(L, 2))
		{
			GameOverRegistry::fileMap().erase(name);
			lua_pushboolean(L, 1);
			return 1;
		}
		const char *path = luaL_checkstring(L, 2);
		if (!*name || !*path)
		{
			lua_pushboolean(L, 0);
			return 1;
		}
		GameOverRegistry::fileMap()[name] = path;
		lua_pushboolean(L, 1);
		return 1;
	}

	static int goResolvePath(lua_State *L)
	{
		lua_pushstring(L, GameOverRegistry::halfPath(luaL_checkstring(L, 1)).c_str());
		return 1;
	}

	static int goSetDefaultScale(lua_State *L)
	{
		const lua_Number v = luaL_checknumber(L, 1);
		if (v <= 0)
		{
			lua_pushboolean(L, 0);
			return 1;
		}
		GameOverRegistry::defaultScale() = (float)v;
		lua_pushboolean(L, 1);
		return 1;
	}

	static int goGetDefaultScale(lua_State *L)
	{
		lua_pushnumber(L, GameOverRegistry::defaultScale());
		return 1;
	}

	static int goSetScale(lua_State *L)
	{
		const char *name = luaL_checkstring(L, 1);
		if (lua_isnoneornil(L, 2))
		{
			GameOverRegistry::scaleMap().erase(name);
			lua_pushboolean(L, 1);
			return 1;
		}
		const lua_Number v = luaL_checknumber(L, 2);
		if (!*name || v <= 0)
		{
			lua_pushboolean(L, 0);
			return 1;
		}
		GameOverRegistry::scaleMap()[name] = (float)v;
		lua_pushboolean(L, 1);
		return 1;
	}

	static int goGetScale(lua_State *L)
	{
		lua_pushnumber(L, GameOverRegistry::halfScale(luaL_checkstring(L, 1)));
		return 1;
	}

	static int goSetHalf(lua_State *L) { return setAlias(L, GameOverRegistry::halfMap()); }
	static int goSetRecord(lua_State *L) { return setAlias(L, GameOverRegistry::recordMap()); }

	static int goResolveHalf(lua_State *L)
	{
		lua_pushstring(L, GameOverRegistry::halfName(luaL_checkstring(L, 1)).c_str());
		return 1;
	}

	static int goResolveRecord(lua_State *L)
	{
		lua_pushstring(L, GameOverRegistry::recordName(luaL_checkstring(L, 1)).c_str());
		return 1;
	}

	static int goClear(lua_State *)
	{
		GameOverRegistry::clear();
		return 0;
	}

	static int goCount(lua_State *L)
	{
		lua_pushinteger(L, (lua_Integer)GameOverRegistry::count());
		return 1;
	}

	static void registerLua()
	{
		if (!LuaBridge::isReady())
		{
			CCLOG("[GameOverSystem] Lua engine not ready, GameOver not registered");
			return;
		}

		lua_getL;
		static const struct
		{
			const char *name;
			lua_CFunction func;
		} kFuncs[] = {
			{"setAll", goSetAll},
			{"setFlipX", goSetFlipX},
			{"setFlipXAll", goSetFlipXAll},
			{"isFlipX", goIsFlipX},
			{"setPath", goSetPath},
			{"getPath", goGetPath},
			{"setFile", goSetFile},
			{"resolvePath", goResolvePath},
			{"setDefaultScale", goSetDefaultScale},
			{"getDefaultScale", goGetDefaultScale},
			{"setScale", goSetScale},
			{"getScale", goGetScale},
			{"setHalf", goSetHalf},
			{"resolveHalf", goResolveHalf},
			{"setRecord", goSetRecord},
			{"resolveRecord", goResolveRecord},
			{"clear", goClear},
			{"count", goCount},
		};

		lua_newtable(L);
		for (const auto &f : kFuncs)
		{
			lua_pushcfunction(L, f.func);
			lua_setfield(L, -2, f.name);
		}
		lua_setglobal(L, "GameOver");
	}
} // namespace GameOverSystem
