#include <algorithm>
#include "StartMenu.h"
#include "UI/GameModeLayer.h"
#include "UI/ModeMenuButton.hpp"
#include "Constants/UiFlowKeys.hpp"

// Flag dari StartMenu.cpp (tombol Yes di kotak HardCore)
extern bool enableCustomSelect;

extern const GameData kDefaultGameData;

// Map yang dipilih pemain (1 = 1.tmx, dst). Dibaca oleh GameLayer::initTileMap().
extern int s_SelectedMapId;

// Hitung jumlah Maps/N.tmx yang ada (N berurutan mulai dari 1)
static int countMaps()
{
	auto fileUtils = CCFileUtils::sharedFileUtils();
	int n = 0;
	while (fileUtils->isFileExist(format("Maps/{}.tmx", n + 1).c_str()))
		n++;
	return n;
}

// const GameModeData &GameModeData::from(const char *path)
// {
//     GameModeData data = {}
//     return data;
// }

// Gambar tombol tiap mode (urut sesuai enum GameMode).
// 2v2 memakai GameMode/2v2.png; kalau file itu belum ada, sementara memakai gambar Boss lama (5.png).
static string getModeImage(GameMode mode)
{
	static const char *kImages[] = {
		"GameMode/1v1.png",	// OneVsOne
		"GameMode/2v2.png", // TwoVsTwo
		"GameMode/3v3.png",	// Classic
		"GameMode/4v4.png",	// FourVsFour
		"GameMode/HardCore.png",	// HardCore_4Vs4
		"GameMode/Clone.png",	// Clone
		"GameMode/Deathmatch.png",	// Deathmatch
		"GameMode/RandomDeathmatch.png",	// RandomDeathmatch
	};
	string path = kImages[(size_t)mode];
	if (!CCFileUtils::sharedFileUtils()->isFileExist(CCFileUtils::sharedFileUtils()->fullPathForFilename(path.c_str())))
		path = "GameMode/Mode.png";
	return path;
}

