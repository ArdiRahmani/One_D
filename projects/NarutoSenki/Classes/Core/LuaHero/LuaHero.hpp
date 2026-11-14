#pragma once
#include "Core/Hero.hpp"
#include "Systems/LuaAISystem.hpp"

#include <string>

/**
 * Hero generik yang SELURUH AI-nya dijalankan dari Lua (tanpa class C++ per karakter).
 *
 * Provider::create() memakai class ini untuk nama AI apa pun yang tidak punya class C++
 * tetapi terdaftar di tabel `LuaAI` (lua/Hook/AI.lua), mis.  Guy = 'AI.Tsunade'.
 * Tidak ada nama karakter yang ditulis di C++ -- kunci registry = nama AI hasil resolve CloneAI.
 *
 * Handler Lua yang dipanggil (lihat LuaAISystem.hpp): perform, changeAction, resumeAction,
 * createClone. Fallback bila handler hilang/error (dicatat sekali di log, engine tidak crash):
 *   perform      -> checkHealingState()
 *   changeAction -> CharacterBase::changeAction()
 *   resumeAction -> CharacterBase::resumeAction(dt)
 *   createClone  -> nullptr
 */
class LuaHero : public Hero
{
	std::string _luaAi;

public:
	explicit LuaHero(const std::string &ai) : _luaAi(ai) {}

private:
	void perform() override
	{
		if (!LuaAISystem::callVoid(_luaAi, "perform", this))
			checkHealingState();
	}

	void changeAction() override
	{
		if (!LuaAISystem::callVoid(_luaAi, "changeAction", this))
			CharacterBase::changeAction();
	}

	void resumeAction(float dt) override
	{
		if (!LuaAISystem::callVoid(_luaAi, "resumeAction", this, dt))
			CharacterBase::resumeAction(dt);
	}

	Hero *createClone(int cloneTime) override
	{
		return LuaAISystem::callHero(_luaAi, "createClone", this, cloneTime);
	}
};
