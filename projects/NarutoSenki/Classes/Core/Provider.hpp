#pragma once

// Shinobi
#include "Shinobi/Asuma.hpp"
#include "Shinobi/Chiyo.hpp"
#include "Shinobi/Choji.hpp"
#include "Shinobi/Deidara.hpp"
#include "Shinobi/Gaara.hpp"
#include "Shinobi/Hashirama.hpp"
#include "Shinobi/Hidan.hpp"
#include "Shinobi/Hinata.hpp"
#include "Shinobi/Hiruzen.hpp"
#include "Shinobi/Ino.hpp"
#include "Shinobi/Itachi.hpp"
#include "Shinobi/Jiraiya.hpp"
#include "Shinobi/Jugo.hpp"
#include "Shinobi/Kakashi.hpp"
#include "Shinobi/Kakuzu.hpp"
#include "Shinobi/Kankuro.hpp"
#include "Shinobi/Kabuto.hpp"
#include "Shinobi/Karin.hpp"
#include "Shinobi/Kiba.hpp"
#include "Shinobi/Kimimaro.hpp"
#include "Shinobi/Kisame.hpp"
#include "Shinobi/Konan.hpp"
#include "Shinobi/Lee.hpp"
#include "Shinobi/Minato.hpp"
// #include "Shinobi/Nagato.hpp"
#include "Shinobi/Naruto.hpp"
#include "Shinobi/Neji.hpp"
#include "Shinobi/Orochimaru.hpp"
#include "Shinobi/Pakura.hpp"
#include "Shinobi/Pain.hpp"
#include "Shinobi/Sai.hpp"
#include "Shinobi/Sakura.hpp"
#include "Shinobi/Sasuke.hpp"
#include "Shinobi/Shikamaru.hpp"
#include "Shinobi/Shino.hpp"
#include "Shinobi/Suigetsu.hpp"
#include "Shinobi/Tenten.hpp"
#include "Shinobi/Tobi.hpp"
#include "Shinobi/Tobirama.hpp"
#include "Shinobi/Tsunade.hpp"
#include "LuaHero/LuaTsunade.hpp"
#include "LuaHero/LuaHero.hpp"
#include "Shinobi/Zetsu.hpp"

// Bunshin
#include "Shinobi/Bunshin/NarutoClone.hpp"
#include "Shinobi/Bunshin/SageNarutoClone.hpp"
#include "Shinobi/Bunshin/RikudoNarutoClone.hpp"

// Guardian
// Juubi
#include "Guardian/Guardian.hpp"

#include "Data/UnitDefines.h"

// Jutsu

// Kuchiyose
#include "Kuchiyose/Akamaru.hpp"
#include "Kuchiyose/Centipede.hpp"
#include "Kuchiyose/DogWall.hpp"
#include "Kuchiyose/Karasu.hpp"
#include "Kuchiyose/Kurama.hpp"
#include "Kuchiyose/Mask.hpp"
#include "Kuchiyose/Parents.hpp"
#include "Kuchiyose/Sanshouuo.hpp"
#include "Kuchiyose/Saso.hpp"
#include "Kuchiyose/Slug.hpp"
#include "Kuchiyose/WhiteZetsu.hpp"




#define __begin__ if(0) {}
#define is(var) else if (aiName == var)
#define is_or(var1, var2) else if (aiName == var1 || aiName == var2)
#define is_role(_Role) if (role == _Role)

// TODO: Impl a default AI logic
class DefaultAI : public Hero
{
	void perform() override
	{
	}
};

