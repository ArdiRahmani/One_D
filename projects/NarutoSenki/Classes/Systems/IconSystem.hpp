#pragma once
#include "Data/UnitDefines.h"
#include "Utils/Cocos2dxHelper.hpp"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

/**
 * Jembatan Lua -> C++ untuk gambar hero (IconRegistry): _rp.png, _rpf.png, _CutIn.png
 * (plus _avator / _small). Aturannya sama dengan GameOver (half.png), tetapi gambarnya
 * dari plist (frame), bukan dari file.
 *
 * Mendaftarkan tabel global `Icons` ke Lua:
 *   Icons.setAll(cfg)                  -> integer  (lihat format di bawah; return jumlah entri)
 *   Icons.setBind(kind, unit, source)  -> boolean  (unit memakai gambar hero `source`; source nil = hapus)
 *   Icons.setFile(kind, unit, frame)   -> boolean  (nama frame LENGKAP di plist; frame nil = hapus)
 *   Icons.setFrame(kind, template)     -> boolean  (templat nama frame, "{}" = hero sumber)
 *   Icons.getFrame(kind)               -> string   (templat nama frame yang aktif)
 *   Icons.setDefaultScale(kind, n)     -> boolean  (skala umum jenis ini; bawaan 1.0)
 *   Icons.getDefaultScale(kind)        -> number
 *   Icons.setScale(kind, unit, n)      -> boolean  (skala khusus unit; n nil = hapus)
 *   Icons.getScale(kind, unit)         -> number   (skala yang dipakai unit; skala hanya rp, rpf, CutIn)
 *   Icons.resolve(kind, unit)          -> string   (hero sumber; = unit kalau tidak ada binding)
 *   Icons.frame(kind, unit)            -> string   (nama frame yang dipakai, mis. 'Obito_rpf.png';
 *                                                   kembali ke frame unit sendiri kalau tidak ada di plist)
 *   Icons.half(unit)                   -> string|nil  (path half yang BENAR-BENAR ada: Img/Half/<n>_half.png
 *                                                   ikut GameOver.half/file/path, lalu half unit sendiri; nil kalau tidak ada)
 *   Icons.has(kind, unit)              -> boolean  (ada binding / file untuk unit ini)
 *   Icons.clear([kind])                           (kembalikan ke bawaan: tanpa binding, skala 1.0)
 *   Icons.count()                      -> integer
 *
 * kind (tidak peka huruf besar/kecil): rp, rpf, CutIn, Avator, small
 * cfg  = { rp = {frame=, file={}, bind={}, defaultScale=, scale={}}, rpf = {...}, CutIn = {...}, ... }
 *
 * Khusus hero: nama banner kill-streak (DoubleKill, KillingSpree, ..., MonsterKill) dan Tower
 * ditolak (setBind / setFile / setScale mengembalikan false).
 *
 * Dipanggil sekali dari Internal::initAllSystems(), sebelum main.lua dijalankan.
 */
namespace IconSystem
{
	static inline bool isStr(lua_State *L, int idx) { return lua_type(L, idx) == LUA_TSTRING; }

	static inline int absIndex(lua_State *L, int idx)
	{
		return (idx > 0 || idx <= LUA_REGISTRYINDEX) ? idx : lua_gettop(L) + idx + 1;
	}

	static bool readKind(lua_State *L, int idx, IconRegistry::Kind &out)
	{
		const char *key = luaL_checkstring(L, idx);
		if (IconRegistry::findKind(key, out))
			return true;
		CCLOG("[IconSystem] jenis gambar tidak dikenal: %s (rp, rpf, CutIn, Avator, small)", key);
		return false;
	}

