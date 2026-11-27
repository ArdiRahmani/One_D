#pragma once
#include "Hero.hpp"
#include "Systems/LuaAISystem.hpp"

/**
 * LuaHero: Hero yang AI-nya ditulis di Lua (lua/AI/<Script>.lua).
 *
 * Dibuat oleh Provider::create() untuk unit yang di-bind lewat `LuaAI.set('Unit', 'Script')`
 * (lihat lua/Hook/LuaAI.lua). Setara dengan class Hero C++ (mis. Tsunade.hpp), tapi
 * perform() / changeAction() / resumeAction() / createClone() dijalankan dari Lua:
 *
 *   -- lua/AI/Tsunade.lua
 *   return {
 *       perform      = function(self) ... end,          -- WAJIB, tiap tick AI
 *       changeAction = function(self) ... end,          -- opsional
 *       resumeAction = function(self, dt) ... end,      -- opsional
 *       setActionResume = function(self) ... end,       -- opsional (setara override setActionResume, mis. Hashirama)
 *       createClone  = function(self, cloneTime) ... end, -- opsional, return self:createSummon('Slug')
 *       onSetTrap    = function(self, trapType) ... end,  -- opsional (setara override onSetTrap, mis. Karin)
 *   }
 *
 * `self` dan hasil self:target() adalah proxy "unit". Aturan umum:
 *   - Method dipanggil dengan titik dua:   self:attack('SKILL1'),  target:distanceTo(self)
 *   - Properti dibaca/ditulis dengan titik: self.hp,  self.isArmored = true
 *   - Field lain yang kamu tulis ke `self` (mis. self.origin = 1) disimpan per-instance di Lua,
 *     jadi setara anggota private C++ (contoh: `string _originNAttackType` di Tsunade.hpp).
 *   - Proxy `self` otomatis tidak valid begitu hero dibuang C++ (memanggil method di dalamnya
 *     melempar error Lua, bukan crash). Proxy target menahan retain, jadi aman disimpan.
 *
 * ---- Method self (hero sendiri) -------------------------------------------------------
 *  Cari target (isi self._mainTarget):
 *    findHero(range[, masterRange]) findFlog(...) findTower(...)           -> boolean
 *    notFindHero(...) notFindFlog(...) notFindTower(...)                   -> boolean
 *    findHeroHalf() findFlogHalf() findTowerHalf()                         -> boolean
 *    notFindHeroHalf() notFindFlogHalf() notFindTowerHalf()                -> boolean
 *    target() -> unit|nil      hasTarget() -> boolean      clearTarget()      setTarget(unit|nil)
 *    distance() -> dx, dy|nil  (getDistanceToTarget)       distanceIgnoreOriginY() -> dx, dy|nil
 *  Gerak & serang:
 *    walk(dx, dy)    (arah otomatis dinormalisasi, setara walk(sp.getNormalized()))
 *    idle()  stepBack() -> boolean  stepBack2() -> boolean  stepOn()  changeSide(dx, dy)
 *    attack(tipe)    tipe: 'NAttack' 'SKILL1' 'SKILL2' 'SKILL3' 'OUGIS1' 'OUGIS2'
 *                          (juga 'Item1' 'GearItem' 'GearBtn'; huruf besar/kecil bebas)
 *  Gear & pola AI siap pakai:
 *    useGear('Gear03')   tryBuyGear('Gear03','Gear02','Gear04')   tryUseGear6()
 *    needBackToTowerToRestoreHP([isNeedBack=true]) -> boolean
 *    needBackToDefendTower([isNeedBack=true])      -> boolean
 *    checkHealingState()
 *  Animasi / state (untuk changeAction & resumeAction):
 *    setAction(slot[, array[, fps[, loop[, returnToIdle]]]]) -> boolean
 *        slot  : idle walk hurt airhurt knockdown float dead nattack skill1..skill5
 *        array : idle walk hurt airhurt knockdown float dead nattack skill1..skill5 spc1..spc5
 *                (kosong = array bawaan slot itu, jadi setAction('idle') = kembalikan ke semula)
 *    lockOugisButtons()  unlockOugisButtons()
 *    startHealBuff(value[, interval=1])  stopHealBuff()
 *    baseChangeAction()  baseResumeAction(dt)    (panggil versi C++ CharacterBase)
 *    createSummon('Slug') createClone('Nama') createKugutsu('Nama') -> unit|nil
 *        (Provider::create dengan Role::Summon / Clone / Kugutsu; AI unit itu ikut LuaAI/CloneAI)
 *    setItem('Item1')  cancelResumeAction()  releaseControlledHeroes([idleAfter])  (Ino)
 *    createTrap('Kusuri', offsetX, offsetY)   (Karin)
 *    knockDown()   unit:resetDefenseValue(v)   setupSpcSkill1Clone(clone)
 *    createClone('Nama'[, native=true])  native: abaikan binding LuaAI, pakai class C++ (mis. NarutoClone)
 *    revealEnemies([range=kAttackRange])   (musuh invisible dalam jarak jadi terlihat; dipakai Kakashi)
 *    setSkillButtonFrame(1|2, 'frame.png')  (ikon tombol HUD, hanya jika self Player)
 *    unit:setPosition(x, y)  unit:setAnchorPoint(x, y)   (unit mana pun, mis. hasil createSummon)
 *    addMonster(unit)  lockSkill4Button() unlockSkill4Button() lockSkill5Button() unlockSkill5Button()
 *    data() -> tabel per-instance
 *
 * ---- Method semua unit -------------------------------------------------------------------
 *    isValid() -> boolean       distanceTo(unit) -> dx, dy
 *
 * ---- Properti (baca) semua unit ----------------------------------------------------------
 *    name role group state hp maxHp hpPercent def level exp ckr ckr2 coin x y height walkSpeed
 *    nAttackValue nAttackType isFlog isNotFlog isTower isNotTower isCom isPlayer isPlayerOrCom
 *    isClone isKugutsu isSummon isMon isBullet isGuardian isArmored isInvincible isControlled
 *    isHealing canGear06 originSpeed(r/w) tempAttackValue1(r/w)
 *    battleCondiction canItem1 buffStartTime (self, baca)
 *    def(r/w) hasArmorBroken(r/w) spcAttackRangeX3/Y3(self)
 *    isSticking isFlipped skillUPBuffValue spcAttackValue1 sAttackValue1(r/w) isVisable(r/w)
 *    role  : 'Com' 'Player' 'Hero' 'Flog' 'Tower' 'Bullet' 'Clone' 'Kugutsu' 'Mon' 'Summon'
 *    state : 'IDLE' 'WALK' 'NATTACK' 'SATTACK' 'OATTACK' 'O2ATTACK' 'HURT' 'KNOCKDOWN' ...
 * ---- Properti khusus self (baca) ---------------------------------------------------------
 *    canSkill1 canSkill2 canSkill3 canOugis1 canOugis2 canGear00 canGear03
 *    isBaseDanger enemyCombatPoint friendCombatPoint isOugis2Game isFreeState gearCount
 *    isOnlySkillLocked healBuffValue skillChangeBuffValue nAttackRangeX nAttackRangeY
 *    spcAttackType1 spcAttackType2 spcAttackType3
 * ---- Properti yang bisa DITULIS ----------------------------------------------------------
 *    semua unit : isArmored nAttackValue nAttackType walkSpeed
 *    khusus self: nAttackRangeX nAttackRangeY isOnlySkillLocked healBuffValue state
 *
 * Catatan error: error Lua di dalam hook dicatat ke log ("[LuaAI] ...") lengkap dengan traceback
 * (maks. 5 kali per hero, supaya log tidak banjir), lalu hero lanjut ke tick berikutnya.
 */

struct LuaHeroBinding;

