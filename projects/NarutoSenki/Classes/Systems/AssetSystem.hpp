#pragma once
#include "Data/UnitDefines.h"
#include "Utils/Cocos2dxHelper.hpp"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

/**
 * Jembatan Lua -> C++ untuk LoadLayer::perloadCharIMG / LoadLayer::unloadCharIMG
 * (aset tambahan per karakter).
 *
 * Mendaftarkan tabel global `Assets` ke Lua:
 *   Assets.set(name, spec)          -> integer  (ganti seluruh aset `name`; return jumlah entri)
 *   Assets.add(name, kind, value)   -> boolean  (tambah satu entri)
 *   Assets.setAll(tbl)              -> integer  ({Guy = {...}, Naruto = {...}}; return jumlah entri)
 *   Assets.remove(name)             -> boolean
 *   Assets.clear()
 *   Assets.has(name)                -> boolean
 *   Assets.count()                  -> integer  (jumlah karakter terdaftar)
 *
 * spec = tabel dengan key (masing-masing string atau array string):
 *   ninja, skill, kugutsu, kuchiyose, guardian, flog, plist (path mentah), ogg
 *
 * Contoh:
 *   Assets.set('Guy', { ninja = 'Guy8Gates', skill = 'Guy8Gates', ogg = 'Guy8Gates' })
 *   -- preload  : addSprites(mk_ninja_plist("Guy8Gates")); addSprites(mk_skill_plist("Guy8Gates"));
 *   --            KTools::prepareFileOGG("Guy8Gates");
 *   -- unload   : removeSprites(...) x2; KTools::prepareFileOGG("Guy8Gates", true);
 *
 * Dipanggil sekali dari Internal::initAllSystems(), sebelum main.lua dijalankan.
 */
namespace AssetSystem
{
	struct KindName
	{
		const char *key;
		AssetRegistry::Kind kind;
	};

	static const KindName kKinds[] = {
		{"ninja", AssetRegistry::Ninja},
		{"skill", AssetRegistry::Skill},
		{"kugutsu", AssetRegistry::Kugutsu},
		{"kuchiyose", AssetRegistry::Kuchiyose},
		{"guardian", AssetRegistry::Guardian},
		{"flog", AssetRegistry::Flog},
		{"plist", AssetRegistry::Plist},
		{"ogg", AssetRegistry::Ogg},
	};

	static bool findKind(const char *key, AssetRegistry::Kind &out)
	{
		for (const auto &k : kKinds)
		{
			if (strcmp(k.key, key) == 0)
			{
				out = k.kind;
				return true;
			}
		}
		return false;
	}

	// Lua 5.1 / LuaJIT tidak punya lua_absindex
	static inline int absIndex(lua_State *L, int idx)
	{
		return (idx > 0 || idx <= LUA_REGISTRYINDEX) ? idx : lua_gettop(L) + idx + 1;
	}

	// Baca nilai di indeks `idx` (string atau array string) dan tambahkan ke registry.
	static int addValues(lua_State *L, int idx, const std::string &name, AssetRegistry::Kind kind)
	{
		int added = 0;
		idx = absIndex(L, idx);
		if (lua_type(L, idx) == LUA_TSTRING)
		{
			if (AssetRegistry::add(name, kind, lua_tostring(L, idx)))
				added++;
		}
		else if (lua_type(L, idx) == LUA_TTABLE)
		{
			int n = (int)lua_objlen(L, idx);
			for (int i = 1; i <= n; i++)
			{
				lua_rawgeti(L, idx, i);
				if (lua_type(L, -1) == LUA_TSTRING && AssetRegistry::add(name, kind, lua_tostring(L, -1)))
					added++;
				lua_pop(L, 1);
			}
		}
		return added;
	}

	// Isi registry dari tabel spec di indeks `idx` (tabel sudah dikosongkan sebelumnya oleh pemanggil).
	static int applySpec(lua_State *L, int idx, const std::string &name)
	{
		idx = absIndex(L, idx);
		int added = 0;
		for (const auto &k : kKinds)
		{
			lua_getfield(L, idx, k.key);
			added += addValues(L, -1, name, k.kind);
			lua_pop(L, 1);
		}
		return added;
	}

	static int asSet(lua_State *L)
	{
		const char *name = luaL_checkstring(L, 1);
		luaL_checktype(L, 2, LUA_TTABLE);
		AssetRegistry::remove(name);
		lua_pushinteger(L, (lua_Integer)applySpec(L, 2, name));
		return 1;
	}

	static int asAdd(lua_State *L)
	{
		const char *name = luaL_checkstring(L, 1);
		const char *kindStr = luaL_checkstring(L, 2);
		const char *value = luaL_checkstring(L, 3);
		AssetRegistry::Kind kind;
		if (!findKind(kindStr, kind))
		{
			CCLOG("[AssetSystem] kind tidak dikenal: %s", kindStr);
			lua_pushboolean(L, 0);
			return 1;
		}
		lua_pushboolean(L, AssetRegistry::add(name, kind, value) ? 1 : 0);
		return 1;
	}

	static int asSetAll(lua_State *L)
	{
		luaL_checktype(L, 1, LUA_TTABLE);
		int added = 0;
		lua_pushnil(L);
		while (lua_next(L, 1) != 0)
		{
			if (lua_type(L, -2) == LUA_TSTRING && lua_type(L, -1) == LUA_TTABLE)
			{
				std::string name = lua_tostring(L, -2);
				AssetRegistry::remove(name);
				added += applySpec(L, lua_gettop(L), name);
			}
			lua_pop(L, 1);
		}
		lua_pushinteger(L, (lua_Integer)added);
		return 1;
	}

	static int asRemove(lua_State *L)
	{
		lua_pushboolean(L, AssetRegistry::remove(luaL_checkstring(L, 1)) ? 1 : 0);
		return 1;
	}

	static int asClear(lua_State *)
	{
		AssetRegistry::clear();
		return 0;
	}

	static int asHas(lua_State *L)
	{
		lua_pushboolean(L, AssetRegistry::has(luaL_checkstring(L, 1)) ? 1 : 0);
		return 1;
	}

	static int asCount(lua_State *L)
	{
		lua_pushinteger(L, (lua_Integer)AssetRegistry::count());
		return 1;
	}

	static void registerLua()
	{
		if (!LuaBridge::isReady())
		{
			CCLOG("[AssetSystem] Lua engine not ready, Assets not registered");
			return;
		}

		lua_getL;
		static const struct
		{
			const char *name;
			lua_CFunction func;
		} kFuncs[] = {
			{"set", asSet},
			{"add", asAdd},
			{"setAll", asSetAll},
			{"remove", asRemove},
			{"clear", asClear},
			{"has", asHas},
			{"count", asCount},
		};

		lua_newtable(L);
		for (const auto &f : kFuncs)
		{
			lua_pushcfunction(L, f.func);
			lua_setfield(L, -2, f.name);
		}
		lua_setglobal(L, "Assets");
	}
} // namespace AssetSystem
