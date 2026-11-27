--
-- AI Naruto versi Lua (port 1:1 dari Classes/Core/Shinobi/Naruto.hpp)
--
-- Satu script untuk tiga bentuk, sama seperti C++ (Provider: "Naruto", "SageNaruto", "RikudoNaruto" -> class Naruto):
--   * name == 'Naruto'        -> perform_Naruto        + changeAction/resumeAction/createClone Naruto (Kyuubi)
--   * name == 'SageNaruto'    -> perform_SageNaruto    + ...
--   * name == 'RikudoNaruto'  -> perform_RikudoNaruto  + ...
-- Nama selain tiga itu: perform memakai bentuk Naruto, hook lain tidak melakukan apa-apa
-- (sama seperti match_char_exp3 di C++ yang tidak cocok).
--
-- Cara bind di lua/Hook/LuaAI.lua (tiga nama harus didaftarkan kalau mau ketiganya pakai Lua):
--     Naruto = 'Naruto', SageNaruto = 'Naruto', RikudoNaruto = 'Naruto',
-- Atau unit lain:  Mei = 'Naruto'   (hanya perform bentuk Naruto, karena nama tidak cocok)
--
-- Daftar API `self` / `target`: lua/AI/README.md
--
-- CATATAN PORT:
--   * createClone memakai  self:createClone(self.name, true)  -> argumen `true` = pakai class C++ bawaan
--     (NarutoClone / SageNarutoClone / RikudoNarutoClone), bukan LuaHero, walau nama ini ada di LuaAI.
--     Kalau unit bukan salah satu dari tiga nama di atas, harus ada CloneAI (mis. Mei -> 'Naruto')
--     supaya Provider tahu class klonnya.
--   * Kurama (cloneTime == 9) dibuat lewat self:createSummon('Kurama').
--   * Butuh binding tambahan di LuaHero.hpp: def (r/w), hasArmorBroken (r/w), spcAttackRangeX3/Y3,
--     knockDown, resetDefenseValue, setupSpcSkill1Clone, createClone(name, native).
--
local abs = math.abs

local Naruto = {}

-- ============================================================================
-- Naruto (bentuk biasa)
-- ============================================================================

-- setara  void Naruto::perform()
local function perform_Naruto(self)
    self:clearTarget()                    -- _mainTarget = nullptr
    self:findHeroHalf()

    self:tryBuyGear('Gear03', 'Gear07', 'Gear02')

    if self:needBackToTowerToRestoreHP() or self:needBackToDefendTower() then
        return
    end

    local target = self:target()
    if target and target.isNotFlog then
        local sx, sy = self:distance()    -- getDistanceToTarget()

        if self.isFreeState then
            if self.canOugis2 and not self.isControlled and self.isOugis2Game and not self.isArmored then
                self:changeSide(sx, sy)
                self:attack('OUGIS2')
                return

            elseif self.canSkill2 and not self.isArmored then
                self:changeSide(sx, sy)
                self:attack('SKILL2')
                return

            elseif self.enemyCombatPoint > self.friendCombatPoint
                and abs(self.enemyCombatPoint - self.friendCombatPoint) > 3000
                and not self.isHealing and not self.isArmored and not self.isControlled then
                if abs(sx) < 160 then
                    self:stepBack2()
                else
                    self:idle()
                end
                return

            elseif abs(sx) < 128 then
                if abs(sx) > 96 or abs(sy) > 32 then
                    self:walk(sx, sy)
                    return
                elseif abs(sx) < 32 and (self.canSkill1 or self.canOugis1 or self.canSkill3) then
                    self:stepBack()
                    return
                end

                if self.canOugis1 and not self.isControlled and target.def < 5000 and not self.isArmored then
                    self:changeSide(sx, sy)
                    self:attack('OUGIS1')
                elseif self.canGear03 then
                    self:useGear('Gear03')
                elseif self.canSkill3 and not self.isArmored and target.def < 5000 then
                    self:changeSide(sx, sy)
                    self:attack('SKILL3')
                elseif self.canSkill1 and not self.isArmored and target.def < 5000 then
                    self:changeSide(sx, sy)
                    self:attack('SKILL1')
                else
                    self:changeSide(sx, sy)
                    self:attack('NAttack')
                end
                return
            end
        end
    end

    self:clearTarget()
    if self:notFindFlogHalf() then
        self:findTowerHalf()
    end

    target = self:target()
    if target then
        local sx, sy = self:distance()

        if abs(sx) > 32 or abs(sy) > 32 then
            self:walk(sx, sy)
            return
        end

        if self.isFreeState then
            if self.canSkill2 and not self.isArmored then
                self:changeSide(sx, sy)
                self:attack('SKILL2')
                return
            else
                self:changeSide(sx, sy)
                self:attack('NAttack')
            end
        end
        return
    end

    self:checkHealingState()