/** Isi userdata proxy unit di Lua */
struct LuaUnit
{
	CharacterBase *ptr; // nullptr = sudah tidak valid
	bool retained;		// true = proxy memegang retain (target / hasil createSummon)
	bool isSelf;		// true = proxy hero sendiri (boleh method aksi)
};

class LuaHero : public Hero
{
	friend struct LuaHeroBinding;

public:
	/** Buat LuaHero dari script. nullptr kalau script gagal dimuat (Provider lalu jatuh balik ke AI C++). */
	static LuaHero *createFromScript(const string &script);

	/** Diisi Provider::create() supaya Lua bisa membuat summon/clone lewat nama. */
	using HeroFactoryFn = Hero *(*)(const string &, Role, Group);
	static inline HeroFactoryFn heroFactory = nullptr;

	/** true = Provider::create() mengabaikan binding LuaAI (dipakai createClone(name, true)) */
	static inline bool bypassLuaAI = false;

	~LuaHero();

	void perform() override;
	void changeAction() override;
	void resumeAction(float dt) override;
	void setActionResume() override;
	void onSetTrap(const string &trapType) override;
	Hero *createClone(int cloneTime) override;

private:
	// Dipanggil lewat hook. Return true kalau hook ada dan sukses; hasil nresults ada di puncak stack.
	// Pemanggil WAJIB menyimpan lua_gettop(L) sebelumnya dan mengembalikannya dengan lua_settop.
	template <typename PushExtra>
	bool callHook(lua_State *L, const char *hook, int nresults, int nextra, PushExtra pushExtra);
	bool hasHook(lua_State *L, const char *hook);
	void pushSelf(lua_State *L);

	string _script;
	int _scriptRef = LUA_NOREF; // tabel script
	int _dataRef = LUA_NOREF;	// tabel field per-instance (self.xxx = ...)
	int _proxyRef = LUA_NOREF;	// userdata proxy self (cache)
	int _errorCount = 0;
};

// =====================================================================================
// Binding Lua <-> C++ (metatable proxy unit)
// =====================================================================================

struct LuaHeroBinding
{
	static constexpr const char *kMT = "NarutoSenki.LuaUnit";
	static constexpr int kMaxErrorLogs = 5;

	// ---------- util ----------

	static bool ieq(const char *a, const char *b)
	{
		for (; *a && *b; ++a, ++b)
			if (std::tolower((unsigned char)*a) != std::tolower((unsigned char)*b))
				return false;
		return *a == *b;
	}

	static const char *roleName(Role r)
	{
		switch (r)
		{
		case Role::Com: return "Com";
		case Role::Player: return "Player";
		case Role::Hero: return "Hero";
		case Role::Flog: return "Flog";
		case Role::Tower: return "Tower";
		case Role::Bullet: return "Bullet";
		case Role::Clone: return "Clone";
		case Role::Kugutsu: return "Kugutsu";
		case Role::Mon: return "Mon";
		case Role::Summon: return "Summon";
		}
		return "?";
	}

	static const char *groupName(Group g)
	{
		return g == Group::Konoha ? "Konoha" : "Akatsuki";
	}

	static const char *stateName(State s)
	{
		switch (s)
		{
		case State::DEAD: return "DEAD";
		case State::IDLE: return "IDLE";
		case State::WALK: return "WALK";
		case State::NATTACK: return "NATTACK";
		case State::SATTACK: return "SATTACK";
		case State::OATTACK: return "OATTACK";
		case State::O2ATTACK: return "O2ATTACK";
		case State::HURT: return "HURT";
		case State::KNOCKDOWN: return "KNOCKDOWN";
		case State::FLOAT: return "FLOAT";
		case State::JUMP: return "JUMP";
		case State::AIRHURT: return "AIRHURT";
		}
		return "?";
	}

	static bool parseState(const char *s, State &out)
	{
		static const struct
		{
			const char *name;
			State v;
		} kTable[] = {
			{"DEAD", State::DEAD}, {"IDLE", State::IDLE}, {"WALK", State::WALK}, {"NATTACK", State::NATTACK},
			{"SATTACK", State::SATTACK}, {"OATTACK", State::OATTACK}, {"O2ATTACK", State::O2ATTACK},
			{"HURT", State::HURT}, {"KNOCKDOWN", State::KNOCKDOWN}, {"FLOAT", State::FLOAT},
			{"JUMP", State::JUMP}, {"AIRHURT", State::AIRHURT}};
		for (const auto &e : kTable)
			if (ieq(s, e.name))
			{
				out = e.v;
				return true;
			}
		return false;
	}

	static bool parseAttack(lua_State *L, int idx, ABType &out)
	{
		if (lua_type(L, idx) == LUA_TNUMBER)
		{
			const int n = (int)lua_tointeger(L, idx);
			if (n < (int)NAttack || n > (int)OUGIS2)
				return false;
			out = (ABType)n;
			return true;
		}
		const char *s = luaL_checkstring(L, idx);
		static const struct
		{
			const char *name;
			ABType v;
		} kTable[] = {
			{"NAttack", NAttack}, {"Item1", Item1}, {"GearItem", GearItem}, {"GearBtn", GearBtn},
			{"SKILL1", SKILL1}, {"SKILL2", SKILL2}, {"SKILL3", SKILL3}, {"OUGIS1", OUGIS1}, {"OUGIS2", OUGIS2}};
		for (const auto &e : kTable)
			if (ieq(s, e.name))
			{
				out = e.v;
				return true;
			}
		return false;
	}

	static bool parseGear(lua_State *L, int idx, GearType &out)
	{
		if (lua_type(L, idx) == LUA_TNUMBER)
		{
			const int n = (int)lua_tointeger(L, idx);
			if (n < 0 || n > (int)GearType::None)
				return false;
			out = (GearType)n;
			return true;
		}
		const char *s = luaL_checkstring(L, idx);
		static const char *kNames[] = {"Gear00", "Gear01", "Gear02", "Gear03", "Gear04",
									   "Gear05", "Gear06", "Gear07", "Gear08", "None"};
		for (int i = 0; i < 10; ++i)
			if (ieq(s, kNames[i]))
			{
				out = (GearType)i;
				return true;
			}
		return false;
	}

	// ---------- userdata ----------

	static void pushUnit(lua_State *L, CharacterBase *c, bool retain, bool isSelf)
	{
		if (!c)
		{
			lua_pushnil(L);
			return;
		}
		auto *u = (LuaUnit *)lua_newuserdata(L, sizeof(LuaUnit));
		u->ptr = c;
		u->retained = retain;
		u->isSelf = isSelf;
		if (retain)
			c->retain();
		luaL_getmetatable(L, kMT);
		lua_setmetatable(L, -2);
	}

	static LuaUnit *toUnit(lua_State *L, int idx)
	{
		return (LuaUnit *)luaL_checkudata(L, idx, kMT);
	}

	// Versi tanpa error Lua (aman dipanggil dari kode C++ game): nullptr kalau bukan proxy unit.
	static LuaUnit *testUnit(lua_State *L, int idx)
	{
		if (!lua_isuserdata(L, idx) || !lua_getmetatable(L, idx))
			return nullptr;
		luaL_getmetatable(L, kMT);
		const bool same = lua_rawequal(L, -1, -2) != 0;
		lua_pop(L, 2);
		return same ? (LuaUnit *)lua_touserdata(L, idx) : nullptr;
	}

	// unit di argumen idx, error Lua kalau sudah tidak valid
	static CharacterBase *aliveUnit(lua_State *L, int idx)
	{
		LuaUnit *u = toUnit(L, idx);
		if (!u->ptr)
			luaL_error(L, "unit sudah tidak valid (hero sudah dibuang)");
		return u->ptr;
	}

