#pragma once
#include "Data/UnitDefines.h"
#include "Utils/Cocos2dxHelper.hpp"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

/**
 * Jembatan Lua -> C++ untuk flag perilaku karakter (UnitFlagRegistry).
 *
 * Mendaftarkan tabel global `UnitFlags` ke Lua:
 *   UnitFlags.set(name, flag [, flag ...])   -> boolean  (menambah flag; boleh banyak argumen)
 *   UnitFlags.set(name, {flag, flag})        -> boolean  (versi tabel)
 *   UnitFlags.set(name, nil)                 -> boolean  (hapus semua flag unit, = remove)
 *   UnitFlags.setAll({ Madara = {'holdBuff','lockWalk'}, Mei = 'holdBuff' }) -> integer
 *   UnitFlags.remove(name)  UnitFlags.clear()  UnitFlags.count()
 *   UnitFlags.has(name [, flag])             -> boolean
 *   UnitFlags.get(name)                      -> {flag, ...}
 *
 * Nama flag (huruf besar/kecil bebas):
 *   'holdBuff'  setBuff tidak menjadwalkan resumeAction otomatis (lihat UnitFlagRegistry::HoldBuff)
 *   'lockWalk'  walk() AI diblok saat NATTACK + _isOnlySkillLocked   (UnitFlagRegistry::LockWalk)
 *   'healGroup' / 'healChakra' / 'healSelf' / 'healFlog'  perilaku healBuff (GroupBuff/GroupHeal/hBuff)
 *
 * Dipanggil sekali dari Internal::initAllSystems(), sebelum main.lua dijalankan.
 */
namespace UnitFlagSystem
{
	static bool parseFlag(const char *s, unsigned &out)
	{
		auto eq = [](const char *a, const char *b) {
			for (; *a && *b; ++a, ++b)
				if (std::tolower((unsigned char)*a) != std::tolower((unsigned char)*b))
					return false;
			return *a == *b;
		};
		if (eq(s, "holdBuff")) { out = UnitFlagRegistry::HoldBuff; return true; }
		if (eq(s, "lockWalk")) { out = UnitFlagRegistry::LockWalk; return true; }
		if (eq(s, "healGroup")) { out = UnitFlagRegistry::HealGroup; return true; }
		if (eq(s, "healChakra")) { out = UnitFlagRegistry::HealChakra; return true; }
		if (eq(s, "healSelf")) { out = UnitFlagRegistry::HealSelf; return true; }
		if (eq(s, "healFlog")) { out = UnitFlagRegistry::HealFlog; return true; }
		return false;
	}

	// Baca flag dari stack index `idx`: string tunggal atau tabel array string. Return false kalau ada nama tak dikenal.
	static bool readFlags(lua_State *L, int idx, unsigned &out, const char **bad)
	{
		out = 0;
		unsigned f = 0;
		if (lua_type(L, idx) == LUA_TSTRING)
		{
			if (!parseFlag(lua_tostring(L, idx), f)) { *bad = lua_tostring(L, idx); return false; }
			out |= f;
			return true;
		}
		if (lua_type(L, idx) == LUA_TTABLE)
		{
			const int n = (int)lua_objlen(L, idx);
			for (int i = 1; i <= n; ++i)
			{
				lua_rawgeti(L, idx, i);
				if (lua_type(L, -1) != LUA_TSTRING || !parseFlag(lua_tostring(L, -1), f))
				{
					*bad = lua_type(L, -1) == LUA_TSTRING ? lua_tostring(L, -1) : "<bukan string>";
					lua_pop(L, 1);
					return false;
				}
				out |= f;
				lua_pop(L, 1);
			}
			return true;
		}
		*bad = "<bukan string/tabel>";
		return false;
	}

