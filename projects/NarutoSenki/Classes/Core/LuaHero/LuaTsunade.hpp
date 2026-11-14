#pragma once
#include "Shinobi/Tsunade.hpp"
#include "Systems/LuaAISystem.hpp"

/**
 * Tsunade yang logikanya dijalankan dari Lua (lua/AI/Tsunade.lua).
 *
 * Tsunade.hpp TIDAK diubah. Semua method-nya (perform, changeAction, resumeAction, createClone)
 * bersifat private + virtual, jadi class ini cukup meng-override keempatnya dan meneruskan
 * pemanggilan ke handler Lua yang terdaftar di tabel `LuaAI` dengan kunci "Tsunade".
 *
 * Provider::create() memilih class ini HANYA bila `LuaAI.has("Tsunade")`; kalau tidak,
 * `new Tsunade()` (versi C++ asli) dipakai seperti sebelumnya. Jadi C++ asli tetap utuh
 * sebagai fallback engine: cukup `LuaAI.set('Tsunade', nil)` untuk kembali ke C++.
 *
 * Fallback bila handler Lua hilang/error (tercatat sekali di log, engine tidak crash):
 *   perform      -> checkHealingState() (idle / maju sepanjang lane)
 *   changeAction -> tidak melakukan apa-apa
 *   resumeAction -> CharacterBase::resumeAction(dt)
 *   createClone  -> nullptr (engine menulis log "can not create clone")
 */
class LuaTsunade : public Tsunade
{
	static const std::string &ai()
	{
		static const std::string kName = "Tsunade";
		return kName;
	}

	void perform() override
	{
		if (!LuaAISystem::callVoid(ai(), "perform", this))
			checkHealingState();
	}

	void changeAction() override
	{
		LuaAISystem::callVoid(ai(), "changeAction", this);
	}

	void resumeAction(float dt) override
	{
		if (!LuaAISystem::callVoid(ai(), "resumeAction", this, dt))
			CharacterBase::resumeAction(dt);
	}

	Hero *createClone(int cloneTime) override
	{
		return LuaAISystem::callHero(ai(), "createClone", this, cloneTime);
	}
};