	// argumen 1 harus self (hero sendiri) dan masih valid
	static LuaHero *selfOrError(lua_State *L)
	{
		LuaUnit *u = toUnit(L, 1);
		if (!u->isSelf)
			luaL_error(L, "method ini hanya untuk hero sendiri (self), bukan unit lain");
		if (!u->ptr)
			luaL_error(L, "hero sudah tidak valid (sudah dibuang)");
		return static_cast<LuaHero *>(u->ptr);
	}

	static int mtGC(lua_State *L)
	{
		LuaUnit *u = (LuaUnit *)lua_touserdata(L, 1);
		if (u && u->retained && u->ptr)
			u->ptr->release();
		if (u)
		{
			u->ptr = nullptr;
			u->retained = false;
		}
		return 0;
	}

	static int mtEq(lua_State *L)
	{
		LuaUnit *a = (LuaUnit *)lua_touserdata(L, 1);
		LuaUnit *b = (LuaUnit *)lua_touserdata(L, 2);
		lua_pushboolean(L, (a && b && a->ptr == b->ptr) ? 1 : 0);
		return 1;
	}

	static int mtToString(lua_State *L)
	{
		LuaUnit *u = toUnit(L, 1);
		if (!u->ptr)
			lua_pushstring(L, "Unit(<invalid>)");
		else
			lua_pushfstring(L, "Unit(%s, %s)", u->ptr->getName().c_str(), roleName(u->ptr->getRole()));
		return 1;
	}

	// ---------- properti ----------

	using Getter = int (*)(lua_State *, CharacterBase *, LuaHero *);
	using Setter = void (*)(lua_State *, int, CharacterBase *, LuaHero *);
	struct Prop
	{
		Getter get;
		Setter set;
		bool selfOnly;
	};

#define LH_GET(expr_push) [](lua_State *L, CharacterBase *c, LuaHero *h) -> int { (void)c; (void)h; expr_push; return 1; }
#define LH_SET(body) [](lua_State *L, int v, CharacterBase *c, LuaHero *h) { (void)c; (void)h; (void)v; (void)L; body; }

#define LH_NUM(name, expr) {name, {LH_GET(lua_pushnumber(L, (lua_Number)(expr))), nullptr, false}}
#define LH_BOOL(name, expr) {name, {LH_GET(lua_pushboolean(L, (expr) ? 1 : 0)), nullptr, false}}
#define LH_STR(name, expr) {name, {LH_GET(lua_pushstring(L, (expr))), nullptr, false}}
#define LH_SELF_NUM(name, expr) {name, {LH_GET(lua_pushnumber(L, (lua_Number)(expr))), nullptr, true}}
#define LH_SELF_BOOL(name, expr) {name, {LH_GET(lua_pushboolean(L, (expr) ? 1 : 0)), nullptr, true}}
#define LH_SELF_STR(name, expr) {name, {LH_GET(lua_pushstring(L, (expr))), nullptr, true}}

	static const std::unordered_map<std::string, Prop> &props()
	{
		static const std::unordered_map<std::string, Prop> kProps = {
			// ---- semua unit (baca) ----
			LH_STR("name", c->getName().c_str()),
			LH_STR("role", roleName(c->getRole())),
			LH_STR("group", groupName(c->getGroup())),
			LH_STR("state", stateName(c->getState())),
			LH_NUM("hp", c->getHP()),
			LH_NUM("maxHp", c->getMaxHP()),
			LH_NUM("hpPercent", c->getHpPercent()),
			LH_NUM("level", c->getLV()),
			LH_NUM("exp", c->getEXP()),
			LH_NUM("ckr", c->getCKR()),
			LH_NUM("ckr2", c->getCKR2()),
			LH_NUM("coin", c->getCoin()),
			LH_NUM("x", c->getPositionX()),
			LH_NUM("y", c->getPositionY()),
			LH_NUM("height", c->getHeight()),
			LH_BOOL("isFlog", c->isFlog()),
			LH_BOOL("isNotFlog", c->isNotFlog()),
			LH_BOOL("isTower", c->isTower()),
			LH_BOOL("isNotTower", c->isNotTower()),
			LH_BOOL("isCom", c->isCom()),
			LH_BOOL("isPlayer", c->isPlayer()),
			LH_BOOL("isPlayerOrCom", c->isPlayerOrCom()),
			LH_BOOL("isClone", c->isClone()),
			LH_BOOL("isKugutsu", c->isKugutsu()),
			LH_BOOL("isSummon", c->isSummon()),
			LH_BOOL("isMon", c->isMon()),
			LH_BOOL("isBullet", c->isBullet()),
			LH_BOOL("isGuardian", c->isGuardian()),
			LH_BOOL("isInvincible", c->_isInvincible),
			LH_BOOL("isControlled", c->_isControlled),
			LH_BOOL("isHealing", c->_isHealing),
			LH_BOOL("canGear06", c->_isCanGear06),
			LH_BOOL("isSticking", c->_isSticking),
			LH_BOOL("isFlipped", c->_isFlipped),
			LH_NUM("skillUPBuffValue", c->_skillUPBuffValue),
			LH_NUM("spcAttackValue1", c->getSpcAttackValue1()),

			// ---- semua unit (baca + tulis) ----
			{"isArmored", {LH_GET(lua_pushboolean(L, c->_isArmored ? 1 : 0)), LH_SET(c->_isArmored = lua_toboolean(L, v) != 0), false}},
			{"nAttackValue", {LH_GET(lua_pushnumber(L, (lua_Number)c->getNAttackValue())), LH_SET(c->setNAttackValue((uint32_t)luaL_checknumber(L, v))), false}},
			{"nAttackType", {LH_GET(lua_pushstring(L, c->getNAttackType().c_str())), LH_SET(c->setNAttackType(string(luaL_checkstring(L, v)))), false}},
			{"walkSpeed", {LH_GET(lua_pushnumber(L, (lua_Number)c->getWalkSpeed())), LH_SET(c->setWalkSpeed((int)luaL_checkinteger(L, v))), false}},
			{"originSpeed", {LH_GET(lua_pushnumber(L, (lua_Number)c->_originSpeed)), LH_SET(c->_originSpeed = (int)luaL_checkinteger(L, v)), false}},
			{"def", {LH_GET(lua_pushnumber(L, (lua_Number)c->getDEF())), LH_SET(c->setDEF((int)luaL_checkinteger(L, v))), false}},
			{"hasArmorBroken", {LH_GET(lua_pushboolean(L, c->hasArmorBroken ? 1 : 0)), LH_SET(c->hasArmorBroken = lua_toboolean(L, v) != 0), false}},
			{"sAttackValue1", {LH_GET(lua_pushnumber(L, (lua_Number)c->getSAttackValue1())), LH_SET(c->setSAttackValue1((uint32_t)luaL_checknumber(L, v))), false}},
			{"isVisable", {LH_GET(lua_pushboolean(L, c->_isVisable ? 1 : 0)), LH_SET(c->_isVisable = lua_toboolean(L, v) != 0), false}},
			{"tempAttackValue1", {LH_GET(lua_pushnumber(L, (lua_Number)c->getTempAttackValue1())), LH_SET(c->setTempAttackValue1((uint32_t)luaL_checknumber(L, v))), false}},

			// ---- khusus self (baca) ----
			LH_SELF_BOOL("canSkill1", h->_isCanSkill1),
			LH_SELF_BOOL("canSkill2", h->_isCanSkill2),
			LH_SELF_BOOL("canSkill3", h->_isCanSkill3),
			LH_SELF_BOOL("canOugis1", h->_isCanOugis1),
			LH_SELF_BOOL("canOugis2", h->_isCanOugis2),
			LH_SELF_BOOL("canGear00", h->_isCanGear00),
			LH_SELF_BOOL("canGear03", h->_isCanGear03),
			LH_SELF_BOOL("isBaseDanger", h->isBaseDanger),
			LH_SELF_NUM("enemyCombatPoint", h->enemyCombatPoint),
			LH_SELF_NUM("friendCombatPoint", h->friendCombatPoint),
			LH_SELF_BOOL("isOugis2Game", getGameLayer()->_isOugis2Game),
			LH_SELF_BOOL("isFreeState", h->isFreeState()),
			LH_SELF_NUM("gearCount", h->getGearArray().size()),
			LH_SELF_NUM("battleCondiction", h->battleCondiction),
			LH_SELF_BOOL("canItem1", h->_isCanItem1),
			LH_SELF_NUM("buffStartTime", h->_buffStartTime),
			LH_SELF_NUM("spcAttackRangeX3", h->_spcAttackRangeX3),
			LH_SELF_NUM("spcAttackRangeY3", h->_spcAttackRangeY3),
			LH_SELF_STR("spcAttackType1", h->_spcAttackType1.c_str()),
			LH_SELF_STR("spcAttackType2", h->_spcAttackType2.c_str()),
			LH_SELF_STR("spcAttackType3", h->_spcAttackType3.c_str()),

			// ---- khusus self (baca + tulis) ----
			{"isOnlySkillLocked", {LH_GET(lua_pushboolean(L, h->_isOnlySkillLocked ? 1 : 0)), LH_SET(h->_isOnlySkillLocked = lua_toboolean(L, v) != 0), true}},
			{"skillChangeBuffValue", {LH_GET(lua_pushnumber(L, (lua_Number)h->_skillChangeBuffValue)), LH_SET(h->_skillChangeBuffValue = (uint32_t)luaL_checknumber(L, v)), true}},
			{"healBuffValue", {LH_GET(lua_pushnumber(L, (lua_Number)h->_healBuffValue)), LH_SET(h->_healBuffValue = (uint32_t)luaL_checknumber(L, v)), true}},
			{"nAttackRangeX", {LH_GET(lua_pushnumber(L, h->_nAttackRangeX)), LH_SET(h->_nAttackRangeX = (int)luaL_checkinteger(L, v)), true}},
			{"nAttackRangeY", {LH_GET(lua_pushnumber(L, h->_nAttackRangeY)), LH_SET(h->_nAttackRangeY = (int)luaL_checkinteger(L, v)), true}},
		};
		return kProps;
	}

