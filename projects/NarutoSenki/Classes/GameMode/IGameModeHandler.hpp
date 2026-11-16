#pragma once
#include "Core/Hero.hpp"
#include "GameLayer.h"
#include "LoadLayer.h"
#include "SelectLayer.h"

class IGameModeHandler;

enum GameMode : uint32_t
{
	// 1 VS 1 // TODO: As a test mode
	OneVsOne,
	// 3 VS 3
	Classic,
	// 4 VS 4
	FourVsFour,
	// 4 VS 4 (Disabled gears)
	HardCore_4Vs4,
	// 3 VS 1
	Boss,
	// 3 VS 3
	Clone,
	// TODO：3 VS 3
	Deathmatch,
	// 3 VS 3
	RandomDeathmatch,

	__Internal_Max_Length
}; // namespace GameMode

struct GameModeData
{
public:
	string title;
	string description;

	bool isLocked;
	bool useMask2;

	bool hasSelected;
	// IGameModeHandler *handler;

	// static const GameModeData &from(const char *path);
};

struct GameData
{
	bool enableGear = true;
	bool isHardCore = true;
	bool isRandomChar = false;

	bool use4v4SpawnLayout = false;

	Group playerGroup;
};

class IGameModeHandler
{
	friend class GameLayer;
	friend class GameModeLayer;
	friend class SelectLayer;

private:
	vector<HeroData> heroDataVector;
	// map
	int mapId = 0;
	// tower
	bool enableKonohaTowers = true;
	bool enableAkatsukiTowers = true;
	// flog
	bool skipInitFlogs = false;
	bool enableKonohaFlogs = false;
	bool enableAkatsukiFlogs = false;
	float flogSpawnDuration = 15.0f;
	bool isLimitFlog = false;
	int maxFlogWaves;
	// hero
	bool enableHeroReborn = true;

	void Internal_GameOver()
	{
		onGameOver();

		gd = {};
		clearHeroArray();
		resetCheats();
	}

protected:
	SelectLayer *selectLayer = nullptr;
	// game
	GameData gd;
	int oldCheats = -1;
	vector<string> heroVector;
	// player
	Group playerGroup;
	// penyimpan nama hero pemain yang diacak (dipakai _playerSelect)
	string _randomPlayerName;

public:
	const uint8_t kMaxCharCount = 4;
	const int kDefaultMap = 1;
	const Group kDefaultGroup = Group::Konoha;

	virtual void init() = 0;

	virtual void onInitHeros() = 0;
	virtual void onGameStart() = 0;
	virtual void onGameOver() = 0;

	// callbacks
	virtual void onCharacterInit(CharacterBase *c) = 0;
	virtual void onCharacterDead(CharacterBase *c) = 0;
	virtual void onCharacterReborn(CharacterBase *c) = 0;

	inline const GameData &getGameData() { return gd; }
	inline int getOldCheats() { return oldCheats; }
	inline void setOldCheats(int val) { oldCheats = val; }
	inline void resetCheats()
	{
		if (oldCheats != -1)
		{
			Cheats = oldCheats;
			oldCheats = -1;
		}
	}
	inline const vector<HeroData> &getHerosArray() { return heroDataVector; }

protected:
	// IGameModeHandler()
	// {
	// 	oldCheats = Cheats;
	// }

	void clearHeroArray()
	{
		heroDataVector.clear();
		heroVector.clear();
	}

	// Warpper of game layer
	void setMap(int id)
	{
		this->mapId = id;
	}

	void setTower(bool enableKonohaTowers, bool enableAkatsukiTowers)
	{
		this->enableKonohaTowers = enableKonohaTowers;
		this->enableAkatsukiTowers = enableAkatsukiTowers;
	}

	void setFlog(bool enableKonohaFlogs, bool enableAkatsukiFlogs, float duration = 15.0f, int maxWaves = -1)
	{
		this->enableKonohaFlogs = enableKonohaFlogs;
		this->enableAkatsukiFlogs = enableAkatsukiFlogs;
		this->flogSpawnDuration = (duration < 5 || duration > 60) ? 15.0f : duration;
		this->isLimitFlog = maxWaves >= 0;
		this->maxFlogWaves = maxWaves;
	}

	void setHero(bool enableReborn)
	{
		this->enableHeroReborn = enableReborn;
	}

	// init hero
	inline void addHero(const char *name, Role role, Group group, uint32_t lv = 1)
	{
		// Nama disesuaikan dengan page skill yang dipilih (mis. Guy page 2 => Guy8Gates)
		string finalName = resolveHeroVariant(name);
		heroDataVector.push_back({finalName, role, group});
		heroVector.push_back(finalName);
	}

	inline void addHeros(int count, const char *name, Role role, Group group, uint32_t lv = 1)
	{
		for (int i = 0; i < count; i++)
			addHero(name, role, group, lv);
	}

