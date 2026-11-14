#pragma once
#include "Core/Hero.hpp"
#include "Utils/Cocos2dxHelper.hpp"

// Kelas yang boleh di-spawn dari Lua lewat createCloneHero / createSummonHero
#include "Core/Kuchiyose/Akamaru.hpp"
#include "Core/Kuchiyose/Centipede.hpp"
#include "Core/Kuchiyose/DogWall.hpp"
#include "Core/Kuchiyose/Karasu.hpp"
#include "Core/Kuchiyose/Kurama.hpp"
#include "Core/Kuchiyose/Sanshouuo.hpp"
#include "Core/Kuchiyose/Saso.hpp"
#include "Core/Kuchiyose/Slug.hpp"
#include "Core/Kuchiyose/WhiteZetsu.hpp"
#include "Core/Shinobi/Bunshin/NarutoClone.hpp"
#include "Core/Shinobi/Bunshin/RikudoNarutoClone.hpp"
#include "Core/Shinobi/Bunshin/SageNarutoClone.hpp"

#include <cmath>
#include <cstring>

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "tolua++.h"
#include "tolua_fix.h"
}

/**
 * Jembatan Lua -> C++ untuk karakter (CharacterBase / Hero).
 *
 * Sistem Lua yang dipakai = tolua++ yang sudah ada. Kelas "CharacterBase" sudah didaftarkan di
 * scripting/lua/cocos2dx_support/LuaCocos2d.cpp sebagai kelas KOSONG (turunan CCSprite), jadi
 * semua method CCNode/CCSprite (getPositionX, setPosition, ...) sudah tersedia di Lua dan TIDAK
 * di-bind ulang di sini. Class ini hanya MENAMBAHKAN method yang belum ada, memakai pola yang sama
 * dengan extendCCNode() di lua_cocos2dx_manual.cpp (tambah fungsi ke tabel kelas di registry).
 *
 * Class (bukan namespace) karena di-`friend` oleh CharacterBase: helper AI seperti findHeroHalf(),
 * tryBuyGear(), getDistanceToTarget() berstatus protected.
 *
 * Method pada userdata "CharacterBase" (self):
 *   -- aksi
 *   self:idle()                        self:walk(dx, dy)                 self:stepOn()
 *   self:stepBack2() -> bool           self:changeSide(sx, sy)           self:attack(ABType.X)
 *   self:useGear(GearType.X)           self:tryBuyGear(g1, g2, g3)
 *   -- pencarian target / AI helper
 *   self:findHeroHalf() -> bool        self:findTowerHalf() -> bool      self:notFindFlogHalf() -> bool
 *   self:needBackToTowerToRestoreHP([bool]) -> bool    self:needBackToDefendTower([bool]) -> bool
 *   self:checkHealingState()
 *   self:clearMainTarget()             self:hasMainTarget() -> bool
 *   self:mainTargetIsFlog() / mainTargetIsNotFlog() -> bool              self:getMainTargetDEF() -> int
 *   self:getDistanceToTarget() -> dx, dy
 *   -- state
 *   self:isFreeState() -> bool         self:getState() / setState(State.X)
 *   self:getFlag(name) / setFlag(name, bool)      (whitelist field bool, lihat kBoolFields)
 *   self:getNum(name)  / setNum(name, number)     (whitelist field angka, lihat kNumFields)
 *   self:getGameFlag(name) -> bool                (whitelist field GameLayer, lihat kGameFlags)
 *   self:getNAttackValue() / setNAttackValue(v)
 *   self:getNAttackType() / setNAttackType(str)   self:getSpcAttack3Type() -> string
 *   self:getUID() -> integer                      (id unik objek; kunci state per-instance di Lua)
 *   -- animasi / buff / UI
 *   self:setAnimation(slot, array, fps, loop, returnToIdle)   slot: idle|walk|nattack
 *                                                              array: idle|walk|nattack|skillSPC1|skillSPC2|skillSPC3
 *   self:startHealBuff(interval)
 *   self:lockOugisButtons() / unlockOugisButtons()
 *   self:callBaseResumeAction(dt)      -- CharacterBase::resumeAction (non-virtual, "super")
 *   -- spawn  (return CharacterBase userdata | nil)
 *   self:createCloneHero(className, unitName)     -> Hero::createCloneHero<T>(unitName)
 *   self:createSummonHero(className, unitName)    -> Hero::createSummonHero<T>(unitName)
 *
 * Tabel konstanta global: ABType, GearType, State.
 *
 * Dipanggil sekali dari Internal::initAllSystems(), sebelum main.lua dijalankan.
 */
