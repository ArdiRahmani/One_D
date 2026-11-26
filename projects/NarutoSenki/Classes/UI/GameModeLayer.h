#pragma once
#include "GameMode/GameModeImpl.h"

/**
 * GameModeLayer
 *
 * Tampilan (UI) layer ini sudah dipindah ke Lua: lua/ui/GameModeLayer.lua
 * Sisi C++ hanya tinggal "binding" yang dipanggil dari Lua (lihat LuaCocos2d.cpp):
 *
 *   GameModeLayer:create()           -> layer kosong (UI dibangun di GameModeLayer:init() Lua)
 *   GameModeLayer:getModeCount()     -> jumlah mode
 *   GameModeLayer:getModeImage(mode) -> path gambar tombol mode (fallback GameMode/Mode.png)
 *   GameModeLayer:startMode(mode)    -> konfirmasi mode: set s_GameMode, buka SelectLayer
 *   GameModeLayer:setCheats(n)       -> set variabel global Cheats
 */
class GameModeLayer : public Layer
{
public:
	bool init();

	static int getModeCount();
	static const char *getModeImage(int mode);
	static void startMode(int mode);
	static void setCheats(int cheats);

	CREATE_FUNC(GameModeLayer);
};