	inline void addKonohaHero(const vector<string> &heros, const char *name, Role role, uint32_t lv = 1)
	{
		addHero(name, role, Group::Konoha, lv);
	}

	inline void addAkatsukiHero(const vector<string> &heros, const char *name, Role role, uint32_t lv = 1)
	{
		addHero(name, role, Group::Akatsuki, lv);
	}

	inline void initHeros(uint32_t playerCount, uint32_t enemyCount)
	{
		initHeros(playerCount, enemyCount, nullptr, kDefaultGroup);
	}

	void initHeros(uint8_t playerCount, uint8_t enemyCount,
				   const char *playerSelect, Group playerGroup,
				   const char *com1Select = nullptr, const char *com2Select = nullptr, const char *com3Select = nullptr)
	{
		clearHeroArray();

		playerCount = playerCount > kMaxCharCount ? kMaxCharCount : playerCount;
		enemyCount = enemyCount > kMaxCharCount ? kMaxCharCount : enemyCount;
		setRand();
		int team = random(2);
		// TODO: Support custom player group
		// playerGroup = playerGroup == nullptr ? (team > 0 ? Group::Konoha : Group::Akatsuki) : playerGroup;
		playerGroup = team > 0 ? Group::Akatsuki : Group::Konoha;
		this->gd.playerGroup = playerGroup;
		this->playerGroup = playerGroup;
		uint8_t totalCount = playerCount + enemyCount;
		uint8_t comCount = totalCount > 0 ? totalCount - 1 : 0;
		uint8_t comOfPlayerGroupCount = playerCount > 0 ? playerCount - 1 : 0;

		if (playerSelect)
			selectLayer->_playerSelect = playerSelect;
		if (com1Select)
			selectLayer->_com1Select = com1Select;
		if (com2Select)
			selectLayer->_com2Select = com2Select;
		if (com3Select)
			selectLayer->_com3Select = com3Select;

		// Pilihan musuh dari layar pilih (SelectLayer.lua menyimpannya di UserDefault)
		string customEnemy[4];
		for (int j = 0; j < 4; j++)
			customEnemy[j] = takeCustomEnemy(j);

		// init player hero
		string tmpChar;
		if (selectLayer->_playerSelect)
		{
			tmpChar = selectLayer->_playerSelect;
		}
		else
		{
			auto selcetList = selectLayer->_selectList;
			int count = selcetList->count();
			setRand();
			int index = random(count);
			tmpChar = ((CCString *)selcetList->objectAtIndex(index))->m_sString;
			do
			{
				setRand();
				index = random(count);
				tmpChar = ((CCString *)selcetList->objectAtIndex(index))->m_sString;
			} while (tmpChar == "None");

			// Simpan di member: sebelumnya _playerSelect menunjuk ke string lokal (tmpChar)
			// yang sudah hancur begitu fungsi ini selesai (dangling pointer).
			_randomPlayerName = tmpChar;
			selectLayer->_playerSelect = _randomPlayerName.c_str();
			gd.isRandomChar = true;
		}

		// push player (page skill 2 => nama varian, mis. Guy8Gates)
		tmpChar = resolveHeroVariant(tmpChar);
		heroDataVector.push_back({tmpChar, Role::Player, playerGroup});
		heroVector.push_back(tmpChar);

		// init com heros
		vector<string> realHeroVector;
		for (size_t i = 0; i < HeroRegistry::count(); i++)
		{
			const char *heroName = HeroRegistry::at(i);
			if (is_same(selectLayer->_playerSelect, heroName))
				continue;
			if (selectLayer->_com1Select && is_same(selectLayer->_com1Select, heroName))
				continue;
			if (selectLayer->_com2Select && is_same(selectLayer->_com2Select, heroName))
				continue;
			if (selectLayer->_com3Select && is_same(selectLayer->_com3Select, heroName))
				continue;
			bool isCustomEnemy = false;
			for (int j = 0; j < 4; j++)
				if (!customEnemy[j].empty() && customEnemy[j] == heroName)
					isCustomEnemy = true;
			if (isCustomEnemy)
				continue;

			realHeroVector.push_back(heroName);
		}

		// Ambil satu hero acak dari sisa pool (lalu dibuang dari pool).
		// Kalau pool habis (mis. daftar hero di Lua sedikit) boleh duplikat daripada crash.
		auto takeRandomHero = [&]() -> string
		{
			if (realHeroVector.empty())
				return getRandomHero();
			setRand();
			size_t index = random(realHeroVector.size());
			string picked = realHeroVector[index];
			realHeroVector.erase(realHeroVector.begin() + index);
			return picked;
		};

		string hero;
		for (int i = 0; i < comCount; i++)
		{
			if (i < comOfPlayerGroupCount)
			{
				if (i == 0 && selectLayer->_com1Select)
				{
					hero = selectLayer->_com1Select;
				}
				else if (i == 1 && selectLayer->_com2Select)
				{
					hero = selectLayer->_com2Select;
				}
				else if (i == 2 && selectLayer->_com3Select)
				{
					hero = selectLayer->_com3Select;
				}
				else
				{
					hero = takeRandomHero();
				}

				// COM rekan tim / hero acak: ikut page skill yang dipilih
				hero = resolveHeroVariant(hero);
				heroDataVector.push_back({hero, Role::Com, team > 0 ? Group::Akatsuki : Group::Konoha});
				heroVector.push_back(hero);
			}
			else
			{
				int enemyIndex = i - comOfPlayerGroupCount;
				if (enemyIndex >= 0 && enemyIndex < 4 && !customEnemy[enemyIndex].empty())
				{
					// musuh pilihan dari layar pilih
					hero = customEnemy[enemyIndex];
				}
				else
				{
					hero = takeRandomHero();
				}
				// Musuh pilihan / hero acak: ikut page skill yang dipilih
				hero = resolveHeroVariant(hero);
				heroDataVector.push_back({hero, Role::Com, team > 0 ? Group::Konoha : Group::Akatsuki});
				heroVector.push_back(hero);
			}
		}
	}