class HeroBinding
{
public:
	static void registerLua()
	{
		if (!LuaBridge::isReady())
		{
			CCLOG("[HeroBinding] Lua engine not ready, CharacterBase methods not registered");
			return;
		}

		lua_getL;
		registerConstants(L);
		extendCharacterBase(L);
	}

private:
	// ------------------------------------------------------------------ helpers

	// Ambil CharacterBase dari argumen `idx`. Error Lua kalau bukan CharacterBase / sudah dibebaskan.
	// (Tidak ada objek ber-destruktor di scope pemanggil saat luaL_error dilempar.)
	static CharacterBase *self(lua_State *L, int idx = 1)
	{
		tolua_Error err;
		if (lua_isnil(L, idx) || !tolua_isusertype(L, idx, "CharacterBase", 0, &err))
			luaL_error(L, "bad argument #%d: CharacterBase expected", idx);

		CCObject *obj = static_cast<CCObject *>(tolua_tousertype(L, idx, nullptr));
		CharacterBase *c = dynamic_cast<CharacterBase *>(obj);
		if (!c)
			luaL_error(L, "bad argument #%d: CharacterBase already released", idx);
		return c;
	}

	static void pushChar(lua_State *L, CharacterBase *c)
	{
		if (!c)
			lua_pushnil(L);
		else
			get_luastack->pushCCObject(static_cast<CCObject *>(c), "CharacterBase");
	}

	static bool optBool(lua_State *L, int idx, bool def)
	{
		return lua_isnoneornil(L, idx) ? def : lua_toboolean(L, idx) != 0;
	}

	// ------------------------------------------------------------------ aksi

	static int l_idle(lua_State *L)
	{
		self(L)->idle();
		return 0;
	}

	static int l_walk(lua_State *L)
	{
		CharacterBase *c = self(L);
		c->walk(Vec2((float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3)));
		return 0;
	}

	static int l_stepOn(lua_State *L)
	{
		self(L)->stepOn();
		return 0;
	}

	static int l_stepBack2(lua_State *L)
	{
		lua_pushboolean(L, self(L)->stepBack2() ? 1 : 0);
		return 1;
	}

	static int l_changeSide(lua_State *L)
	{
		CharacterBase *c = self(L);
		c->changeSide(Vec2((float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3)));
		return 0;
	}

	static int l_attack(lua_State *L)
	{
		CharacterBase *c = self(L);
		const lua_Integer t = luaL_checkinteger(L, 2);
		if (t < NAttack || t > OUGIS2)
			return luaL_error(L, "attack: ABType tidak valid (%d)", (int)t);
		c->attack(static_cast<ABType>(t)); // virtual: override subclass (mis. Kabuto) tetap dipakai
		return 0;
	}

	static GearType checkGear(lua_State *L, int idx)
	{
		const lua_Integer g = luaL_checkinteger(L, idx);
		if (g < (lua_Integer)GearType::Gear00 || g > (lua_Integer)GearType::None)
			luaL_error(L, "bad argument #%d: GearType tidak valid (%d)", idx, (int)g);
		return static_cast<GearType>(g);
	}

	static int l_useGear(lua_State *L)
	{
		CharacterBase *c = self(L);
		c->useGear(checkGear(L, 2));
		return 0;
	}

	static int l_tryBuyGear(lua_State *L)
	{
		CharacterBase *c = self(L);
		const GearType g1 = checkGear(L, 2);
		const GearType g2 = checkGear(L, 3);
		const GearType g3 = checkGear(L, 4);
		c->tryBuyGear(g1, g2, g3);
		return 0;
	}

	// ------------------------------------------------------------------ target / AI helper