bool GameModeLayer::init()
{
	auto bgSprite = Sprite::create("red_bg.png");
	FULL_SCREEN_SPRITE(bgSprite);
	bgSprite->setAnchorPoint(Vec2(0, 0));
	bgSprite->setPosition(Vec2(0, 0));
	addChild(bgSprite, -100);

	// menu bars
	auto menu_bar_b = Sprite::create("menu_bar2.png");
	menu_bar_b->setAnchorPoint(Vec2(0, 0));
	FULL_SCREEN_SPRITE(menu_bar_b);
	addChild(menu_bar_b, 2);

	auto menu_bar_t = Sprite::create("menu_bar3.png");
	menu_bar_t->setAnchorPoint(Vec2(0, 0));
	menu_bar_t->setPosition(Vec2(0, winSize.height - menu_bar_t->getContentSize().height));
	FULL_SCREEN_SPRITE(menu_bar_t);
	addChild(menu_bar_t, 2);

	auto modemenu_title = Sprite::createWithSpriteFrameName("startmenu_title.png");
	modemenu_title->setAnchorPoint(Vec2(0, 0));
	modemenu_title->setPosition(Vec2(2, winSize.height - modemenu_title->getContentSize().height - 2));
	addChild(modemenu_title, 3);

	initModeData();

	// init menus (semua tombol dibuat sekali, posisinya diatur per halaman di showPage)
	for (size_t i = 0; i < (size_t)GameMode::__Internal_Max_Length; i++)
	{
		auto mode_btn = ModeMenuButton::create(getModeImage((GameMode)i));
		mode_btn->mode = (GameMode)i;
		mode_btn->setDelegate(this);
		menuButtons[i] = mode_btn;
		addChild(mode_btn);
	}

	for (size_t i = 0; i < menuButtons.size(); i++)
	{
		if (modes[i].isLocked)
		{
			menuButtons.at(i)->useMask2 = modes.at(i).useMask2;
			menuButtons.at(i)->lock();
		}
	}

	menuLabel = CCLabelTTF::create();
	menuLabel->setAnchorPoint(Vec2(0, 0));
	menuLabel->setPosition(Vec2(10, 2));
	addChild(menuLabel, 5);

	// tombol Page Prev / Page Next (ikon sama seperti SelectLayer.lua: Img/Prev.png & Img/Next.png)
	const float arrowY = winSize.height / 2 + 30;

	auto prev_btn = MenuItemSprite::create(Sprite::create("Img/Prev.png"), Sprite::create("Img/Prev_1.png"), nullptr, this, menu_selector(GameModeLayer::prevPage));
	prev_btn->setAnchorPoint(Vec2(0.5f, 0.5f));
	prev_btn->setScale(0.60f);
	prev_btn->setPosition(Vec2(45, arrowY));
	auto mvPrev = MoveBy::create(1.0f, Vec2(-8, 0));
	prev_btn->runAction(RepeatForever::create(Sequence::createWithTwoActions(mvPrev, mvPrev->reverse())));

	auto next_btn = MenuItemSprite::create(Sprite::create("Img/Next.png"), Sprite::create("Img/Next_1.png"), nullptr, this, menu_selector(GameModeLayer::nextPage));
	next_btn->setAnchorPoint(Vec2(0.5f, 0.5f));
	next_btn->setScale(0.60f);
	next_btn->setPosition(Vec2(winSize.width - 45, arrowY));
	auto mvNext = MoveBy::create(1.0f, Vec2(8, 0));
	next_btn->runAction(RepeatForever::create(Sequence::createWithTwoActions(mvNext, mvNext->reverse())));

	Menu *page_menu = Menu::create(prev_btn, next_btn, nullptr);
	page_menu->setAnchorPoint(Vec2(0, 0));
	page_menu->setPosition(Vec2(0, 0));
	addChild(page_menu, 10);

	showPage(0);

	// pilih map: label "MapsN" di bawah gambar mode + tombol prev/next
	mapCount = std::max(1, countMaps());
	if (s_SelectedMapId < 1 || s_SelectedMapId > mapCount)
		s_SelectedMapId = 1;

	float maxBtnH = 0;
	for (auto btn : menuButtons)
		maxBtnH = std::max(maxBtnH, btn->getContentSize().height);
	const float mapY = (winSize.height / 2) + 30 - maxBtnH / 2 - 40;

	mapLabel = CCLabelTTF::create("Maps1", "Fonts/njNaruto.ttf", 42);
	mapLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
	mapLabel->setPosition(Vec2(winSize.width / 2, mapY));
	addChild(mapLabel, 5);

	const float mapArrowOffset = 150;
	auto map_prev = MenuItemSprite::create(Sprite::create("Img/Prev.png"), Sprite::create("Img/Prev_1.png"), nullptr, this, menu_selector(GameModeLayer::prevMap));
	map_prev->setScale(0.45f);
	map_prev->setPosition(Vec2(winSize.width / 2 - mapArrowOffset, mapY));
	auto map_next = MenuItemSprite::create(Sprite::create("Img/Next.png"), Sprite::create("Img/Next_1.png"), nullptr, this, menu_selector(GameModeLayer::nextMap));
	map_next->setScale(0.45f);
	map_next->setPosition(Vec2(winSize.width / 2 + mapArrowOffset, mapY));
	Menu *map_menu = Menu::create(map_prev, map_next, nullptr);
	map_menu->setAnchorPoint(Vec2(0, 0));
	map_menu->setPosition(Vec2(0, 0));
	addChild(map_menu, 10);
	updateMapLabel();

	// init return button
	auto return_img = MenuItemSprite::create(Sprite::create("UI/return_btn.png"), nullptr, nullptr, this, menu_selector(GameModeLayer::backToMenu));
	Menu *return_btn = Menu::create(return_img, nullptr);
	return_btn->setAnchorPoint(Vec2(1, 0.5f));
	return_btn->setPosition(winSize.width - 38, 65);
	addChild(return_btn, 5);

	return Layer::init();
}

void GameModeLayer::backToMenu(Ref *sender)
{
	SimpleAudioEngine::sharedEngine()->playEffect("Audio/Menu/cancel.ogg");

	auto menuScene = Scene::create();
	auto menuLayer = StartMenu::create();
	menuScene->addChild(menuLayer);
	Director::sharedDirector()->replaceScene(TransitionFade::create(1.0f, menuScene));
}

int GameModeLayer::getPageCount() const
{
	const int total = (int)GameMode::__Internal_Max_Length;
	return (total + kModesPerPage - 1) / kModesPerPage;
}

void GameModeLayer::prevPage(Ref *sender)
{
	const int count = getPageCount();
	showPage((currentPage + count - 1) % count); // loop ke halaman terakhir
}

void GameModeLayer::nextPage(Ref *sender)
{
	const int count = getPageCount();
	showPage((currentPage + 1) % count); // loop ke halaman pertama
}

void GameModeLayer::updateMapLabel()
{
	if (mapLabel)
		mapLabel->setString(format("Maps{}", s_SelectedMapId).c_str());
}

void GameModeLayer::prevMap(Ref *sender)
{
	SimpleAudioEngine::sharedEngine()->playEffect("Audio/Menu/confirm.ogg");
	s_SelectedMapId = (s_SelectedMapId - 2 + mapCount) % mapCount + 1; // Maps1 -> map terakhir
	updateMapLabel();
}