	inline void setPlayerTeamByGroup(Group group)
	{
		gd.playerGroup = group;
	}

	/**
	 * Static Utils
	 */

	static inline const char *getRandomHero()
	{
		// Daftar hero diisi dari Lua (lua/Hook/Heroes.lua)
		const size_t total = HeroRegistry::count();
		if (total == 0)
			return "Naruto";
		setRand();
		return HeroRegistry::at(random(total));
	}

	static inline const char *getRandomHeroExcept(const char *except)
	{
		const size_t total = HeroRegistry::count();
		if (total == 0)
			return "Naruto";
		setRand();
		if (!except)
			return HeroRegistry::at(random(total));

		// Pilih dari kandidat yang bukan `except` (tidak ada loop tak berujung
		// kalau daftar hero hanya berisi `except`)
		vector<const char *> candidates;
		for (size_t i = 0; i < total; i++)
		{
			const char *name = HeroRegistry::at(i);
			if (!is_same(name, except))
				candidates.push_back(name);
		}
		if (candidates.empty())
			return HeroRegistry::at(random(total));
		return candidates[random(candidates.size())];
	}

	static inline const char *getRandomHeroExceptAll(const vector<string> &excepts, const char *defaultChar = "Naruto")
	{
		const size_t total = HeroRegistry::count();
		vector<const char *> candidates;
		for (size_t i = 0; i < total; i++)
		{
			const char *name = HeroRegistry::at(i);
			if (std::find(excepts.begin(), excepts.end(), name) == excepts.end())
				candidates.push_back(name);
		}
		if (candidates.empty())
			return defaultChar;
		setRand();
		return candidates[random(candidates.size())];
	}

	inline const char *getSelectedOrRandomHero()
	{
		return selectLayer->_playerSelect ? selectLayer->_playerSelect : getRandomHero();
	}

	inline const char *getSelectOrRandomHeroExcept(const char *except)
	{
		return selectLayer->_playerSelect ? selectLayer->_playerSelect : getRandomHeroExcept(except);
	}

	inline const char *getSelectOrRandomHeroExceptAll(const vector<string> &excepts, const char *defaultChar = "Naruto")
	{
		return selectLayer->_playerSelect ? selectLayer->_playerSelect : getRandomHeroExceptAll(excepts, defaultChar);
	}

	static inline Group getRandomGroup()
	{
		setRand();
		return random(2) == 0 ? Group::Konoha : Group::Akatsuki;
	}

	// Varian hero menurut page skill yang dipilih di SkillLayer.
	// SkillLayer.lua menyimpan "heroPage_<Hero>" = nama varian (mis. heroPage_Guy = "Guy8Gates")
	// saat page 2 dipilih, dan string kosong saat page 1. Kosong / tidak ada => nama asli.
	static inline string resolveHeroVariant(const string &name)
	{
		if (name.empty())
			return name;
		string key = "heroPage_" + name;
		string variant = UserDefault::sharedUserDefault()->getStringForKey(key.c_str());
		return variant.empty() ? name : variant;
	}

	// Ambil (lalu hapus) pilihan musuh ke-index (0..3) dari layar pilih.
	// Kosong berarti musuh diacak.
	static inline string takeCustomEnemy(int index)
	{
		static const char *kKeys[4] = {"customEnemy1", "customEnemy2", "customEnemy3", "customEnemy4"};
		if (index < 0 || index > 3)
			return "";
		auto ud = UserDefault::sharedUserDefault();
		string value = ud->getStringForKey(kKeys[index]);
		if (!value.empty())
			ud->setStringForKey(kKeys[index], "");
		return value;
	}

	static inline void getRandomGroups(Group &groupA, Group &groupB)
	{
		setRand();
		int team = random(2) == 0;
		groupA = team == 0 ? Group::Konoha : Group::Akatsuki;
		groupB = team == 0 ? Group::Akatsuki : Group::Konoha;
	}
};