	// Isi satu jenis dari tabel cfg di indeks `idx`. Setelan jenis ini direset lebih dulu.
	// Return jumlah entri yang masuk.
	static int applyKind(lua_State *L, int idx, IconRegistry::Kind kind)
	{
		idx = absIndex(L, idx);
		IconRegistry::clear(kind);
		int added = 0;

		lua_getfield(L, idx, "frame");
		if (isStr(L, -1) && IconRegistry::setTemplate(kind, lua_tostring(L, -1)))
			added++;
		lua_pop(L, 1);

		lua_getfield(L, idx, "defaultScale");
		if (lua_type(L, -1) == LUA_TNUMBER && IconRegistry::setDefaultScale(kind, (float)lua_tonumber(L, -1)))
			added++;
		lua_pop(L, 1);

		lua_getfield(L, idx, "file");
		if (lua_istable(L, -1))
		{
			const int t = lua_gettop(L);
			lua_pushnil(L);
			while (lua_next(L, t) != 0)
			{
				// Hanya string -> string (jangan lua_tostring ke key angka: mengubah key asli)
				if (isStr(L, -2) && isStr(L, -1) && IconRegistry::setFile(kind, lua_tostring(L, -2), lua_tostring(L, -1)))
					added++;
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, idx, "bind");
		if (lua_istable(L, -1))
		{
			const int t = lua_gettop(L);
			lua_pushnil(L);
			while (lua_next(L, t) != 0)
			{
				if (isStr(L, -2) && isStr(L, -1) && IconRegistry::setBind(kind, lua_tostring(L, -2), lua_tostring(L, -1)))
					added++;
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, idx, "scale");
		if (lua_istable(L, -1))
		{
			const int t = lua_gettop(L);
			lua_pushnil(L);
			while (lua_next(L, t) != 0)
			{
				if (isStr(L, -2) && lua_type(L, -1) == LUA_TNUMBER &&
					IconRegistry::setScale(kind, lua_tostring(L, -2), (float)lua_tonumber(L, -1)))
					added++;
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);

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
			if (isStr(L, -2) && lua_istable(L, -1) && IconRegistry::findKind(lua_tostring(L, -2), kind))
				added += applyKind(L, lua_gettop(L), kind);
			else if (isStr(L, -2))
				CCLOG("[IconSystem] jenis gambar dilewati: %s", lua_tostring(L, -2));
			lua_pop(L, 1);
		}
		lua_pushinteger(L, (lua_Integer)added);
		return 1;
	}

	static int icSetBind(lua_State *L)
	{
		IconRegistry::Kind kind;
		const char *unit = luaL_checkstring(L, 2);
		if (!readKind(L, 1, kind))
		{
			lua_pushboolean(L, 0);
			return 1;
		}
		if (lua_isnoneornil(L, 3))
		{
			IconRegistry::removeBind(kind, unit);
			lua_pushboolean(L, 1);
			return 1;
		}
		lua_pushboolean(L, IconRegistry::setBind(kind, unit, luaL_checkstring(L, 3)) ? 1 : 0);
		return 1;
	}

	static int icSetFile(lua_State *L)
	{
		IconRegistry::Kind kind;
		const char *unit = luaL_checkstring(L, 2);
		if (!readKind(L, 1, kind))
		{
			lua_pushboolean(L, 0);
			return 1;
		}
		if (lua_isnoneornil(L, 3))
		{
			IconRegistry::removeFile(kind, unit);
			lua_pushboolean(L, 1);
			return 1;
		}
		lua_pushboolean(L, IconRegistry::setFile(kind, unit, luaL_checkstring(L, 3)) ? 1 : 0);
		return 1;
	}

	static int icSetFrame(lua_State *L)
	{
		IconRegistry::Kind kind;
		const char *tpl = luaL_checkstring(L, 2);
		lua_pushboolean(L, readKind(L, 1, kind) && IconRegistry::setTemplate(kind, tpl) ? 1 : 0);
		return 1;
	}

	static int icGetFrame(lua_State *L)
	{
		IconRegistry::Kind kind;
		if (readKind(L, 1, kind))
			lua_pushstring(L, IconRegistry::config(kind).frameTemplate.c_str());
		else
			lua_pushnil(L);
		return 1;
	}

	static int icSetDefaultScale(lua_State *L)
	{
		IconRegistry::Kind kind;
		const lua_Number v = luaL_checknumber(L, 2);
		lua_pushboolean(L, readKind(L, 1, kind) && IconRegistry::setDefaultScale(kind, (float)v) ? 1 : 0);
		return 1;
	}

	static int icGetDefaultScale(lua_State *L)
	{
		IconRegistry::Kind kind;
		if (readKind(L, 1, kind))
			lua_pushnumber(L, IconRegistry::config(kind).defaultScale);
		else
			lua_pushnil(L);
		return 1;
	}

	static int icSetScale(lua_State *L)
	{
		IconRegistry::Kind kind;
		const char *unit = luaL_checkstring(L, 2);
		if (!readKind(L, 1, kind))
		{
			lua_pushboolean(L, 0);
			return 1;
		}
		if (lua_isnoneornil(L, 3))
		{
			IconRegistry::removeScale(kind, unit);
			lua_pushboolean(L, 1);
			return 1;
		}
		lua_pushboolean(L, IconRegistry::setScale(kind, unit, (float)luaL_checknumber(L, 3)) ? 1 : 0);
		return 1;
	}

	static int icGetScale(lua_State *L)
	{
		IconRegistry::Kind kind;
		const char *unit = luaL_checkstring(L, 2);
		lua_pushnumber(L, readKind(L, 1, kind) ? IconRegistry::scaleOf(kind, unit) : 1.0);
		return 1;
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

	static int icHalf(lua_State *L)
	{
		const char *unit = luaL_checkstring(L, 1);
		auto fu = FileUtils::sharedFileUtils();
		std::string path = GameOverRegistry::halfPath(unit);
		if (!fu->isFileExist(path.c_str()))
			path = GameOverRegistry::ownHalfPath(unit);
		if (fu->isFileExist(path.c_str()))
			lua_pushstring(L, path.c_str());
		else
			lua_pushnil(L);
		return 1;
	}

	static int icHas(lua_State *L)
	{
		IconRegistry::Kind kind;
		const char *unit = luaL_checkstring(L, 2);
		lua_pushboolean(L, readKind(L, 1, kind) && IconRegistry::has(kind, unit) ? 1 : 0);
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
			{"setBind", icSetBind},
			{"setFile", icSetFile},
			{"setFrame", icSetFrame},
			{"getFrame", icGetFrame},
			{"setDefaultScale", icSetDefaultScale},
			{"getDefaultScale", icGetDefaultScale},
			{"setScale", icSetScale},
			{"getScale", icGetScale},
			{"resolve", icResolve},
			{"frame", icFrame},
			{"half", icHalf},
			{"has", icHas},
			{"clear", icClear},
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