class Provider
{
public:
	static Hero *create(const string &name, Role role, Group group)
	{
		Hero *ptr;
		// CloneAI: nama AI/class yang dipakai. Default = name, bisa dialihkan dari Lua
		// (CloneAI.set("Madara", "Asuma")). `name` tetap dipakai untuk setID/resource.
		const string &aiName = AIRegistry::resolve(name);
		bool aiFallback = false;

		__begin__
		is("Akamaru") 						ptr = new Akamaru();
		is("Asuma") 						ptr = new Asuma();
		is("Centipede") 					ptr = new Centipede();
		is("Chiyo") 						ptr = new Chiyo();
		is("Choji") 						ptr = new Choji();
		is("Deidara") 						ptr = new Deidara();
		is("DogWall") 						ptr = new DogWall();
		is("Gaara") 						ptr = new Gaara();
		is_or("Hashirama", "SageHashirama") 	ptr = new Hashirama();
		is("Hidan") 						ptr = new Hidan();
		is("Hinata") 						ptr = new Hinata();
		is("Hiruzen") 						ptr = new Hiruzen();
		is("Ino") 							ptr = new Ino();
		is("Itachi") 						ptr = new Itachi();
		is_or("Jiraiya", "SageJiraiya") 	ptr = new Jiraiya();
		is("Jugo") 							ptr = new Jugo();
		is("Kakashi") 						ptr = new Kakashi();
		is("Kakuzu") 						ptr = new Kakuzu();
		is("Kankuro") 						ptr = new Kankuro();
		is("Karasu") 						ptr = new Karasu();
		is("Kabuto") 						ptr = new Kabuto();
		is("Karin") 						ptr = new Karin();
		is("Kiba") 							ptr = new Kiba();
		is("Kimimaro") 						ptr = new Kimimaro();
		is("Kisame") 						ptr = new Kisame();
		is("Konan") 						ptr = new Konan();
		is("Kurama") 						ptr = new Kurama();
		is_or("Lee", "RockLee") 			ptr = new Lee();
		is("MaskFuton") 					ptr = new Mask();
		is("MaskKaton") 					ptr = new Mask();
		is("MaskRaiton") 					ptr = new Mask();
		is("Minato") 						ptr = new Minato();
		is("Naruto") 						is_role(Role::Clone) ptr = new NarutoClone();		else ptr = new Naruto();
		is("RikudoNaruto") 					is_role(Role::Clone) ptr = new RikudoNarutoClone();	else ptr = new Naruto();
		is("SageNaruto") 					is_role(Role::Clone) ptr = new SageNarutoClone();	else ptr = new Naruto();
		is("Neji") 							ptr = new Neji();
		is("Orochimaru") 					ptr = new Orochimaru();
		is("Pakura") 					ptr = new Pakura();
		is_or("Pain", "Nagato") 			ptr = new Pain();
		is("AnimalPath") 					ptr = new AnimalPath();
		is("AsuraPath") 					ptr = new AsuraPath();
		is("NarakaPath") 					ptr = new NarakaPath();
		is("PertaPath") 					ptr = new PertaPath();
		is("HumanPath") 					ptr = new HumanPath();
		is("Sai") 							ptr = new Sai();
		is("Sakura") 						ptr = new Sakura();
		is("Sanshouuo") 					ptr = new Sanshouuo();
		is("Saso") 							ptr = new Saso();
		is_or("Sasuke", "ImmortalSasuke") 	ptr = new Sasuke();
		is("Shikamaru") 					ptr = new Shikamaru();
		is("Shino") 						ptr = new Shino();
		is("Slug") 							ptr = new Slug();
		is("Suigetsu") 						ptr = new Suigetsu();
		is("Tenten") 						ptr = new Tenten();
		is("Tobi") 							ptr = new Tobi();
		is("Tobirama") 					ptr = new Tobirama();
		is("Tsunade") 						ptr = LuaAISystem::has("Tsunade") ? static_cast<Hero *>(new LuaTsunade()) : static_cast<Hero *>(new Tsunade()); // Lua AI (lua/AI/Tsunade.lua) bila terdaftar, kalau tidak C++ asli
		is_or("Zetsu", " Zetsu2") 			ptr = new Zetsu();
		is("WhiteZetsu") 						ptr = new WhiteZetsu();
		is("Juubi")				ptr = new Guardian();
		else if (LuaAISystem::has(aiName))	ptr = new LuaHero(aiName); // AI 100% Lua untuk nama apa pun yang terdaftar di LuaAI (Hook/AI.lua)
		else								{ aiFallback = true; ptr = new DefaultAI(); }

		if (aiFallback && aiName != name)
		{
			CCLOG("[CloneAI] AI [ %s ] untuk [ %s ] tidak ditemukan, pakai DefaultAI", aiName.c_str(), name.c_str());
		}

		if (!ptr)
		{
			CCLOG("Not found character [ %s ]", name.c_str());
			return nullptr;
		}

		if (ptr->init())
		{
			ptr->setID(name, role, group);
			ptr->autorelease();
		}
		else
		{
			CCLOG("Set character %s not found", name.c_str());
			delete ptr;
			ptr = nullptr;
		}
		return ptr;
	}
};

#undef __begin__
#undef is
#undef is_or
#undef is_role