	// state juga bisa ditulis (self): dipisah karena butuh parse + error
	static void setState(lua_State *L, LuaHero *h, int idx)
	{
		State s;
		const char *name = luaL_checkstring(L, idx);
		if (!parseState(name, s))
			luaL_error(L, "state tidak dikenal: %s", name);
		h->setState(s);
	}

#undef LH_GET
#undef LH_SET
#undef LH_NUM
#undef LH_BOOL
#undef LH_STR
#undef LH_SELF_NUM
#undef LH_SELF_BOOL
#undef LH_SELF_STR

	static int mtIndex(lua_State *L)
	{
		LuaUnit *u = toUnit(L, 1);
		if (lua_type(L, 2) != LUA_TSTRING)
		{
			lua_pushnil(L);
			return 1;
		}
		const char *key = lua_tostring(L, 2);

		// 1) method
		lua_getmetatable(L, 1);
		lua_getfield(L, -1, "__methods");
		lua_getfield(L, -1, key);
		if (!lua_isnil(L, -1))
			return 1;
		lua_pop(L, 3);

		// 2) properti
		const auto &p = props();
		auto it = p.find(key);
		if (it != p.end())
		{
			if (!u->ptr)
				return luaL_error(L, "unit sudah tidak valid (hero sudah dibuang)");
			if (it->second.selfOnly && !u->isSelf)
				return luaL_error(L, "properti '%s' hanya untuk hero sendiri (self)", key);
			return it->second.get(L, u->ptr, u->isSelf ? static_cast<LuaHero *>(u->ptr) : nullptr);
		}
		// 3) field per-instance (hanya self)
		if (u->isSelf && u->ptr)
		{
			LuaHero *h = static_cast<LuaHero *>(u->ptr);
			lua_rawgeti(L, LUA_REGISTRYINDEX, h->_dataRef);
			lua_getfield(L, -1, key);
			return 1;
		}

		lua_pushnil(L);
		return 1;
	}

	static int mtNewIndex(lua_State *L)
	{
		LuaUnit *u = toUnit(L, 1);
		if (lua_type(L, 2) != LUA_TSTRING)
			return luaL_error(L, "nama properti harus string");
		const char *key = lua_tostring(L, 2);
		if (!u->ptr)
			return luaL_error(L, "unit sudah tidak valid (hero sudah dibuang)");

		if (std::strcmp(key, "state") == 0)
		{
			if (!u->isSelf)
				return luaL_error(L, "properti 'state' hanya bisa ditulis di hero sendiri (self)");
			setState(L, static_cast<LuaHero *>(u->ptr), 3);
			return 0;
		}

		const auto &p = props();
		auto it = p.find(key);
		if (it != p.end())
		{
			if (!it->second.set)
				return luaL_error(L, "properti '%s' read-only", key);
			if (it->second.selfOnly && !u->isSelf)
				return luaL_error(L, "properti '%s' hanya untuk hero sendiri (self)", key);
			it->second.set(L, 3, u->ptr, u->isSelf ? static_cast<LuaHero *>(u->ptr) : nullptr);
			return 0;
		}

		if (!u->isSelf)
			return luaL_error(L, "properti '%s' tidak ada / tidak bisa ditulis di unit lain", key);

		LuaHero *h = static_cast<LuaHero *>(u->ptr);
		lua_rawgeti(L, LUA_REGISTRYINDEX, h->_dataRef);
		lua_pushvalue(L, 2);
		lua_pushvalue(L, 3);
		lua_rawset(L, -3);
		return 0;
	}

	// ---------- method: semua unit ----------

	static int m_isValid(lua_State *L)
	{
		lua_pushboolean(L, toUnit(L, 1)->ptr ? 1 : 0);
		return 1;
	}

	static int m_distanceTo(lua_State *L)
	{
		CharacterBase *a = aliveUnit(L, 1);
		CharacterBase *b = aliveUnit(L, 2);
		const Vec2 d = b->getPosition() - a->getPosition();
		lua_pushnumber(L, d.x);
		lua_pushnumber(L, d.y);
		return 2;
	}

	// ---------- method: self, pencarian target ----------

#define LH_FIND_RANGE(fnName, call)                                       \
	static int fnName(lua_State *L)                                       \
	{                                                                     \
		LuaHero *h = selfOrError(L);                                      \
		const int range = (int)luaL_checkinteger(L, 2);                   \
		const bool master = lua_toboolean(L, 3) != 0;                     \
		lua_pushboolean(L, h->call(range, master) ? 1 : 0);               \
		return 1;                                                         \
	}

#define LH_FIND_HALF(fnName, call)                          \
	static int fnName(lua_State *L)                         \
	{                                                       \
		LuaHero *h = selfOrError(L);                        \
		lua_pushboolean(L, h->call() ? 1 : 0);              \
		return 1;                                           \
	}

	LH_FIND_RANGE(m_findHero, findHero)
	LH_FIND_RANGE(m_findFlog, findFlog)
	LH_FIND_RANGE(m_findTower, findTower)
	LH_FIND_RANGE(m_notFindHero, notFindHero)
	LH_FIND_RANGE(m_notFindFlog, notFindFlog)
	LH_FIND_RANGE(m_notFindTower, notFindTower)
	LH_FIND_HALF(m_findHeroHalf, findHeroHalf)
	LH_FIND_HALF(m_findFlogHalf, findFlogHalf)
	LH_FIND_HALF(m_findTowerHalf, findTowerHalf)
	LH_FIND_HALF(m_notFindHeroHalf, notFindHeroHalf)
	LH_FIND_HALF(m_notFindFlogHalf, notFindFlogHalf)
	LH_FIND_HALF(m_notFindTowerHalf, notFindTowerHalf)

#undef LH_FIND_RANGE
#undef LH_FIND_HALF