end

-- setara  changeAction_Naruto()   (mode Kyuubi)
local function changeAction_Naruto(self)
    self:setAction('idle',      'spc1', 5,  true,  false)
    self:setAction('walk',      'spc2', 10, true,  false)
    self:setAction('nattack',   'spc3', 10, false, true)
    self:setAction('knockdown', 'spc4', 10, false, true)

    self.walkSpeed = 320
    self.originSpeed = 320

    self.isOnlySkillLocked = true

    self.tempAttackValue1 = self.nAttackValue
    self.nAttackValue = 960

    self.nAttackRangeX = self.spcAttackRangeX3
    self.nAttackRangeY = self.spcAttackRangeY3
    self.originNAttackType = self.nAttackType      -- setara  string _originNAttackType (private C++)
    self.nAttackType = self.spcAttackType3

    self.def = self.def + 5000                     -- _defense += 5000
    self.isArmored = true

    self:lockOugisButtons()
end

-- setara  resumeAction_Naruto(dt)
local function resumeAction_Naruto(self, dt)
    -- tanpa array = kembalikan ke animasi bawaan slot itu
    self:setAction('idle')
    self:setAction('walk')
    self:setAction('nattack')

    self.isOnlySkillLocked = false

    self:unlockOugisButtons()

    self.walkSpeed = 224
    self.originSpeed = 224
    if self.tempAttackValue1 > 0 then              -- hasTempAttackValue1()
        self.nAttackValue = self.tempAttackValue1
        self.tempAttackValue1 = 0
    end
    self.hasArmorBroken = false

    self.nAttackRangeX = 16
    self.nAttackRangeY = 48
    self.nAttackType = self.originNAttackType or self.nAttackType
    self:resetDefenseValue(5000)
    self.isArmored = false

    if self.state ~= 'DEAD' then
        self.state = 'WALK'
        self:knockDown()
    end

    self:setAction('knockdown')
    self:baseResumeAction(dt)                      -- CharacterBase::resumeAction(dt)
end

-- setara  createClone_Naruto(cloneTime)
local function createClone_Naruto(self, cloneTime)
    return self:createClone(self.name, true)       -- createCloneHero<NarutoClone>(getName())
end

-- ============================================================================
-- Sage Naruto
-- ============================================================================

-- setara  void Naruto::perform_SageNaruto()
local function perform_SageNaruto(self)
    self:clearTarget()
    self:findHeroHalf()

    self:tryBuyGear('Gear03', 'Gear07', 'Gear02')

    if self:needBackToTowerToRestoreHP() or self:needBackToDefendTower() then
        return
    end

    local target = self:target()
    if target and target.isNotFlog then
        local sx, sy = self:distance()

        if self.isFreeState then
            if self.canOugis2 and not self.isControlled and self.isOugis2Game and not self.isArmored then
                self:changeSide(sx, sy)
                self:attack('OUGIS2')
                return

            elseif self.canSkill2 and not self.isArmored then
                self:changeSide(sx, sy)
                self:attack('SKILL2')
                return

            elseif self.canSkill3 and not self.isArmored and target.def < 5000 then
                self:changeSide(sx, sy)
                self:attack('SKILL3')
                return

            elseif self.enemyCombatPoint > self.friendCombatPoint
                and abs(self.enemyCombatPoint - self.friendCombatPoint) > 3000
                and not self.isHealing and not self.isArmored and not self.isControlled then
                if abs(sx) < 160 then
                    self:stepBack2()
                else
                    self:idle()
                end
                return

            elseif abs(sx) < 128 then
                if abs(sx) > 96 or abs(sy) > 32 then
                    self:walk(sx, sy)
                    return
                elseif abs(sx) < 32 and self.canSkill1 then
                    self:stepBack()
                    return
                end

                if self.canOugis1 and not self.isControlled and target.def < 5000 and not self.isArmored then
                    self:changeSide(sx, sy)
                    self:attack('OUGIS1')
                elseif self.canGear03 then
                    self:useGear('Gear03')
                elseif self.canSkill1 and not self.isArmored and target.def < 5000 then
                    self:changeSide(sx, sy)
                    self:attack('SKILL1')
                else
                    self:changeSide(sx, sy)
                    self:attack('NAttack')
                end
                return
            end
        end
    end

    self:clearTarget()
    if self:notFindFlogHalf() then
        self:findTowerHalf()
    end

    target = self:target()
    if target then
        local sx, sy = self:distance()

        if abs(sx) > 32 or abs(sy) > 32 then
            self:walk(sx, sy)
            return
        end

        if self.isFreeState then
            if self.canSkill2 and not self.isArmored then
                self:changeSide(sx, sy)
                self:attack('SKILL2')
            else
                self:changeSide(sx, sy)
                self:attack('NAttack')
            end
        end
        return
    end

    self:checkHealingState()
