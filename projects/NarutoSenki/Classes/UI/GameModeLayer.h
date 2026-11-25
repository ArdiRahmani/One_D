#pragma once
#include "GameMode/GameModeImpl.h"

class ModeMenuButton;

class GameModeLayer : public Layer
{
public:
	static const int kMenuCount = 5;
	// Jumlah mode yang tampil per halaman
	static const int kModesPerPage = 3;

	bool init();
	void backToMenu(Ref *sender);

	void initModeData();
	bool pushMode(const GameModeData &data);
	void removeMode(const GameModeData &data);
	void selectMode(GameMode mode);

	// Navigasi halaman (loop): prev dari halaman pertama -> halaman terakhir,
	// next dari halaman terakhir -> halaman pertama
	void prevPage(Ref *sender = nullptr);
	void nextPage(Ref *sender = nullptr);
	void showPage(int page);
	int getPageCount() const;

	CREATE_FUNC(GameModeLayer);

private:
	inline bool setSelect(GameMode mode);

	int currentPage = 0;

	CCLabelTTF *menuLabel = nullptr;

	vector<ModeMenuButton *> menuButtons = vector<ModeMenuButton *>(static_cast<size_t>(GameMode::__Internal_Max_Length));
	vector<GameModeData> modes = vector<GameModeData>(static_cast<size_t>(GameMode::__Internal_Max_Length));
};