	static int l_findHeroHalf(lua_State *L)
	{
		lua_pushboolean(L, self(L)->findHeroHalf() ? 1 : 0);
		return 1;
	}

	static int l_findTowerHalf(lua_State *L)
	{
		lua_pushboolean(L, self(L)->findTowerHalf() ? 1 : 0);
		return 1;
	}

	static int l_notFindFlogHalf(lua_State *L)
	{
		lua_pushboolean(L, self(L)->notFindFlogHalf() ? 1 : 0);
		return 1;
	}

	static int l_needBackToTowerToRestoreHP(lua_State *L)
	{
		CharacterBase *c = self(L);
		lua_pushboolean(L, c->needBackToTowerToRestoreHP(optBool(L, 2, true)) ? 1 : 0);
		return 1;
	}

	static int l_needBackToDefendTower(lua_State *L)
	{
		CharacterBase *c = self(L);
		lua_pushboolean(L, c->needBackToDefendTower(optBool(L, 2, true)) ? 1 : 0);
		return 1;
	}

	static int l_checkHealingState(lua_State *L)
	{
		self(L)->checkHealingState();
		return 0;
	}

	static int l_clearMainTarget(lua_State *L)
	{
		self(L)->_mainTarget = nullptr;
		return 0;
	}

	static int l_hasMainTarget(lua_State *L)
	{
		lua_pushboolean(L, self(L)->_mainTarget ? 1 : 0);
		return 1;
	}

	static int l_mainTargetIsFlog(lua_State *L)
	{
		CharacterBase *c = self(L);
		lua_pushboolean(L, (c->_mainTarget && c->_mainTarget->isFlog()) ? 1 : 0);
		return 1;
	}

	static int l_mainTargetIsNotFlog(lua_State *L)
	{
		CharacterBase *c = self(L);
		lua_pushboolean(L, (c->_mainTarget && c->_mainTarget->isNotFlog()) ? 1 : 0);
		return 1;
	}

	static int l_getMainTargetDEF(lua_State *L)
	{
		CharacterBase *c = self(L);
		lua_pushinteger(L, c->_mainTarget ? (lua_Integer)c->_mainTarget->getDEF() : 0);
		return 1;
	}

	static int l_getDistanceToTarget(lua_State *L)
	{
		CharacterBase *c = self(L);
		if (!c->_mainTarget)
		{
			lua_pushnumber(L, 0);
			lua_pushnumber(L, 0);
			return 2;
		}
		const Vec2 sp = c->getDistanceToTarget();
		lua_pushnumber(L, sp.x);
		lua_pushnumber(L, sp.y);
		return 2;
	}

	// ------------------------------------------------------------------ state

	static int l_isFreeState(lua_State *L)
	{
		lua_pushboolean(L, self(L)->isFreeState() ? 1 : 0);
		return 1;
	}

	static int l_getState(lua_State *L)
	{
		lua_pushinteger(L, (lua_Integer)self(L)->getState());
		return 1;
	}

	static int l_setState(lua_State *L)
	{
		CharacterBase *c = self(L);
		const lua_Integer s = luaL_checkinteger(L, 2);
		if (s < (lua_Integer)State::DEAD || s > (lua_Integer)State::AIRHURT)
			return luaL_error(L, "setState: State tidak valid (%d)", (int)s);
		c->setState(static_cast<State>(s));
		return 0;
	}

	struct BoolField
	{
		const char *name;
		bool CharacterBase::*ptr;
		bool writable;
	};

	static const BoolField *findBool(const char *name)
	{
		static const BoolField kBoolFields[] = {
			{"_isCanOugis1", &CharacterBase::_isCanOugis1, false},
			{"_isCanOugis2", &CharacterBase::_isCanOugis2, false},
			{"_isCanSkill1", &CharacterBase::_isCanSkill1, false},
			{"_isCanSkill2", &CharacterBase::_isCanSkill2, false},
			{"_isCanSkill3", &CharacterBase::_isCanSkill3, false},
			{"_isCanGear03", &CharacterBase::_isCanGear03, false},
			{"_isControlled", &CharacterBase::_isControlled, false},
			{"_isHealing", &CharacterBase::_isHealing, false},
			{"isBaseDanger", &CharacterBase::isBaseDanger, false},
			{"_isArmored", &CharacterBase::_isArmored, true},
			{"_isOnlySkillLocked", &CharacterBase::_isOnlySkillLocked, true},
		};
		for (const auto &f : kBoolFields)
			if (std::strcmp(f.name, name) == 0)
				return &f;
		return nullptr;
	}

