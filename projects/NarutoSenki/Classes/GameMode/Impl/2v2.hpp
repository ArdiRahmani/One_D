#pragma once
#include "GameMode/IGameModeHandler.hpp"

class Mode2v2 : public IGameModeHandler
{
public:
	void init()
	{
		CCLOG("Enter 2 VS 2 mode.");

		gd.isHardCore = true;
	}

	void onInitHeros()
	{
		initHeros(2, 2);
	}

	void onGameStart()
	{
	}

	void onGameOver()
	{
	}

	void onCharacterInit(CharacterBase *c)
	{
	}

	void onCharacterDead(CharacterBase *c)
	{
	}

	void onCharacterReborn(CharacterBase *c)
	{
	}
};
