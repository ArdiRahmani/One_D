#pragma once
#include "Systems/AliasBridge.hpp"
#include "Systems/AssetSystem.hpp"
#include "Systems/CommandSystem.hpp"
#include "Systems/HeroSystem.hpp"
#include "Systems/IconSystem.hpp"
#include "Systems/MonsterAISystem.hpp"
#include "Systems/MonsterSystem.hpp"
#include "Systems/ProviderSystem.hpp"
#include "Systems/TransformSystem.hpp"

namespace Internal
{
	static void initAllSystems()
	{
		CommandSystem::reset();
		// Harus sebelum main.lua: lua/Hook/Heroes.lua mengisi daftar hero lewat tabel ini
		HeroSystem::registerLua();
		// Harus sebelum main.lua: lua/Hook/CloneAI.lua mengisi alias AI lewat tabel `CloneAI`
		ProviderSystem::registerLua();
		// Harus sebelum main.lua: lua/Hook/Monster.lua mengisi alias AI monster lewat tabel `CloneMon`
		MonsterSystem::registerLua();
		// Harus sebelum main.lua: lua/Hook/MonsterAI.lua mengisi alias Monster::setAI lewat tabel `CloneMonAI`
		MonsterAISystem::registerLua();
		// Harus sebelum main.lua: lua/Hook/Bullet.lua mengisi alias peluru lewat tabel `CloneBullet`
		AliasBridge<BulletRegistry::map>::registerLua("CloneBullet");
		// Harus sebelum main.lua: lua/Hook/Catch.lua mengisi alias master tangkapan lewat tabel `CloneCatch`
		AliasBridge<CatchRegistry::map>::registerLua("CloneCatch");
		// Harus sebelum main.lua: lua/Hook/Transform.lua mengisi daftar transformasi lewat tabel `Transform`
		TransformSystem::registerLua();
		// Harus sebelum main.lua: lua/Hook/Assets.lua mengisi aset per karakter lewat tabel `Assets`
		AssetSystem::registerLua();
		// Harus sebelum main.lua: lua/Hook/Icons.lua mengisi binding _rp/_rpf/_CutIn/_avator/_small lewat tabel `Icons`
		IconSystem::registerLua();
	}
} // namespace Internal
