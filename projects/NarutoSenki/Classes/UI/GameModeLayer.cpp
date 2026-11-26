#include "StartMenu.h"
#include "UI/GameModeLayer.h"
#include "Constants/UiFlowKeys.hpp"

// Flag dari StartMenu.cpp (tombol Yes di kotak HardCore)
extern bool enableCustomSelect;

// UI dibangun oleh Lua (lua/ui/GameModeLayer.lua -> GameModeLayer:init()),
// dipanggil lewat hook.registerInitHandlerOnly(layer).
bool GameModeLayer::init()
{
	return Layer::init();
}

int GameModeLayer::getModeCount()
{
	return (int)GameMode::__Internal_Max_Length;
}

// Gambar tombol tiap mode (urut sesuai enum GameMode).
// Kalau file gambarnya belum ada, memakai GameMode/Mode.png.
const char *GameModeLayer::getModeImage(int mode)
{
	static const char *kImages[] = {
		"GameMode/1v1.png",			   // OneVsOne
		"GameMode/2v2.png",			   // TwoVsTwo
		"GameMode/3v3.png",			   // Classic
		"GameMode/4v4.png",			   // FourVsFour
		"GameMode/HardCore.png",	   // HardCore_4Vs4
		"GameMode/Clone.png",		   // Clone
		"GameMode/Deathmatch.png",	   // Deathmatch
		"GameMode/RandomDeathmatch.png", // RandomDeathmatch
	};
	static const char *kFallback = "GameMode/Mode.png";

	if (mode < 0 || mode >= getModeCount())
		return kFallback;

	auto fu = CCFileUtils::sharedFileUtils();
	if (!fu->isFileExist(fu->fullPathForFilename(kImages[mode]).c_str()))
		return kFallback;
	return kImages[mode];
}

void GameModeLayer::setCheats(int cheats)
{
	Cheats = cheats;
}

// Dipanggil Lua saat mode sudah dikonfirmasi (tap kedua pada mode yang sama).
void GameModeLayer::startMode(int modeId)
{
	if (modeId < 0 || modeId >= getModeCount())
		return;

	GameMode mode = (GameMode)modeId;
	CCLOG("Selected mode %d", modeId);

	s_GameMode = mode;
	bool customSelect = false;
	if (Cheats < kMaxCheats && (mode == GameMode::FourVsFour || mode == GameMode::HardCore_4Vs4))
	{
		customSelect = false;
	}
	else if (mode != GameMode::RandomDeathmatch)
	{
		customSelect = Cheats >= kMaxCheats;
	}

	// Dari tombol Yes di kotak HardCore (StartMenu): aktifkan pilih tim
	if (enableCustomSelect && mode != GameMode::RandomDeathmatch)
	{
		customSelect = true;
	}
	enableCustomSelect = false; // sekali pakai

	// call lua global function enterSelectLayer(mode, customSelect)
	auto pStack = get_luastack;
	auto state = pStack->getLuaState();
	lua_getglobal(state, UiFlowKeys::kEnterSelectLayer);
	pStack->pushInt(mode);
	pStack->pushBoolean(customSelect);
	pStack->executeFunction(2);

	auto handler = getGameModeHandler();
	handler->setOldCheats(Cheats);
	handler->init();
}