	static int l_getFlag(lua_State *L)
	{
		CharacterBase *c = self(L);
		const char *name = luaL_checkstring(L, 2);
		const BoolField *f = findBool(name);
		if (!f)
			return luaL_error(L, "getFlag: field '%s' tidak ada di whitelist", name);
		lua_pushboolean(L, (c->*(f->ptr)) ? 1 : 0);
		return 1;
	}

	static int l_setFlag(lua_State *L)
	{
		CharacterBase *c = self(L);
		const char *name = luaL_checkstring(L, 2);
		luaL_checkany(L, 3);
		const BoolField *f = findBool(name);
		if (!f)
			return luaL_error(L, "setFlag: field '%s' tidak ada di whitelist", name);
		if (!f->writable)
			return luaL_error(L, "setFlag: field '%s' read-only", name);
		c->*(f->ptr) = lua_toboolean(L, 3) != 0;
		return 0;
	}

	struct NumField
	{
		const char *name;
		int CharacterBase::*iptr;		// salah satu saja yang non-null
		uint32_t CharacterBase::*uptr;
		bool writable;
	};

	static const NumField *findNum(const char *name)
	{
		static const NumField kNumFields[] = {
			{"enemyCombatPoint", &CharacterBase::enemyCombatPoint, nullptr, false},
			{"friendCombatPoint", &CharacterBase::friendCombatPoint, nullptr, false},
			{"_nAttackRangeX", &CharacterBase::_nAttackRangeX, nullptr, true},
			{"_nAttackRangeY", &CharacterBase::_nAttackRangeY, nullptr, true},
			{"_healBuffValue", nullptr, &CharacterBase::_healBuffValue, true},
		};
		for (const auto &f : kNumFields)
			if (std::strcmp(f.name, name) == 0)
				return &f;
		return nullptr;
	}

	static int l_getNum(lua_State *L)
	{
		CharacterBase *c = self(L);
		const char *name = luaL_checkstring(L, 2);
		const NumField *f = findNum(name);
		if (!f)
			return luaL_error(L, "getNum: field '%s' tidak ada di whitelist", name);
		lua_pushnumber(L, f->iptr ? (lua_Number)(c->*(f->iptr)) : (lua_Number)(c->*(f->uptr)));
		return 1;
	}

	static int l_setNum(lua_State *L)
	{
		CharacterBase *c = self(L);
		const char *name = luaL_checkstring(L, 2);
		const lua_Number v = luaL_checknumber(L, 3);
		const NumField *f = findNum(name);
		if (!f)
			return luaL_error(L, "setNum: field '%s' tidak ada di whitelist", name);
		if (!f->writable)
			return luaL_error(L, "setNum: field '%s' read-only", name);
		if (f->iptr)
			c->*(f->iptr) = (int)v;
		else
			c->*(f->uptr) = v < 0 ? 0u : (uint32_t)v;
		return 0;
	}

	static int l_getGameFlag(lua_State *L)
	{
		const char *name = luaL_checkstring(L, 2);
		GameLayer *g = getGameLayer();
		bool value = false;
		if (std::strcmp(name, "_isOugis2Game") == 0)
			value = g && g->_isOugis2Game;
		else if (std::strcmp(name, "_enableGear") == 0)
			value = g && g->_enableGear;
		else
			return luaL_error(L, "getGameFlag: '%s' tidak ada di whitelist", name);
		lua_pushboolean(L, value ? 1 : 0);
		return 1;
	}

	static int l_getNAttackValue(lua_State *L)
	{
		lua_pushnumber(L, (lua_Number)self(L)->getNAttackValue());
		return 1;
	}