	static int m_target(lua_State *L)
	{
		LuaHero *h = selfOrError(L);
		pushUnit(L, h->_mainTarget.get(), true, false);
		return 1;
	}

	static int m_hasTarget(lua_State *L)
	{
		LuaHero *h = selfOrError(L);
		lua_pushboolean(L, h->_mainTarget != nullptr ? 1 : 0);
		return 1;
	}

	static int m_clearTarget(lua_State *L)
	{
		LuaHero *h = selfOrError(L);
		h->_mainTarget = nullptr;
		return 0;
	}

	static int m_setTarget(lua_State *L)
	{
		LuaHero *h = selfOrError(L);
		if (lua_isnoneornil(L, 2))
		{
			h->_mainTarget = nullptr;
			return 0;
		}
		h->_mainTarget = aliveUnit(L, 2);
		return 0;
	}

	static int m_distance(lua_State *L)
	{
		LuaHero *h = selfOrError(L);
		if (h->_mainTarget == nullptr)
		{
			lua_pushnil(L);
			return 1;
		}
		const Vec2 sp = h->getDistanceToTarget();
		lua_pushnumber(L, sp.x);
		lua_pushnumber(L, sp.y);
		return 2;
	}

	static int m_distanceIgnoreOriginY(lua_State *L)
	{
		LuaHero *h = selfOrError(L);
		if (h->_mainTarget == nullptr)
		{
			lua_pushnil(L);
			return 1;
		}
		const Vec2 sp = h->getDistanceToTargetAndIgnoreOriginY();
		lua_pushnumber(L, sp.x);
		lua_pushnumber(L, sp.y);
		return 2;
	}

	// ---------- method: self, gerak & serang ----------

	static int m_walk(lua_State *L)
	{
		LuaHero *h = selfOrError(L);
		Vec2 d((float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3));
		if (d.x != 0.0f || d.y != 0.0f)
			d = d.getNormalized();
		h->walk(d);
		return 0;
	}

	static int m_idle(lua_State *L)
	{
		selfOrError(L)->idle();
		return 0;
	}

	static int m_stepBack(lua_State *L)
	{
		lua_pushboolean(L, selfOrError(L)->stepBack() ? 1 : 0);
		return 1;
	}

	static int m_stepBack2(lua_State *L)
	{
		lua_pushboolean(L, selfOrError(L)->stepBack2() ? 1 : 0);
		return 1;
	}

	static int m_stepOn(lua_State *L)
	{
		selfOrError(L)->stepOn();
		return 0;
	}

	static int m_changeSide(lua_State *L)
	{
		LuaHero *h = selfOrError(L);
		h->changeSide(Vec2((float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3)));
		return 0;
	}

	static int m_attack(lua_State *L)
	{
		LuaHero *h = selfOrError(L);
		ABType t;
		if (!parseAttack(L, 2, t))
			return luaL_error(L, "tipe serangan tidak dikenal (pakai 'NAttack','SKILL1'..'SKILL3','OUGIS1','OUGIS2')");
		h->attack(t);
		return 0;
	}

	// ---------- method: self, gear & pola AI ----------

	static int m_useGear(lua_State *L)
	{
		LuaHero *h = selfOrError(L);
		GearType g;
		if (!parseGear(L, 2, g))
			return luaL_error(L, "gear tidak dikenal (pakai 'Gear00'..'Gear08')");
		h->useGear(g);
		return 0;
	}

	static int m_tryBuyGear(lua_State *L)
	{
		LuaHero *h = selfOrError(L);
		GearType g1, g2, g3;
		if (!parseGear(L, 2, g1) || !parseGear(L, 3, g2) || !parseGear(L, 4, g3))
			return luaL_error(L, "gear tidak dikenal (pakai 'Gear00'..'Gear08')");
		h->tryBuyGear(g1, g2, g3);
		return 0;
	}

	static int m_tryUseGear6(lua_State *L)
	{
		selfOrError(L)->tryUseGear6();
		return 0;
	}

	static int m_needBackToTowerToRestoreHP(lua_State *L)
	{
		LuaHero *h = selfOrError(L);
		const bool need = lua_isnoneornil(L, 2) ? true : (lua_toboolean(L, 2) != 0);
		lua_pushboolean(L, h->needBackToTowerToRestoreHP(need) ? 1 : 0);
		return 1;
	}

	static int m_needBackToDefendTower(lua_State *L)
	{
		LuaHero *h = selfOrError(L);
		const bool need = lua_isnoneornil(L, 2) ? true : (lua_toboolean(L, 2) != 0);
		lua_pushboolean(L, h->needBackToDefendTower(need) ? 1 : 0);
		return 1;
	}

	static int m_checkHealingState(lua_State *L)
	{
		selfOrError(L)->checkHealingState();
		return 0;
	}

	// ---------- method: self, animasi / lifecycle ----------

	static bool arrayByName(LuaHero *h, const char *name, CCArray *&out)
	{
		static const struct
		{
			const char *name;
			CCArray *LuaHero::*member;
		} kTable[] = {
			{"idle", &LuaHero::idleArray}, {"walk", &LuaHero::walkArray}, {"hurt", &LuaHero::hurtArray},
			{"airhurt", &LuaHero::airHurtArray}, {"knockdown", &LuaHero::knockDownArray},
			{"float", &LuaHero::floatArray}, {"dead", &LuaHero::deadArray}, {"nattack", &LuaHero::nattackArray},
			{"skill1", &LuaHero::skill1Array}, {"skill2", &LuaHero::skill2Array}, {"skill3", &LuaHero::skill3Array},
			{"skill4", &LuaHero::skill4Array}, {"skill5", &LuaHero::skill5Array},
			{"spc1", &LuaHero::skillSPC1Array}, {"spc2", &LuaHero::skillSPC2Array}, {"spc3", &LuaHero::skillSPC3Array},
			{"spc4", &LuaHero::skillSPC4Array}, {"spc5", &LuaHero::skillSPC5Array}};
		for (const auto &e : kTable)
			if (ieq(name, e.name))
			{
				out = h->*(e.member);
				return true;
			}
		return false;
	}

	// default fps/loop/returnToIdle per slot (sama dengan Hero::initAction())
	static bool slotDefaults(const char *slot, int &fps, bool &loop, bool &ret)
	{
		static const struct
		{
			const char *name;
			int fps;
			bool loop;
			bool ret;
		} kTable[] = {
			{"idle", 5, true, false}, {"walk", 10, true, false}, {"hurt", 10, false, false},
			{"airhurt", 10, false, false}, {"knockdown", 10, false, true}, {"float", 10, false, false},
			{"dead", 10, false, false}, {"nattack", 10, false, true}, {"skill1", 10, false, true},
			{"skill2", 10, false, true}, {"skill3", 10, false, true}, {"skill4", 10, false, true},
			{"skill5", 10, false, true}};
		for (const auto &e : kTable)
			if (ieq(slot, e.name))
			{
				fps = e.fps;
				loop = e.loop;
				ret = e.ret;
				return true;
			}
		return false;
	}