end

-- setara  changeAction_SageNaruto()
local function changeAction_SageNaruto(self)
    self.def = self.def + 5000
    self.isArmored = true
    self.hasArmorBroken = true
    self.isOnlySkillLocked = true

    self:setAction('idle',    'spc4', 5,  true,  false)
    self:setAction('walk',    'spc2', 10, true,  false)
    self:setAction('nattack', 'spc3', 10, false, true)

    self.walkSpeed = 320
    self.originSpeed = 320

    self:lockOugisButtons()
end

-- setara  resumeAction_SageNaruto(dt)
local function resumeAction_SageNaruto(self, dt)
    self:setAction('idle')
    self:setAction('walk')
    self:setAction('nattack')

    self.isOnlySkillLocked = false

    self:unlockOugisButtons()

    self.walkSpeed = 224
    self.originSpeed = 224

    self:resetDefenseValue(5000)
    self.isArmored = false
    self.hasArmorBroken = false

    if self.state ~= 'DEAD' then
        self.state = 'WALK'
        self:setAction('knockdown', 'spc5', 10, false, true)
        self:knockDown()
        self:setAction('knockdown')
    elseif self.tempAttackValue1 > 0 then
        self.nAttackValue = self.tempAttackValue1
        self.tempAttackValue1 = 0
    end
    self:baseResumeAction(dt)
end

-- setara  createClone_SageNaruto(cloneTime)
local function createClone_SageNaruto(self, cloneTime)
    local clone = self:createClone(self.name, true)
    if clone then
        self:setupSpcSkill1Clone(clone)
    end
    return clone
end

-- ============================================================================
-- Rikudo Naruto
-- ============================================================================

-- setara  void Naruto::perform_RikudoNaruto()
local function perform_RikudoNaruto(self)
    self:clearTarget()
    self:findHeroHalf()

    self:tryBuyGear('Gear03', 'Gear07', 'Gear02')

    if self:needBackToTowerToRestoreHP() or self:needBackToDefendTower() then
        return
    end

    local target = self:target()
    if target and target.isNotFlog then
        local sx, sy = self:distance()

        if self.isFreeState then
            if self.canOugis2 and not self.isControlled and self.isOugis2Game and not self.isArmored then
                self:changeSide(sx, sy)
                self:attack('OUGIS2')
                return

            elseif self.canSkill2 and not self.isArmored then
                self:changeSide(sx, sy)
                self:attack('SKILL2')
                return

            elseif self.canSkill3 and not self.isArmored and target.def < 5000 then
                self:changeSide(sx, sy)
                self:attack('SKILL3')
                return

            elseif self.canSkill1 and not self.isArmored and target.def < 5000 then
                self:changeSide(sx, sy)
                self:attack('SKILL1')
                return

            elseif self.canGear03 then
                self:useGear('Gear03')
                -- (C++: tanpa return -> lanjut ke pencarian flog/tower)

            elseif self.enemyCombatPoint > self.friendCombatPoint
                and abs(self.enemyCombatPoint - self.friendCombatPoint) > 3000
                and not self.isHealing and not self.isArmored and not self.isControlled then
                if abs(sx) < 160 then
                    self:stepBack2()
                else
                    self:idle()
                end
                return

            elseif abs(sx) < 128 then
                if abs(sx) > 32 or abs(sy) > 32 then
                    self:walk(sx, sy)
                    return
                end
                if self.canOugis1 and not self.isControlled and target.def < 5000 and not self.isArmored then
                    self:changeSide(sx, sy)
                    self:attack('OUGIS1')
                else
                    self:changeSide(sx, sy)
                    self:attack('NAttack')
                end
                return
            end
        end
    end

    self:clearTarget()
    if self:notFindFlogHalf() then
        self:findTowerHalf()
    end

    target = self:target()
    if target then
        local sx, sy = self:distance()

        if abs(sx) > 32 or abs(sy) > 32 then
            self:walk(sx, sy)
            return
        end

        if self.isFreeState then
            if self.canSkill1 and not self.isArmored and target.isFlog then
                self:changeSide(sx, sy)
                self:attack('SKILL1')
            elseif self.canSkill2 and not self.isArmored then
                self:changeSide(sx, sy)
                self:attack('SKILL2')
            else
                self:changeSide(sx, sy)
                self:attack('NAttack')
            end
        end
        return
    end

    self:checkHealingState()
