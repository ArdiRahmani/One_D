# LuaAI — bikin AI karakter dari Lua

`CloneAI` (lua/Hook/CloneAI.lua) hanya bisa **meminjam class C++** yang sudah ada.
`LuaAI` menjalankan AI yang **ditulis di Lua** (`lua/AI/<Script>.lua`) lewat kelas `LuaHero`
(`Classes/Core/LuaHero.hpp`). Binding-nya langsung menyentuh `Hero` / `CharacterBase`, jadi
script Lua bisa melakukan hal yang sama dengan `perform()` di file `.hpp`.

## Cara pakai (contoh: Yagura pakai AI Tsunade.lua)

1. Script AI: `lua/AI/Tsunade.lua` (sudah ada, port 1:1 dari `Tsunade.hpp`).
2. Binding unit -> script, di `lua/Hook/LuaAI.lua`:

   ```lua
   local list = {
       Yagura = 'Tsunade',     -- unit Yagura menjalankan lua/AI/Tsunade.lua
   }
   ```
3. Hasilnya:
   * `Yagura`  -> `LuaHero` + `lua/AI/Tsunade.lua`
   * `Tsunade` -> tetap class C++ `Tsunade` (`Tsunade.hpp`), tidak ada yang berubah
4. Supaya Yagura bisa dipilih / muncul sebagai COM:
   * resource unit harus ada: `Resources/Unit/Ninja/Yagura/Yagura.xml` (+ plist)
   * `lua/Hook/Heroes.lua` -> tambahkan `'Yagura'` ke `roster`
   * aset Slug milik Tsunade: `lua/Hook/Assets.lua` -> `Yagura = { kuchiyose = 'Slug' }`

Prioritas saat `Provider::create(nama)`: **LuaAI** > CloneAI > class C++ bernama sama > DefaultAI.
Kalau script Lua gagal dimuat (file tidak ada, error sintaks, tidak ada `perform`), game menulis
`[LuaAI] ...` ke log dan jatuh balik ke AI C++, jadi tidak crash.

Satu script boleh dipakai banyak unit: `Yagura = 'Tsunade', Mei = 'Tsunade'`.

## Bentuk script

```lua
-- lua/AI/NamaScript.lua
local AI = {}

function AI.perform(self)            -- WAJIB. Dipanggil tiap tick AI (setara Hero::perform)
end
function AI.changeAction(self) end   -- opsional (setara override changeAction)
function AI.resumeAction(self, dt) end -- opsional (setara override resumeAction)
function AI.createClone(self, t)     -- opsional, return self:createSummon('Nama') atau nil
end
function AI.setActionResume(self) end   -- opsional (setara override setActionResume, mis. Hashirama.lua)

return AI
```

Tanpa file, langsung dari Lua lain:

```lua
LuaAI.define('Diam', { perform = function(self) self:idle() end })
LuaAI.set('Mei', 'Diam')
```

Tabel global `LuaAI`: `set(unit, script)`, `set(unit, nil)`, `setAll{}`, `remove`, `clear`, `get`,
`has`, `count`, `define(script, tabel)`, `reload([script])`.

## Contoh AI dari nol

```lua
-- lua/AI/Simple.lua : kejar hero musuh, serang, pakai SKILL1 kalau siap
local abs = math.abs
local AI = {}

function AI.perform(self)
    self:clearTarget()
    if not self:findHeroHalf() and self:notFindFlogHalf() then
        self:findTowerHalf()
    end

    local t = self:target()
    if not t then return self:checkHealingState() end

    local dx, dy = self:distance()
    if abs(dx) > 32 or abs(dy) > 32 then
        self:walk(dx, dy)
    elseif self.isFreeState then
        self:changeSide(dx, dy)
        self:attack(self.canSkill1 and 'SKILL1' or 'NAttack')
    end
end

return AI
```

## API `self` (hero sendiri)

Method dipanggil dengan `:`; properti dengan `.`.