	static void applyAction(LuaHero *h, const char *slot, FiniteTimeAction *a)
	{
		if (ieq(slot, "idle")) h->setIdleAction(a);
		else if (ieq(slot, "walk")) h->setWalkAction(a);
		else if (ieq(slot, "hurt")) h->setHurtAction(a);
		else if (ieq(slot, "airhurt")) h->setAirHurtAction(a);
		else if (ieq(slot, "knockdown")) h->setKnockDownAction(a);
		else if (ieq(slot, "float")) h->setFloatAction(a);
		else if (ieq(slot, "dead")) h->setDeadAction(a);
		else if (ieq(slot, "nattack")) h->setNAttackAction(a);
		else if (ieq(slot, "skill1")) h->setSkill1Action(a);
		else if (ieq(slot, "skill2")) h->setSkill2Action(a);
		else if (ieq(slot, "skill3")) h->setSkill3Action(a);
		else if (ieq(slot, "skill4")) h->setSkill4Action(a);
		else if (ieq(slot, "skill5")) h->setSkill5Action(a);
	}

	static int m_setAction(lua_State *L)
	{
		LuaHero *h = selfOrError(L);
		const char *slot = luaL_checkstring(L, 2);
		const char *arr = lua_isnoneornil(L, 3) ? slot : luaL_checkstring(L, 3);

		int fps = 10;
		bool loop = false, ret = false;
		if (!slotDefaults(slot, fps, loop, ret))
			return luaL_error(L, "slot action tidak dikenal: %s", slot);

		CCArray *frames = nullptr;
		if (!arrayByName(h, arr, frames))
			return luaL_error(L, "array animasi tidak dikenal: %s", arr);
		if (!frames)
		{
			// unit ini tidak punya data animasi itu (mis. spc3 tidak ada di XML)
			lua_pushboolean(L, 0);
			return 1;
		}

		if (!lua_isnoneornil(L, 4))
			fps = (int)luaL_checkinteger(L, 4);
		if (!lua_isnoneornil(L, 5))
			loop = lua_toboolean(L, 5) != 0;
		if (!lua_isnoneornil(L, 6))
			ret = lua_toboolean(L, 6) != 0;

		applyAction(h, slot, h->createAnimation(frames, (uint8_t)fps, loop, ret));
		lua_pushboolean(L, 1);
		return 1;
	}

	static int m_lockOugisButtons(lua_State *L)
	{
		selfOrError(L)->lockOugisButtons();
		return 0;
	}

	static int m_unlockOugisButtons(lua_State *L)
	{
		selfOrError(L)->unlockOugisButtons();
		return 0;
	}

	static int m_startHealBuff(lua_State *L)
	{
		LuaHero *h = selfOrError(L);
		h->_healBuffValue = (uint32_t)luaL_checknumber(L, 2);
		const float interval = lua_isnoneornil(L, 3) ? 1.0f : (float)luaL_checknumber(L, 3);
		h->schedule(schedule_selector(CharacterBase::healBuff), interval);
		return 0;
	}

	static int m_stopHealBuff(lua_State *L)
	{
		selfOrError(L)->unschedule(schedule_selector(CharacterBase::healBuff));
		return 0;
	}

	static int m_baseChangeAction(lua_State *L)
	{
		selfOrError(L)->CharacterBase::changeAction();
		return 0;
	}

	static int m_baseResumeAction(lua_State *L)
	{
		LuaHero *h = selfOrError(L);
		h->CharacterBase::resumeAction((float)luaL_optnumber(L, 2, 0));
		return 0;
	}

	static int spawnAs(lua_State *L, Role role, const char *fn)
	{
		LuaHero *h = selfOrError(L);
		const char *name = luaL_checkstring(L, 2);
		if (!LuaHero::heroFactory)
		{
			CCLOG("[LuaAI] %s('%s'): Provider belum siap", fn, name);
			lua_pushnil(L);
			return 1;
		}
		// arg ke-3 (opsional, boolean): true = pakai class C++ bawaan unit itu, abaikan binding LuaAI.
		// Perlu untuk klon bernama sama dengan pemiliknya (mis. NarutoClone bernama "Naruto").
		const bool native = lua_toboolean(L, 3) != 0;
		LuaHero::bypassLuaAI = native;
		Hero *unit = LuaHero::heroFactory(string(name), role, h->getGroup());
		LuaHero::bypassLuaAI = false;
		pushUnit(L, unit, true, false);
		return 1;
	}

	// Buat unit lewat nama dengan role tertentu (AI-nya ikut LuaAI / CloneAI / class C++ unit itu).
	static int m_createSummon(lua_State *L) { return spawnAs(L, Role::Summon, "createSummon"); }
	static int m_createClone(lua_State *L) { return spawnAs(L, Role::Clone, "createClone"); }
	static int m_createKugutsu(lua_State *L) { return spawnAs(L, Role::Kugutsu, "createKugutsu"); }

	// _monsterArray.push_back(unit)  (dipakai Kankuro untuk kugutsu)
	static int m_addMonster(lua_State *L)
	{
		LuaHero *h = selfOrError(L);
		h->_monsterArray.push_back(aliveUnit(L, 2));
		return 0;
	}

	// setItem('Item1'): pakai item (setara setItem(Item1))
	static int m_setItem(lua_State *L)
	{
		LuaHero *h = selfOrError(L);
		ABType t;
		if (!parseAttack(L, 2, t))
			return luaL_error(L, "tipe item tidak dikenal (pakai 'Item1')");
		h->setItem(t);
		return 0;
	}

	// cancelResumeAction(): unschedule(schedule_selector(CharacterBase::resumeAction))
	static int m_cancelResumeAction(lua_State *L)
	{
		selfOrError(L)->unschedule(schedule_selector(CharacterBase::resumeAction));
		return 0;
	}

	// releaseControlledHeroes([idleAfter=false]): lepas semua hero yang sedang DIKENDALIKAN self
	// (possession Shintenshin Ino): _isControlled=false, tukar grup balik, lepas controller, buka kunci HUD.
	// GUARD sama seperti Ino.hpp: hanya hero yang controller-nya bernama Ino (bukan stun-lock Kabuto).
	// idleAfter=true  -> urutan Ino::resumeAction  (self idle() dulu kalau belum DEAD, sebelum changeGroup)
	// idleAfter=false -> urutan Ino::setActionResume
	static int m_releaseControlledHeroes(lua_State *L)
	{
		LuaHero *h = selfOrError(L);
		const bool idleAfter = lua_toboolean(L, 2) != 0;
		for (auto hero : getGameLayer()->_CharacterArray)
		{
			if (!hero->_isControlled || !hero->getController() || hero->getController()->getName() != HeroEnum::Ino)
				continue;

			hero->_isControlled = false;
			if (!idleAfter)
				hero->changeGroup();
			if (hero->isPlayer())
			{
				hero->_isAI = false;
				hero->unschedule(schedule_selector(CharacterBase::setAI));
				getGameLayer()->getHudLayer()->_isAllButtonLocked = false;
			}
			if (h->isPlayer())
				getGameLayer()->controlChar = nullptr;
			if (idleAfter)
			{
				if (h->getState() != State::DEAD)
					h->idle();
				hero->changeGroup();
			}
			hero->setController(nullptr);
		}
		return 0;
	}

	// createTrap('Kusuri', offsetX, offsetY): Bullet trap (Role::Mon) di posisi self + offset, langsung attack(NAttack)
	// (setara blok di Karin::onSetTrap; flip & offset dihitung di Lua)
	static int m_createTrap(lua_State *L)
	{
		LuaHero *h = selfOrError(L);
		const char *type = luaL_checkstring(L, 2);
		const float ox = (float)luaL_checknumber(L, 3);
		const float oy = (float)luaL_checknumber(L, 4);
		auto trap = Bullet::create();
		trap->setAnchorPoint(Vec2(0.5f, 0));
		trap->setMaster(h);
		trap->setID(string(type), Role::Mon, h->getGroup());
		trap->setPosition(Vec2(h->getPositionX() + ox, h->getPositionY() + oy));
		trap->attack(NAttack);
		getGameLayer()->addChild(trap, -trap->getPositionY());
		return 0;
	}