void GameModeLayer::nextMap(Ref *sender)
{
	SimpleAudioEngine::sharedEngine()->playEffect("Audio/Menu/confirm.ogg");
	s_SelectedMapId = s_SelectedMapId % mapCount + 1; // map terakhir -> Maps1
	updateMapLabel();
}

void GameModeLayer::showPage(int page)
{
	const int total = (int)GameMode::__Internal_Max_Length;
	currentPage = page;

	const int first = page * kModesPerPage;
	const int last = std::min(first + kModesPerPage, total); // halaman terakhir bisa kurang dari 3

	// lebar tombol berbeda-beda, jadi spasi dihitung dari lebar sebenarnya
	const float gap = 20.0f;
	float totalWidth = 0;
	for (int i = first; i < last; i++)
		totalWidth += menuButtons[i]->getContentSize().width;
	totalWidth += gap * (last - first - 1);

	const float posY = (winSize.height / 2) + 30;
	float x = (winSize.width - totalWidth) / 2;

	for (int i = 0; i < total; i++)
		menuButtons[i]->setVisible(i >= first && i < last);

	for (int i = first; i < last; i++)
	{
		auto btn = menuButtons[i];
		const float w = btn->getContentSize().width;
		btn->setPosition(Vec2(x + w / 2, posY));
		x += w + gap;
	}

	if (menuLabel)
		menuLabel->setString("");
}

void GameModeLayer::initModeData()
{
	// init mode text data
	auto lang = Application::sharedApplication()->getCurrentLanguage();
	if (lang == LanguageType::kLanguageChinese)
	{
		modes[GameMode::OneVsOne] = {"1 VS 1", ""};
		modes[GameMode::TwoVsTwo] = {"2 VS 2", ""};
		modes[GameMode::Classic] = {"3 VS 3", "经典模式"};
		modes[GameMode::FourVsFour] = {"4 VS 4", ""};
		modes[GameMode::HardCore_4Vs4] = {"硬核模式 (4 VS 4)", "禁用装备"};
		modes[GameMode::Clone] = {"克隆模式 (3 VS 3)", ""};
		modes[GameMode::Deathmatch] = {"死亡竞赛 (3 VS 3)", ""};
		modes[GameMode::RandomDeathmatch] = {"随机死亡竞赛 (3 VS 3)", ""};
	}
	else // English
	{
		modes[GameMode::OneVsOne] = {"1 VS 1", ""};
		modes[GameMode::TwoVsTwo] = {"2 VS 2", ""};
		modes[GameMode::Classic] = {"3 VS 3", "Classic Mode"};
		modes[GameMode::FourVsFour] = {"4 VS 4", ""};
		modes[GameMode::HardCore_4Vs4] = {"HardCore (4 VS 4)", "Disabled gear"};
		modes[GameMode::Clone] = {"Clone (3 VS 3)", ""};
		modes[GameMode::Deathmatch] = {"Deathmatch (3 VS 3)", ""};
		modes[GameMode::RandomDeathmatch] = {"Random Deathmatch (3 VS 3)", ""};
	}

	// init in developtment game modes
	modes[GameMode::Deathmatch].isLocked = true;
	modes[GameMode::Deathmatch].useMask2 = true;

	// init mode handlers
	for (size_t i = 0; i < GameMode::__Internal_Max_Length; i++)
	{
		auto &data = modes.at(i);
		if (data.isLocked)
			data.description += " (In developtment)";
		// data.handler = s_ModeHandlers[i];
	}
}

bool GameModeLayer::pushMode(const GameModeData &data)
{
	return true;
}

void GameModeLayer::removeMode(const GameModeData &data)
{
}

void GameModeLayer::selectMode(GameMode mode)
{
	auto data = modes[(size_t)mode];
	string label = data.title;
	if (data.description.size() > 0)
	{
		label += " | ";
		label += data.description;
	}
	menuLabel->setString(label.c_str());

	if (setSelect(mode))
	{
		for (size_t i = 0; i < modes.size(); i++)
			modes.at(i).isLocked = true;
		CCLOG("Selected %s mode", data.title.c_str());

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

		// call lua global function StartMenu.enterSelectLayer
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
}

bool GameModeLayer::setSelect(GameMode mode)
{
	auto &data = modes.at(mode);
	if (!data.isLocked && data.hasSelected)
		return true;

	for (size_t i = 0; i < modes.size(); i++)
		modes.at(i).hasSelected = false;
	data.hasSelected = true;
	return false;
}