	static int l_setNAttackValue(lua_State *L)
	{
		CharacterBase *c = self(L);
		const lua_Number v = luaL_checknumber(L, 2);
		c->setNAttackValue(v < 0 ? 0u : (uint32_t)v);
		return 0;
	}

	static int l_getNAttackType(lua_State *L)
	{
		lua_pushstring(L, self(L)->getNAttackType().c_str());
		return 1;
	}

	static int l_setNAttackType(lua_State *L)
	{
		CharacterBase *c = self(L);
		c->setNAttackType(std::string(luaL_checkstring(L, 2)));
		return 0;
	}

	static int l_getSpcAttack3Type(lua_State *L)
	{
		lua_pushstring(L, self(L)->getSpcAttack3Type().c_str());
		return 1;
	}

	static int l_getUID(lua_State *L)
	{
		lua_pushinteger(L, (lua_Integer) static_cast<CCObject *>(self(L))->m_uID);
		return 1;
	}

	// ------------------------------------------------------------------ animasi / buff / UI

	static CCArray *arrayByName(CharacterBase *c, const char *name)
	{
		if (!std::strcmp(name, "idle"))
			return c->idleArray;
		if (!std::strcmp(name, "walk"))
			return c->walkArray;
		if (!std::strcmp(name, "nattack"))
			return c->nattackArray;
		if (!std::strcmp(name, "skillSPC1"))
			return c->skillSPC1Array;
		if (!std::strcmp(name, "skillSPC2"))
			return c->skillSPC2Array;
		if (!std::strcmp(name, "skillSPC3"))
			return c->skillSPC3Array;
		return nullptr;
	}

	static int l_setAnimation(lua_State *L)
	{
		CharacterBase *c = self(L);
		const char *slot = luaL_checkstring(L, 2);
		const char *array = luaL_checkstring(L, 3);
		const lua_Integer fps = luaL_checkinteger(L, 4);
		const bool loop = optBool(L, 5, false);
		const bool toIdle = optBool(L, 6, false);

		CCArray *frames = arrayByName(c, array);
		if (!frames)
			return luaL_error(L, "setAnimation: array '%s' tidak dikenal / belum dimuat", array);
		if (fps < 1 || fps > 255)
			return luaL_error(L, "setAnimation: fps harus 1..255");

		auto action = c->createAnimation(frames, (uint8_t)fps, loop, toIdle);
		if (!std::strcmp(slot, "idle"))
			c->setIdleAction(action);
		else if (!std::strcmp(slot, "walk"))
			c->setWalkAction(action);
		else if (!std::strcmp(slot, "nattack"))
			c->setNAttackAction(action);
		else
			return luaL_error(L, "setAnimation: slot '%s' tidak dikenal", slot);
		return 0;
	}

	static int l_startHealBuff(lua_State *L)
	{
		CharacterBase *c = self(L);
		c->schedule(schedule_selector(CharacterBase::healBuff), (float)luaL_optnumber(L, 2, 1));
		return 0;
	}

	static int l_lockOugisButtons(lua_State *L)
	{
		self(L)->lockOugisButtons();
		return 0;
	}

	static int l_unlockOugisButtons(lua_State *L)
	{
		self(L)->unlockOugisButtons();
		return 0;
	}

	static int l_callBaseResumeAction(lua_State *L)
	{
		CharacterBase *c = self(L);
		c->CharacterBase::resumeAction((float)luaL_optnumber(L, 2, 0));
		return 0;
	}

	// ------------------------------------------------------------------ spawn

	using SpawnFn = Hero *(*)(Hero *, const std::string &);

	template <typename T>
	static Hero *spawnClone(Hero *owner, const std::string &name) { return owner->createCloneHero<T>(name); }

	template <typename T>
	static Hero *spawnSummon(Hero *owner, const std::string &name) { return owner->createSummonHero<T>(name); }