	static int m_knockDown(lua_State *L) { selfOrError(L)->knockDown(); return 0; }

	// resetDefenseValue(v): def = (def > v) ? def - v : 0   (unit mana pun)
	static int m_resetDefenseValue(lua_State *L)
	{
		aliveUnit(L, 1)->resetDefenseValue((int)luaL_checkinteger(L, 2));
		return 0;
	}

	// setupSpcSkill1Clone(clone): klon memakai SKILL1 versi spc milik self
	// (setara blok di Naruto::createClone_SageNaruto / createClone_RikudoNaruto)
	static int m_setupSpcSkill1Clone(lua_State *L)
	{
		LuaHero *h = selfOrError(L);
		CharacterBase *c = aliveUnit(L, 2);
		c->setSkill1Action(c->createAnimation(c->skillSPC1Array, 10, false, true));
		c->setSAttackValue1(h->getSAttackValue1());
		c->setSAttackType1(h->_spcAttackType1);
		c->_sAttackCD1 = h->_spcAttackCD1;
		return 0;
	}

	// setPosition(x, y) / setAnchorPoint(x, y): untuk unit mana pun (mis. hasil createSummon)
	static int m_setPosition(lua_State *L)
	{
		aliveUnit(L, 1)->setPosition(Vec2((float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3)));
		return 0;
	}
	static int m_setAnchorPoint(lua_State *L)
	{
		aliveUnit(L, 1)->setAnchorPoint(Vec2((float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3)));
		return 0;
	}

	// revealEnemies([range=kAttackRange]): musuh (hero Player/Com, bukan HURT/DEAD) yang tak terlihat dan
	// berada dalam jarak x < range jadi terlihat lagi (setara loop di Kakashi::changeAction / Sharingan)
	static int m_revealEnemies(lua_State *L)
	{
		LuaHero *h = selfOrError(L);
		const float range = (float)luaL_optnumber(L, 2, (lua_Number)kAttackRange);
		for (auto hero : getGameLayer()->_CharacterArray)
		{
			if (h->getGroup() == hero->getGroup() ||
				!hero->isPlayerOrCom() ||
				hero->getState() == State::HURT ||
				hero->getState() == State::DEAD)
				continue;
			if ((hero->getPosition() - h->getPosition()).x >= range || hero->_isVisable)
				continue;

			if (hero->getName() == HeroEnum::Konan || hero->getName() == HeroEnum::Deidara)
				hero->unschedule(schedule_selector(CharacterBase::disableBuff));

			hero->setOpacity(255);
			hero->setVisible(true);
			if (hero->_hpBar)
				hero->_hpBar->setVisible(true);
			if (hero->_shadow)
				hero->_shadow->setVisible(true);
			hero->_isVisable = true;
		}
		return 0;
	}

	// setSkillButtonFrame(slot 1|2, 'Kakashi_skill1_1.png'): ganti ikon tombol HUD (hanya jika self Player)
	static int m_setSkillButtonFrame(lua_State *L)
	{
		LuaHero *h = selfOrError(L);
		const int slot = (int)luaL_checkinteger(L, 2);
		const char *frameName = luaL_checkstring(L, 3);
		if (!h->isPlayer())
			return 0;
		auto frame = getSpriteFrame(string(frameName));
		if (!frame)
			return luaL_error(L, "SpriteFrame '%s' tidak ditemukan", frameName);
		auto hud = getGameLayer()->getHudLayer();
		if (slot == 1) hud->skill1Button->setDisplayFrame(frame);
		else if (slot == 2) hud->skill2Button->setDisplayFrame(frame);
		else return luaL_error(L, "slot tombol harus 1 atau 2");
		return 0;
	}

	static int m_lockSkill4Button(lua_State *L) { selfOrError(L)->lockSkill4Button(); return 0; }
	static int m_unlockSkill4Button(lua_State *L) { selfOrError(L)->unlockSkill4Button(); return 0; }
	static int m_lockSkill5Button(lua_State *L) { selfOrError(L)->lockSkill5Button(); return 0; }
	static int m_unlockSkill5Button(lua_State *L) { selfOrError(L)->unlockSkill5Button(); return 0; }

	static int m_data(lua_State *L)
	{
		LuaHero *h = selfOrError(L);
		lua_rawgeti(L, LUA_REGISTRYINDEX, h->_dataRef);
		return 1;
	}

	// ---------- registrasi ----------

	static void ensureTypes(lua_State *L)
	{
		if (!luaL_newmetatable(L, kMT))
		{
			lua_pop(L, 1); // sudah terdaftar
			return;
		}

		static const struct
		{
			const char *name;
			lua_CFunction func;
		} kMethods[] = {
			// semua unit
			{"isValid", m_isValid},
			{"distanceTo", m_distanceTo},
			// target
			{"findHero", m_findHero},
			{"findFlog", m_findFlog},
			{"findTower", m_findTower},
			{"notFindHero", m_notFindHero},
			{"notFindFlog", m_notFindFlog},
			{"notFindTower", m_notFindTower},
			{"findHeroHalf", m_findHeroHalf},
			{"findFlogHalf", m_findFlogHalf},
			{"findTowerHalf", m_findTowerHalf},
			{"notFindHeroHalf", m_notFindHeroHalf},
			{"notFindFlogHalf", m_notFindFlogHalf},
			{"notFindTowerHalf", m_notFindTowerHalf},
			{"target", m_target},
			{"hasTarget", m_hasTarget},
			{"clearTarget", m_clearTarget},
			{"setTarget", m_setTarget},
			{"distance", m_distance},
			{"distanceIgnoreOriginY", m_distanceIgnoreOriginY},
			// gerak & serang
			{"walk", m_walk},
			{"idle", m_idle},
			{"stepBack", m_stepBack},
			{"stepBack2", m_stepBack2},
			{"stepOn", m_stepOn},
			{"changeSide", m_changeSide},
			{"attack", m_attack},
			// gear & pola AI
			{"useGear", m_useGear},
			{"tryBuyGear", m_tryBuyGear},
			{"tryUseGear6", m_tryUseGear6},
			{"needBackToTowerToRestoreHP", m_needBackToTowerToRestoreHP},
			{"needBackToDefendTower", m_needBackToDefendTower},
			{"checkHealingState", m_checkHealingState},
			// animasi / lifecycle
			{"setAction", m_setAction},
			{"lockOugisButtons", m_lockOugisButtons},
			{"unlockOugisButtons", m_unlockOugisButtons},
			{"startHealBuff", m_startHealBuff},
			{"stopHealBuff", m_stopHealBuff},
			{"baseChangeAction", m_baseChangeAction},
			{"baseResumeAction", m_baseResumeAction},
			{"createSummon", m_createSummon},
			{"createClone", m_createClone},
			{"createKugutsu", m_createKugutsu},
			{"addMonster", m_addMonster},
			{"setItem", m_setItem},
			{"cancelResumeAction", m_cancelResumeAction},
			{"releaseControlledHeroes", m_releaseControlledHeroes},
			{"createTrap", m_createTrap},
			{"knockDown", m_knockDown},
			{"resetDefenseValue", m_resetDefenseValue},
			{"setupSpcSkill1Clone", m_setupSpcSkill1Clone},
			{"setPosition", m_setPosition},
			{"setAnchorPoint", m_setAnchorPoint},
			{"revealEnemies", m_revealEnemies},
			{"setSkillButtonFrame", m_setSkillButtonFrame},
			{"lockSkill4Button", m_lockSkill4Button},
			{"unlockSkill4Button", m_unlockSkill4Button},
			{"lockSkill5Button", m_lockSkill5Button},
			{"unlockSkill5Button", m_unlockSkill5Button},
			{"data", m_data},
		};

		// stack: [mt]
		lua_newtable(L);
		for (const auto &m : kMethods)
		{
			lua_pushcfunction(L, m.func);
			lua_setfield(L, -2, m.name);
		}
		lua_setfield(L, -2, "__methods");

		lua_pushcfunction(L, mtIndex);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, mtNewIndex);
		lua_setfield(L, -2, "__newindex");
		lua_pushcfunction(L, mtGC);
		lua_setfield(L, -2, "__gc");
		lua_pushcfunction(L, mtEq);
		lua_setfield(L, -2, "__eq");
		lua_pushcfunction(L, mtToString);
		lua_setfield(L, -2, "__tostring");
		lua_pop(L, 1);
	}
};