| Kelompok | Isi |
|---|---|
| Cari target | `findHero(range[,master])` `findFlog(..)` `findTower(..)` `notFindHero/Flog/Tower(..)` · `findHeroHalf()` `findFlogHalf()` `findTowerHalf()` `notFindHeroHalf/FlogHalf/TowerHalf()` · `target()` `hasTarget()` `clearTarget()` `setTarget(unit)` · `distance()` -> `dx, dy` · `distanceIgnoreOriginY()` |
| Gerak & serang | `walk(dx, dy)` (otomatis dinormalisasi) · `idle()` · `stepBack()` `stepBack2()` -> bool · `stepOn()` · `changeSide(dx, dy)` · `attack('NAttack'/'SKILL1'/'SKILL2'/'SKILL3'/'OUGIS1'/'OUGIS2')` |
| Gear & pola siap pakai | `useGear('Gear03')` · `tryBuyGear(g1,g2,g3)` · `tryUseGear6()` · `needBackToTowerToRestoreHP([bool])` · `needBackToDefendTower([bool])` · `checkHealingState()` |
| Animasi & lifecycle | `setAction(slot[, array[, fps[, loop[, returnToIdle]]]])` · `lockOugisButtons()` `unlockOugisButtons()` · `startHealBuff(value[,interval])` `stopHealBuff()` · `baseChangeAction()` `baseResumeAction(dt)` · `createSummon('Slug')` · `data()` |
| Semua unit | `isValid()` · `distanceTo(unit)` -> `dx, dy` |

`setAction` — slot: `idle walk hurt airhurt knockdown float dead nattack skill1..skill5`;
array: slot yang sama + `spc1..spc5`. Tanpa `array` = kembali ke animasi bawaan slot itu.

### Properti (semua unit, termasuk hasil `self:target()`)

`name role group state hp maxHp hpPercent def level exp ckr ckr2 coin x y height walkSpeed
nAttackValue nAttackType isFlog isNotFlog isTower isNotTower isCom isPlayer isPlayerOrCom isClone
isKugutsu isSummon isMon isBullet isGuardian isArmored isInvincible isControlled isHealing canGear06`

* `role`: `'Com' 'Player' 'Hero' 'Flog' 'Tower' 'Bullet' 'Clone' 'Kugutsu' 'Mon' 'Summon'`
* `state`: `'IDLE' 'WALK' 'NATTACK' 'SATTACK' 'OATTACK' 'O2ATTACK' 'HURT' 'KNOCKDOWN' 'FLOAT' 'JUMP' 'AIRHURT' 'DEAD'`

### Properti khusus `self`

Baca: `canSkill1 canSkill2 canSkill3 canOugis1 canOugis2 canGear00 canGear03 isBaseDanger
enemyCombatPoint friendCombatPoint isOugis2Game isFreeState gearCount spcAttackType1..3`
Baca + tulis: `isOnlySkillLocked healBuffValue nAttackRangeX nAttackRangeY state`
Bisa ditulis di semua unit: `isArmored nAttackValue nAttackType walkSpeed originSpeed tempAttackValue1`
Baca + tulis (self): `skillChangeBuffValue`

> Di Lua `0` itu *truthy*: `!_skillChangeBuffValue` di C++ ditulis `self.skillChangeBuffValue == 0`.

### Variabel milik script (setara anggota `private` di C++)

Field lain yang kamu tulis ke `self` disimpan per-instance:

```lua
self.originNAttackType = self.nAttackType   -- setara  string _originNAttackType;
```

## Keamanan & error

* `self` otomatis tidak valid begitu hero dibuang C++; memanggilnya melempar error Lua, bukan crash.
  `self:target()` memegang retain, jadi aman disimpan.
* Error di dalam hook dicatat ke log (`[LuaAI] Script.hook (unit X) error: ...` + traceback),
  maksimal 5 kali per hero agar log tidak banjir. Hero lanjut ke tick berikutnya.
* `LuaAI.reload('Tsunade')` memuat ulang script untuk unit yang **dibuat sesudahnya**
  (unit yang sudah ada tetap memakai versi lama).

## Batasan

LuaAI mengganti AI + hook lifecycle saja. Logika C++ yang membandingkan **nama unit**
(mis. `CharacterBase::healBuff` hanya menyembuhkan jika nama == Tsunade/Sakura, dan batas buff 15 dtk
untuk Tsunade) tetap berlaku per nama, jadi untuk Yagura efek itu tidak ikut aktif
kecuali ditambahkan di C++.