	struct SpawnEntry
	{
		const char *className;
		SpawnFn clone;
		SpawnFn summon;
	};

#define SPAWN_ENTRY(T) {#T, &spawnClone<T>, &spawnSummon<T>}
	static const SpawnEntry *findSpawn(const char *className)
	{
		static const SpawnEntry kSpawnTable[] = {
			SPAWN_ENTRY(Akamaru),
			SPAWN_ENTRY(Centipede),
			SPAWN_ENTRY(DogWall),
			SPAWN_ENTRY(Karasu),
			SPAWN_ENTRY(Kurama),
			SPAWN_ENTRY(Sanshouuo),
			SPAWN_ENTRY(Saso),
			SPAWN_ENTRY(Slug),
			SPAWN_ENTRY(WhiteZetsu),
			SPAWN_ENTRY(NarutoClone),
			SPAWN_ENTRY(SageNarutoClone),
			SPAWN_ENTRY(RikudoNarutoClone),
		};
		for (const auto &e : kSpawnTable)
			if (std::strcmp(e.className, className) == 0)
				return &e;
		return nullptr;
	}
#undef SPAWN_ENTRY

	static int spawnCommon(lua_State *L, bool asClone)
	{
		CharacterBase *c = self(L);
		const char *className = luaL_checkstring(L, 2);
		const char *unitName = luaL_checkstring(L, 3);

		Hero *owner = dynamic_cast<Hero *>(c);
		if (!owner)
			return luaL_error(L, "createCloneHero/createSummonHero: self bukan Hero");
		const SpawnEntry *entry = findSpawn(className);
		if (!entry)
			return luaL_error(L, "spawn: class '%s' tidak ada di whitelist (HeroBinding::findSpawn)", className);

		Hero *created = (asClone ? entry->clone : entry->summon)(owner, std::string(unitName));
		pushChar(L, created);
		return 1;
	}

	static int l_createCloneHero(lua_State *L) { return spawnCommon(L, true); }
	static int l_createSummonHero(lua_State *L) { return spawnCommon(L, false); }

	// ------------------------------------------------------------------ registrasi

	static void setConst(lua_State *L, const char *key, lua_Integer value)
	{
		lua_pushinteger(L, value);
		lua_setfield(L, -2, key);
	}

	// Konstanta global. Tidak menimpa tabel yang sudah ada (mencegah binding ganda).
	static void registerConstants(lua_State *L)
	{
		static const char *kTables[] = {"ABType", "GearType", "State"};
		for (auto t : kTables)
		{
			lua_getglobal(L, t);
			const bool exists = !lua_isnil(L, -1);
			lua_pop(L, 1);
			if (exists)
			{
				CCLOG("[HeroBinding] global '%s' sudah ada, dilewati", t);
				return;
			}
		}

		lua_newtable(L);
		setConst(L, "NAttack", NAttack);
		setConst(L, "Item1", Item1);
		setConst(L, "GearItem", GearItem);
		setConst(L, "GearBtn", GearBtn);
		setConst(L, "SKILL1", SKILL1);
		setConst(L, "SKILL2", SKILL2);
		setConst(L, "SKILL3", SKILL3);
		setConst(L, "OUGIS1", OUGIS1);
		setConst(L, "OUGIS2", OUGIS2);
		lua_setglobal(L, "ABType");

		lua_newtable(L);
		setConst(L, "Gear00", (lua_Integer)GearType::Gear00);
		setConst(L, "Gear01", (lua_Integer)GearType::Gear01);
		setConst(L, "Gear02", (lua_Integer)GearType::Gear02);
		setConst(L, "Gear03", (lua_Integer)GearType::Gear03);
		setConst(L, "Gear04", (lua_Integer)GearType::Gear04);
		setConst(L, "Gear05", (lua_Integer)GearType::Gear05);
		setConst(L, "Gear06", (lua_Integer)GearType::Gear06);
		setConst(L, "Gear07", (lua_Integer)GearType::Gear07);
		setConst(L, "Gear08", (lua_Integer)GearType::Gear08);
		setConst(L, "None", (lua_Integer)GearType::None);
		lua_setglobal(L, "GearType");

		lua_newtable(L);
		setConst(L, "DEAD", (lua_Integer)State::DEAD);
		setConst(L, "IDLE", (lua_Integer)State::IDLE);
		setConst(L, "WALK", (lua_Integer)State::WALK);
		setConst(L, "NATTACK", (lua_Integer)State::NATTACK);
		setConst(L, "SATTACK", (lua_Integer)State::SATTACK);
		setConst(L, "OATTACK", (lua_Integer)State::OATTACK);
		setConst(L, "O2ATTACK", (lua_Integer)State::O2ATTACK);
		setConst(L, "HURT", (lua_Integer)State::HURT);
		setConst(L, "KNOCKDOWN", (lua_Integer)State::KNOCKDOWN);
		setConst(L, "FLOAT", (lua_Integer)State::FLOAT);
		setConst(L, "JUMP", (lua_Integer)State::JUMP);
		setConst(L, "AIRHURT", (lua_Integer)State::AIRHURT);
		lua_setglobal(L, "State");
	}