// =====================================================================================
// LuaHero
// =====================================================================================

inline LuaHero *LuaHero::createFromScript(const string &script)
{
	if (!LuaBridge::isReady())
		return nullptr;

	lua_getL;
	if (!LuaAISystem::pushScript(L, script)) // stack: [tabel script]
		return nullptr;

	LuaHeroBinding::ensureTypes(L);

	auto *hero = new LuaHero();
	hero->_script = script;
	hero->_scriptRef = luaL_ref(L, LUA_REGISTRYINDEX); // pop tabel script
	lua_newtable(L);
	hero->_dataRef = luaL_ref(L, LUA_REGISTRYINDEX);
	return hero;
}

inline LuaHero::~LuaHero()
{
	if (!LuaBridge::isReady())
		return;

	lua_getL;
	if (_proxyRef != LUA_NOREF)
	{
		// Proxy self mungkin masih dipegang script (disimpan di variabel): putuskan supaya
		// pemakaian sesudah hero dibuang melempar error Lua, bukan crash.
		lua_rawgeti(L, LUA_REGISTRYINDEX, _proxyRef);
		if (auto *u = (LuaUnit *)lua_touserdata(L, -1))
			u->ptr = nullptr;
		lua_pop(L, 1);
		luaL_unref(L, LUA_REGISTRYINDEX, _proxyRef);
	}
	if (_dataRef != LUA_NOREF)
		luaL_unref(L, LUA_REGISTRYINDEX, _dataRef);
	if (_scriptRef != LUA_NOREF)
		luaL_unref(L, LUA_REGISTRYINDEX, _scriptRef);
}

inline void LuaHero::pushSelf(lua_State *L)
{
	if (_proxyRef != LUA_NOREF)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, _proxyRef);
		return;
	}
	LuaHeroBinding::pushUnit(L, this, false, true);
	lua_pushvalue(L, -1);
	_proxyRef = luaL_ref(L, LUA_REGISTRYINDEX);
}

inline bool LuaHero::hasHook(lua_State *L, const char *hook)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, _scriptRef);
	lua_getfield(L, -1, hook);
	const bool ok = lua_isfunction(L, -1);
	lua_pop(L, 2);
	return ok;
}

template <typename PushExtra>
inline bool LuaHero::callHook(lua_State *L, const char *hook, int nresults, int nextra, PushExtra pushExtra)
{
	// message handler = debug.traceback supaya log error punya jejak pemanggilan
	lua_getglobal(L, "debug");
	if (lua_istable(L, -1))
		lua_getfield(L, -1, "traceback");
	else
		lua_pushnil(L);
	lua_remove(L, -2);
	const int errIdx = lua_isfunction(L, -1) ? lua_gettop(L) : 0;

	lua_rawgeti(L, LUA_REGISTRYINDEX, _scriptRef);
	lua_getfield(L, -1, hook);
	if (!lua_isfunction(L, -1))
		return false;
	lua_remove(L, -2);

	pushSelf(L);
	pushExtra(L);
	if (lua_pcall(L, 1 + nextra, nresults, errIdx) != LUA_OK)
	{
		if (_errorCount < LuaHeroBinding::kMaxErrorLogs)
		{
			const char *err = lua_tostring(L, -1);
			CCLOG("[LuaAI] %s.%s (unit %s) error: %s", _script.c_str(), hook, getName().c_str(), err ? err : "<no error>");
			if (++_errorCount == LuaHeroBinding::kMaxErrorLogs)
				CCLOG("[LuaAI] %s (unit %s): error berikutnya tidak dicatat lagi", _script.c_str(), getName().c_str());
		}
		return false;
	}
	return true;
}

inline void LuaHero::perform()
{
	if (_scriptRef == LUA_NOREF || !LuaBridge::isReady())
		return;

	lua_getL;
	const int top = lua_gettop(L);
	callHook(L, "perform", 0, 0, [](lua_State *) {});
	lua_settop(L, top);
}

inline void LuaHero::changeAction()
{
	if (_scriptRef == LUA_NOREF || !LuaBridge::isReady())
	{
		CharacterBase::changeAction();
		return;
	}

	lua_getL;
	if (!hasHook(L, "changeAction"))
	{
		CharacterBase::changeAction();
		return;
	}
	const int top = lua_gettop(L);
	callHook(L, "changeAction", 0, 0, [](lua_State *) {});
	lua_settop(L, top);
}

inline void LuaHero::resumeAction(float dt)
{
	if (_scriptRef == LUA_NOREF || !LuaBridge::isReady())
	{
		CharacterBase::resumeAction(dt);
		return;
	}

	lua_getL;
	if (!hasHook(L, "resumeAction"))
	{
		CharacterBase::resumeAction(dt);
		return;
	}
	const int top = lua_gettop(L);
	const bool ok = callHook(L, "resumeAction", 0, 1, [dt](lua_State *L) { lua_pushnumber(L, dt); });
	lua_settop(L, top);
	if (!ok)
		CharacterBase::resumeAction(dt); // idempotent: jaga state tetap konsisten kalau hook error
}

inline void LuaHero::setActionResume()
{
	// base CharacterBase::setActionResume() kosong, jadi tanpa hook = tidak melakukan apa-apa
	if (_scriptRef == LUA_NOREF || !LuaBridge::isReady())
		return;

	lua_getL;
	if (!hasHook(L, "setActionResume"))
		return;
	const int top = lua_gettop(L);
	callHook(L, "setActionResume", 0, 0, [](lua_State *) {});
	lua_settop(L, top);
}

// hook opsional: onSetTrap(self, trapType)   (setara override Karin::onSetTrap)
inline void LuaHero::onSetTrap(const string &trapType)
{
	if (_scriptRef == LUA_NOREF || !LuaBridge::isReady())
		return;

	lua_getL;
	if (!hasHook(L, "onSetTrap"))
		return;
	const int top = lua_gettop(L);
	callHook(L, "onSetTrap", 0, 1, [&trapType](lua_State *L) { lua_pushstring(L, trapType.c_str()); });
	lua_settop(L, top);
}

inline Hero *LuaHero::createClone(int cloneTime)
{
	if (_scriptRef == LUA_NOREF || !LuaBridge::isReady())
		return CharacterBase::createClone(cloneTime);

	lua_getL;
	if (!hasHook(L, "createClone"))
		return CharacterBase::createClone(cloneTime);

	const int top = lua_gettop(L);
	Hero *clone = nullptr;
	if (callHook(L, "createClone", 1, 1, [cloneTime](lua_State *L) { lua_pushinteger(L, cloneTime); }))
	{
		if (auto *u = LuaHeroBinding::testUnit(L, -1))
		{
			clone = u->ptr ? dynamic_cast<Hero *>(u->ptr) : nullptr;
		}
		else if (!lua_isnil(L, -1))
		{
			CCLOG("[LuaAI] %s.createClone harus return hasil self:createSummon(...) atau nil", _script.c_str());
		}
	}
	lua_settop(L, top);
	return clone;
}
