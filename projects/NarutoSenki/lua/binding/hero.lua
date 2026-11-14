--
-- binding/hero.lua  --  sisi Lua dari binding C++ (Classes/Systems/HeroBinding.hpp)
--
-- File ini TIDAK berisi logika AI. Tugasnya hanya:
--   * memastikan method native CharacterBase & konstanta (ABType, GearType, State) sudah
--     didaftarkan C++ sebelum AI Lua dimuat (satu tempat, satu pesan error yang jelas)
--   * menyediakan helper kecil murni-Lua yang dipakai bersama oleh banyak AI
--
-- Semua method native dipanggil dengan sintaks `self:namaMethod(...)` pada userdata "CharacterBase".
-- Daftar lengkap + signature ada di komentar header Classes/Systems/HeroBinding.hpp.
--
local HeroBinding = {}

-- Method yang wajib ada pada kelas native CharacterBase (dicek lewat tabel kelas tolua).
local REQUIRED = {
    'idle', 'walk', 'stepOn', 'stepBack2', 'changeSide', 'attack', 'useGear', 'tryBuyGear',
    'findHeroHalf', 'findTowerHalf', 'notFindFlogHalf',
    'needBackToTowerToRestoreHP', 'needBackToDefendTower', 'checkHealingState',
    'clearMainTarget', 'hasMainTarget', 'mainTargetIsFlog', 'mainTargetIsNotFlog',
    'getMainTargetDEF', 'getDistanceToTarget',
    'isFreeState', 'getState', 'setState', 'getFlag', 'setFlag', 'getNum', 'setNum', 'getGameFlag',
    'getNAttackValue', 'setNAttackValue', 'getNAttackType', 'setNAttackType', 'getSpcAttack3Type',
    'getUID', 'setAnimation', 'startHealBuff', 'lockOugisButtons', 'unlockOugisButtons',
    'callBaseResumeAction', 'createCloneHero', 'createSummonHero',
}

-- true kalau semua binding native tersedia
function HeroBinding.check()
    local missing = {}
    if not (ABType and GearType and State) then
        missing[#missing + 1] = 'ABType/GearType/State'
    end
    if not CharacterBase then
        missing[#missing + 1] = 'CharacterBase (kelas tolua)'
    else
        for _, name in ipairs(REQUIRED) do
            if rawget(CharacterBase, name) == nil then missing[#missing + 1] = name end
        end
    end
    if #missing > 0 then
        log('[binding.hero] binding native belum lengkap: %s', table.concat(missing, ', '))
        return false
    end
    return true
end

-- Vektor satuan (setara Vec2::getNormalized(); vektor nol dikembalikan apa adanya)
function HeroBinding.normalize(x, y)
    local n = math.sqrt(x * x + y * y)
    if n < 1e-30 then return x, y end
    return x / n, y / n
end

-- Jalan menuju (sx, sy) = jarak ke target, setara `walk(sp.getNormalized())` di C++
function HeroBinding.walkToward(self, sx, sy)
    local dx, dy = HeroBinding.normalize(sx, sy)
    self:walk(dx, dy)
end

-- State per-instance (mis. ganti member C++ `_originNAttackType`). Dikunci dengan getUID().
-- Isi per instance harus dibersihkan sendiri oleh AI (set nil) saat sudah tidak dipakai.
function HeroBinding.newInstanceStore()
    local store = {}
    return {
        get = function(self) return store[self:getUID()] end,
        set = function(self, v) store[self:getUID()] = v end,
    }
end

return HeroBinding