	static int ufSet(lua_State *L)
	{
		const char *name = luaL_checkstring(L, 1);
		if (lua_isnoneornil(L, 2))
		{
			UnitFlagRegistry::remove(name);
			lua_pushboolean(L, 1);
			return 1;
		}
		unsigned flags = 0;
		const int top = lua_gettop(L);
		for (int i = 2; i <= top; ++i)
		{
			unsigned f = 0;
			const char *bad = nullptr;
			if (!readFlags(L, i, f, &bad))
				return luaL_error(L, "UnitFlags: flag tidak dikenal '%s' (pakai 'holdBuff' / 'lockWalk' / 'healGroup' / 'healChakra' / 'healSelf' / 'healFlog')", bad);
			flags |= f;
		}
		lua_pushboolean(L, UnitFlagRegistry::add(name, flags) ? 1 : 0);
		return 1;
	}

	static int ufSetAll(lua_State *L)
	{
		luaL_checktype(L, 1, LUA_TTABLE);
		int added = 0;
		lua_pushnil(L);
		while (lua_next(L, 1) != 0)
		{
			if (lua_type(L, -2) == LUA_TSTRING)
			{
				unsigned f = 0;
				const char *bad = nullptr;
				if (readFlags(L, lua_gettop(L), f, &bad))
				{
					if (UnitFlagRegistry::add(lua_tostring(L, -2), f))
						added++;
				}
				else
					CCLOG("[UnitFlags] %s: flag tidak dikenal '%s'", lua_tostring(L, -2), bad);
			}
			lua_pop(L, 1);
		}
		lua_pushinteger(L, (lua_Integer)added);
		return 1;
	}

	static int ufRemove(lua_State *L)
	{
		lua_pushboolean(L, UnitFlagRegistry::remove(luaL_checkstring(L, 1)) ? 1 : 0);
		return 1;
	}

	static int ufClear(lua_State *)
	{
		UnitFlagRegistry::clear();
		return 0;
	}

	static int ufCount(lua_State *L)
	{
		lua_pushinteger(L, (lua_Integer)UnitFlagRegistry::count());
		return 1;
	}

	static int ufHas(lua_State *L)
	{
		const unsigned have = UnitFlagRegistry::get(luaL_checkstring(L, 1));
		if (lua_isnoneornil(L, 2))
		{
			lua_pushboolean(L, have != 0);
			return 1;
		}
		unsigned want = 0;
		const char *bad = nullptr;
		if (!readFlags(L, 2, want, &bad))
			return luaL_error(L, "UnitFlags: flag tidak dikenal '%s'", bad);
		lua_pushboolean(L, (have & want) == want && want != 0);
		return 1;
	}

	static int ufGet(lua_State *L)
	{
		const unsigned have = UnitFlagRegistry::get(luaL_checkstring(L, 1));
		lua_newtable(L);
		int i = 1;
		if (have & UnitFlagRegistry::HoldBuff) { lua_pushstring(L, "holdBuff"); lua_rawseti(L, -2, i++); }
		if (have & UnitFlagRegistry::LockWalk) { lua_pushstring(L, "lockWalk"); lua_rawseti(L, -2, i++); }
		if (have & UnitFlagRegistry::HealGroup) { lua_pushstring(L, "healGroup"); lua_rawseti(L, -2, i++); }
		if (have & UnitFlagRegistry::HealChakra) { lua_pushstring(L, "healChakra"); lua_rawseti(L, -2, i++); }
		if (have & UnitFlagRegistry::HealSelf) { lua_pushstring(L, "healSelf"); lua_rawseti(L, -2, i++); }
		if (have & UnitFlagRegistry::HealFlog) { lua_pushstring(L, "healFlog"); lua_rawseti(L, -2, i++); }
		return 1;
	}

	static void registerLua()
	{
		if (!LuaBridge::isReady())
		{
			CCLOG("[UnitFlagSystem] Lua engine not ready, UnitFlags not registered");
			return;
		}

		lua_getL;
		static const struct
		{
			const char *name;
			lua_CFunction func;
		} kFuncs[] = {
			{"set", ufSet},
			{"setAll", ufSetAll},
			{"remove", ufRemove},
			{"clear", ufClear},
			{"count", ufCount},
			{"has", ufHas},
			{"get", ufGet},
		};

		lua_newtable(L);
		for (const auto &f : kFuncs)
		{
			lua_pushcfunction(L, f.func);
			lua_setfield(L, -2, f.name);
		}
		lua_setglobal(L, "UnitFlags");
	}
} // namespace UnitFlagSystem
