#pragma once
#include "Data/Config.h"
#include "GameMode/GameModeImpl.h"
#include "Utils/Cocos2dxHelper.hpp"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

extern int Cheats;
extern bool enableCustomSelect; // StartMenu.cpp: diisi tombol Yes di kotak HardCore

/**
 * Jembatan Lua -> C++ untuk MEMILIH MODE game dari Lua.
 *
 * Mode yang benar-benar dipakai C++ adalah `s_GameMode` + handler-nya (Mode4v4::init() dst.,
 * yang mengisi Cheats, enableGear, isHardCore, use4v4SpawnLayout). `_G.mode` di Lua hanya dipakai
 * SelectLayer.lua untuk tata letak layar pilih hero. Jadi layar mode buatan Lua WAJIB memanggil
 * fungsi ini, kalau tidak game tetap memakai mode lama (bawaan: Classic = 3v3).
 *
 *   GameModeApi.select(mode)  -> boolean   setara dengan baris akhir GameModeLayer::selectMode:
 *                                          s_GameMode = mode; handler->setOldCheats(Cheats); handler->init();
 *                                          (false kalau mode di luar jangkauan)
 *   GameModeApi.get()         -> integer   mode aktif di C++ (nilainya sama dengan tabel GameMode di Lua)
 *   GameModeApi.customSelect(mode) -> boolean  aturan "pilih tim sendiri" yang SAMA dengan C++
 *                                          (GameModeLayer::selectMode), termasuk flag enableCustomSelect dari
 *                                          kotak HardCore. Flag itu sekali pakai: dibaca lalu direset.
 *
 * Urutan di Lua (sama dengan C++): enterSelectLayer(mode, customSelect) lalu GameModeApi.select(mode).
 */
namespace GameModeSystem
{
	static int gmSelect(lua_State *L)
	{
		const lua_Integer v = luaL_checkinteger(L, 1);
		if (v < 0 || v >= (lua_Integer)GameMode::__Internal_Max_Length)
		{
			CCLOG("[GameModeSystem] mode %d di luar jangkauan", (int)v);
			lua_pushboolean(L, 0);
			return 1;
		}
		s_GameMode = (GameMode)v;
		auto handler = getGameModeHandler();
		handler->setOldCheats(Cheats);
		handler->init();
		lua_pushboolean(L, 1);
		return 1;
	}

	static int gmCustomSelect(lua_State *L)
	{
		const lua_Integer v = luaL_checkinteger(L, 1);
		const bool isRandomDM = v == (lua_Integer)GameMode::RandomDeathmatch;
		bool customSelect = false;
		if (Cheats < kMaxCheats && (v == (lua_Integer)GameMode::FourVsFour || v == (lua_Integer)GameMode::HardCore_4Vs4))
			customSelect = false;
		else if (!isRandomDM)
			customSelect = Cheats >= kMaxCheats;

		if (enableCustomSelect && !isRandomDM)
			customSelect = true;
		enableCustomSelect = false; // sekali pakai

		lua_pushboolean(L, customSelect ? 1 : 0);
		return 1;
	}

	static int gmGet(lua_State *L)
	{
		lua_pushinteger(L, (lua_Integer)s_GameMode);
		return 1;
	}

	static void registerLua()
	{
		if (!LuaBridge::isReady())
		{
			CCLOG("[GameModeSystem] Lua engine not ready, GameModeApi not registered");
			return;
		}

		lua_getL;
		lua_newtable(L);
		lua_pushcfunction(L, gmSelect);
		lua_setfield(L, -2, "select");
		lua_pushcfunction(L, gmGet);
		lua_setfield(L, -2, "get");
		lua_pushcfunction(L, gmCustomSelect);
		lua_setfield(L, -2, "customSelect");
		lua_setglobal(L, "GameModeApi");
	}
} // namespace GameModeSystem