end

-- setara  changeAction_RikudoNaruto()
local function changeAction_RikudoNaruto(self)
    self.def = self.def + 5000
    self.isArmored = true
    self.hasArmorBroken = true
    self.isOnlySkillLocked = true

    self:setAction('idle',    'spc4', 5,  true,  false)
    self:setAction('walk',    'spc2', 10, true,  false)
    self:setAction('nattack', 'spc3', 10, false, true)

    if self.name == 'RikudoNaruto' then
        self.tempAttackValue1 = self.nAttackValue
        self.nAttackValue = 560
    end

    self.walkSpeed = 320
    self.originSpeed = 320

    self:lockOugisButtons()
end

-- setara  resumeAction_RikudoNaruto(dt)
local function resumeAction_RikudoNaruto(self, dt)
    self:setAction('idle')
    self:setAction('walk')
    self:setAction('nattack')

    self.isOnlySkillLocked = false

    self:unlockOugisButtons()

    self.walkSpeed = 224
    self.originSpeed = 224

    self:resetDefenseValue(5000)
    self.isArmored = false
    self.hasArmorBroken = false

    if self.state ~= 'DEAD' then
        self.state = 'WALK'
        self:idle()
    end

    if self.tempAttackValue1 > 0 then
        self.nAttackValue = self.tempAttackValue1
        self.tempAttackValue1 = 0
    end
    self:baseResumeAction(dt)
end

-- setara  createClone_RikudoNaruto(cloneTime)
local function createClone_RikudoNaruto(self, cloneTime)
    if cloneTime == 9 then
        local kurama = self:createSummon('Kurama')   -- createCloneHero<Kurama>(SummonEnum::Kurama)
        if kurama then
            kurama.def = 5000
            kurama.isArmored = true
            kurama.walkSpeed = 320
            kurama.originSpeed = 320
            kurama.hasArmorBroken = true
        end
        return kurama
    end

    local clone = self:createClone(self.name, true)
    if clone then
        self:setupSpcSkill1Clone(clone)
    end
    return clone
end

-- ============================================================================
-- Dispatch per nama (setara match_char_exp3 di C++)
-- ============================================================================

local function pick(self, naruto, sage, rikudo)
    local n = self.name
    if n == 'Naruto' then return naruto
    elseif n == 'SageNaruto' then return sage
    elseif n == 'RikudoNaruto' then return rikudo end
    return nil
end

function Naruto.perform(self)
    -- setAIHandler hanya dipasang untuk 3 nama; selain itu Hero memakai perform() = bentuk Naruto
    local fn = pick(self, perform_Naruto, perform_SageNaruto, perform_RikudoNaruto) or perform_Naruto
    return fn(self)
end

function Naruto.changeAction(self)
    local fn = pick(self, changeAction_Naruto, changeAction_SageNaruto, changeAction_RikudoNaruto)
    if fn then fn(self) end
end

function Naruto.resumeAction(self, dt)
    local fn = pick(self, resumeAction_Naruto, resumeAction_SageNaruto, resumeAction_RikudoNaruto)
    if fn then fn(self, dt) end
end

function Naruto.createClone(self, cloneTime)
    local fn = pick(self, createClone_Naruto, createClone_SageNaruto, createClone_RikudoNaruto)
    if fn then return fn(self, cloneTime) end
    return nil
end

return Naruto
