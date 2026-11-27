#pragma once
#include "Data/UnitDefines.h"
#include "Utils/Cocos2dxHelper.hpp"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

/**
 * Jembatan Lua -> C++ untuk alias tipe buff (BuffRegistry).
 *
 * Mendaftarkan tabel global `Buffs` ke Lua:
 *   Buffs.set(alias, base)   -> boolean  (alias berperilaku sama dengan tipe bawaan `base`)
 *   Buffs.set(alias, nil)    -> boolean  (hapus alias)
 *   Buffs.setAll(tbl)        -> integer  ({h2Buff = 'hBuff', ...}; return jumlah yang masuk)
 *   Buffs.remove(alias)      -> boolean
 *   Buffs.clear()
 *   Buffs.get(alias)         -> string|nil  (tipe bawaan langsung, nil kalau tidak terdaftar)
 *   Buffs.resolve(type)      -> string      (tipe bawaan akhir, mengikuti rantai alias)
 *   Buffs.has(alias)         -> boolean
 *   Buffs.effect(name, {prefix=, frames=, fps=, loop=, anchorX=, anchorY=, offsetX=, offsetY=}) -> boolean
 *                            (efek visual buff sendiri: animasi frame "<prefix>01..NN" di plist yang termuat)
 *   Buffs.effect(name, nil)  -> boolean  (hapus efek)
 *   Buffs.effects(tbl)       -> integer  ({h2Buff = {prefix='h2Buff_Effect_', frames=6}, ...})
 *   Buffs.count()            -> integer
 *
 * Contoh di Lua:
 *   Buffs.set('h2Buff', 'hBuff')
 *   -- skill dengan attackType "h2Buff" diproses seperti "hBuff" oleh CharacterBase::setBuff
 *   Buffs.effect('h2Buff', {prefix = 'h2Buff_Effect_', frames = 6, fps = 8})
 *   -- efek h2Buff memakai animasi sendiri; tanpa ini, memakai efek hBuff
 *
 * Dipanggil sekali dari Internal::initAllSystems(), sebelum main.lua dijalankan.
 */
namespace BuffSystem
{
	static int bfSet(lua_State *L)
	{
		const char *alias = luaL_checkstring(L, 1);
		if (lua_isnoneornil(L, 2))
		{
			BuffRegistry::remove(alias);
			lua_pushboolean(L, 1);
			return 1;
		}
		lua_pushboolean(L, BuffRegistry::set(alias, luaL_checkstring(L, 2)) ? 1 : 0);
		return 1;
	}

	static int bfSetAll(lua_State *L)
	{
		luaL_checktype(L, 1, LUA_TTABLE);
		int added = 0;
		lua_pushnil(L);
		while (lua_next(L, 1) != 0)
		{
			if (lua_type(L, -2) == LUA_TSTRING && lua_type(L, -1) == LUA_TSTRING)
			{
				if (BuffRegistry::set(lua_tostring(L, -2), lua_tostring(L, -1)))
					added++;
			}
			lua_pop(L, 1);
		}
		lua_pushinteger(L, (lua_Integer)added);
		return 1;
	}

	static int bfRemove(lua_State *L)
	{
		lua_pushboolean(L, BuffRegistry::remove(luaL_checkstring(L, 1)) ? 1 : 0);
		return 1;
	}

	static int bfClear(lua_State *)
	{
		BuffRegistry::clear();
		return 0;
	}

	static int bfGet(lua_State *L)
	{
		const char *alias = luaL_checkstring(L, 1);
		auto it = BuffRegistry::store().find(alias);
		if (it != BuffRegistry::store().end())
			lua_pushstring(L, it->second.c_str());
		else
			lua_pushnil(L);
		return 1;
	}

	static int bfResolve(lua_State *L)
	{
		lua_pushstring(L, BuffRegistry::resolve(luaL_checkstring(L, 1)).c_str());
		return 1;
	}