	static void extendCharacterBase(lua_State *L)
	{
		static const struct
		{
			const char *name;
			lua_CFunction func;
		} kMethods[] = {
			{"idle", l_idle},
			{"walk", l_walk},
			{"stepOn", l_stepOn},
			{"stepBack2", l_stepBack2},
			{"changeSide", l_changeSide},
			{"attack", l_attack},
			{"useGear", l_useGear},
			{"tryBuyGear", l_tryBuyGear},

			{"findHeroHalf", l_findHeroHalf},
			{"findTowerHalf", l_findTowerHalf},
			{"notFindFlogHalf", l_notFindFlogHalf},
			{"needBackToTowerToRestoreHP", l_needBackToTowerToRestoreHP},
			{"needBackToDefendTower", l_needBackToDefendTower},
			{"checkHealingState", l_checkHealingState},
			{"clearMainTarget", l_clearMainTarget},
			{"hasMainTarget", l_hasMainTarget},
			{"mainTargetIsFlog", l_mainTargetIsFlog},
			{"mainTargetIsNotFlog", l_mainTargetIsNotFlog},
			{"getMainTargetDEF", l_getMainTargetDEF},
			{"getDistanceToTarget", l_getDistanceToTarget},

			{"isFreeState", l_isFreeState},
			{"getState", l_getState},
			{"setState", l_setState},
			{"getFlag", l_getFlag},
			{"setFlag", l_setFlag},
			{"getNum", l_getNum},
			{"setNum", l_setNum},
			{"getGameFlag", l_getGameFlag},
			{"getNAttackValue", l_getNAttackValue},
			{"setNAttackValue", l_setNAttackValue},
			{"getNAttackType", l_getNAttackType},
			{"setNAttackType", l_setNAttackType},
			{"getSpcAttack3Type", l_getSpcAttack3Type},
			{"getUID", l_getUID},

			{"setAnimation", l_setAnimation},
			{"startHealBuff", l_startHealBuff},
			{"lockOugisButtons", l_lockOugisButtons},
			{"unlockOugisButtons", l_unlockOugisButtons},
			{"callBaseResumeAction", l_callBaseResumeAction},

			{"createCloneHero", l_createCloneHero},
			{"createSummonHero", l_createSummonHero},
		};

		lua_pushstring(L, "CharacterBase");
		lua_rawget(L, LUA_REGISTRYINDEX);
		if (!lua_istable(L, -1))
		{
			lua_pop(L, 1);
			CCLOG("[HeroBinding] tolua class 'CharacterBase' tidak terdaftar, method tidak ditambahkan");
			return;
		}

		int added = 0;
		for (const auto &m : kMethods)
		{
			// Jangan buat binding ganda: lewati bila nama sudah ada di kelas (mis. dari tolua generator).
			// rawget: jangan memicu __index milik tolua (tabel kelas = metatable dirinya sendiri).
			lua_pushstring(L, m.name);
			lua_rawget(L, -2);
			const bool exists = !lua_isnil(L, -1);
			lua_pop(L, 1);
			if (exists)
			{
				CCLOG("[HeroBinding] CharacterBase.%s sudah ada, dilewati", m.name);
				continue;
			}
			tolua_function(L, m.name, m.func);
			added++;
		}
		lua_pop(L, 1);
		CCLOG("[HeroBinding] %d method ditambahkan ke CharacterBase", added);
	}
};