	static int bfHas(lua_State *L)
	{
		lua_pushboolean(L, BuffRegistry::has(luaL_checkstring(L, 1)) ? 1 : 0);
		return 1;
	}

	static float numField(lua_State *L, int idx, const char *key, float def)
	{
		lua_getfield(L, idx, key);
		float v = lua_isnumber(L, -1) ? (float)lua_tonumber(L, -1) : def;
		lua_pop(L, 1);
		return v;
	}

	// Baca tabel definisi efek di stack index `idx`
	static bool readEffect(lua_State *L, int idx, BuffRegistry::EffectDef &def)
	{
		if (lua_type(L, idx) != LUA_TTABLE)
			return false;
		lua_getfield(L, idx, "prefix");
		if (lua_type(L, -1) != LUA_TSTRING)
		{
			lua_pop(L, 1);
			return false;
		}
		def.prefix = lua_tostring(L, -1);
		lua_pop(L, 1);

		def.frames = (size_t)numField(L, idx, "frames", 1);
		def.fps = (unsigned)numField(L, idx, "fps", 10);
		lua_getfield(L, idx, "loop");
		def.loop = lua_isnil(L, -1) ? true : (lua_toboolean(L, -1) != 0);
		lua_pop(L, 1);
		def.anchorX = numField(L, idx, "anchorX", 0.5f);
		def.anchorY = numField(L, idx, "anchorY", 0.0f);
		def.offsetX = numField(L, idx, "offsetX", 0.0f);
		def.offsetY = numField(L, idx, "offsetY", 0.0f);
		return true;
	}

	static int bfEffect(lua_State *L)
	{
		const char *name = luaL_checkstring(L, 1);
		if (lua_isnoneornil(L, 2))
		{
			BuffRegistry::removeEffect(name);
			lua_pushboolean(L, 1);
			return 1;
		}
		BuffRegistry::EffectDef def;
		if (!readEffect(L, 2, def))
			return luaL_error(L, "Buffs.effect: butuh tabel {prefix='...', frames=N, ...}");
		lua_pushboolean(L, BuffRegistry::setEffect(name, def) ? 1 : 0);
		return 1;
	}

	static int bfEffects(lua_State *L)
	{
		luaL_checktype(L, 1, LUA_TTABLE);
		int added = 0;
		lua_pushnil(L);
		while (lua_next(L, 1) != 0)
		{
			BuffRegistry::EffectDef def;
			if (lua_type(L, -2) == LUA_TSTRING && readEffect(L, lua_gettop(L), def))
			{
				if (BuffRegistry::setEffect(lua_tostring(L, -2), def))
					added++;
			}
			else if (lua_type(L, -2) == LUA_TSTRING)
				CCLOG("[Buffs] effect '%s' tidak valid (butuh prefix string)", lua_tostring(L, -2));
			lua_pop(L, 1);
		}
		lua_pushinteger(L, (lua_Integer)added);
		return 1;
	}

	static int bfCount(lua_State *L)
	{
		lua_pushinteger(L, (lua_Integer)BuffRegistry::count());
		return 1;
	}

	static void registerLua()
	{
		if (!LuaBridge::isReady())
		{
			CCLOG("[BuffSystem] Lua engine not ready, Buffs not registered");
			return;
		}

		lua_getL;
		static const struct
		{
			const char *name;
			lua_CFunction func;
		} kFuncs[] = {
			{"set", bfSet},
			{"setAll", bfSetAll},
			{"remove", bfRemove},
			{"clear", bfClear},
			{"get", bfGet},
			{"resolve", bfResolve},
			{"has", bfHas},
			{"effect", bfEffect},
			{"effects", bfEffects},
			{"count", bfCount},
		};

		lua_newtable(L);
		for (const auto &f : kFuncs)
		{
			lua_pushcfunction(L, f.func);
			lua_setfield(L, -2, f.name);
		}
		lua_setglobal(L, "Buffs");
	}
} // namespace BuffSystem
