--
-- AI Tsunade versi Lua (port 1:1 dari Classes/Core/Shinobi/Tsunade.hpp)
--
-- Dipakai oleh unit yang di-bind di lua/Hook/LuaAI.lua, mis. Yagura = 'Tsunade'.
-- Unit "Tsunade" sendiri tetap memakai Tsunade.hpp.
--
-- Daftar API `self` / `target`: lua/AI/README.md
--
local abs = math.abs

local Tsunade = {}

-- setara  void Tsunade::perform()
function Tsunade.perform(self)
    self:clearTarget()                    -- _mainTarget = nullptr
    self:findHeroHalf()

    self:tryBuyGear('Gear03', 'Gear02', 'Gear04')

    if self:needBackToTowerToRestoreHP() or self:needBackToDefendTower() then
        return
    end

    local target = self:target()
    if target and target.isNotFlog then
        local sx, sy = self:distance()    -- getDistanceToTarget()

        if self.isFreeState then
            if self.canOugis2 and not self.isControlled and self.isOugis2Game
                and target.def < 5000 and not self.isArmored then
                if abs(sx) > 64 or abs(sy) > 16 then
                    self:walk(sx, sy)
                    return
                else
                    self:changeSide(sx, sy)
                    self:attack('OUGIS2')
                end
                return
            
            elseif self.enemyCombatPoint > self.friendCombatPoint
                and abs(self.enemyCombatPoint - self.friendCombatPoint) > 3000
                and not self.isHealing and not self.isControlled and not self.isArmored then
                if abs(sx) < 160 then
                    self:stepBack2()
                else
                    self:idle()
                end
                return

            elseif abs(sx) < 128 then
                if abs(sx) > 64 or abs(sy) > 32 then
                    self:walk(sx, sy)
                    return
                elseif abs(sx) > 32
                    or (abs(sy) > 32 and not self.canSkill2 and not self.isArmored and not self.canOugis1) then
                    self:walk(sx, sy)
                    return
                end

                if self.canGear03 then
                    self:useGear('Gear03')
                end

                if self.canOugis1 and not self.isControlled and target.def < 5000 and not self.isArmored then
                    self:changeSide(sx, sy)
                    self:attack('OUGIS1')
                elseif self.canSkill2 and not self.isArmored then
                    self:changeSide(sx, sy)
                    self:attack('SKILL2')
                    return
                elseif self.canSkill3 and target.def < 5000 and not self.isArmored then
                self:changeSide(sx, sy)
                self:attack('SKILL3')
                return
                elseif self.canSkill1 and not self.isArmored then
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
            if self.canGear03 then
                self:useGear('Gear03')
            end
            if target.isFlog and self.canSkill1 and not self.isArmored and self.isBaseDanger then
                self:changeSide(sx, sy)
                self:attack('SKILL1')
            else
                self:changeSide(sx, sy)
                self:attack('NAttack')
            end
        end
        return
    end

    self:checkHealingState()
end

-- setara  void Tsunade::changeAction()   (aktif saat Tsunade masuk mode Byakugou / Seal)
function Tsunade.changeAction(self)
    self.nAttackValue = self.nAttackValue + 250
    self.isOnlySkillLocked = true
    self.nAttackRangeX = 32
    self.nAttackRangeY = 48
    self.originNAttackType = self.nAttackType      -- setara  string _originNAttackType (private C++)
    self.nAttackType = self.spcAttackType3

    self.isArmored = true
    self:startHealBuff(3000, 1)                    -- _healBuffValue = 3000; schedule(healBuff, 1)

    self:setAction('idle',    'spc1', 5,  true,  false)
    self:setAction('walk',    'spc2', 10, true,  false)
    self:setAction('nattack', 'spc3', 10, false, true)

    self:lockOugisButtons()
end

-- setara  void Tsunade::resumeAction(float dt)
function Tsunade.resumeAction(self, dt)
    self.nAttackValue = self.nAttackValue - 250
    self.nAttackRangeX = 16
    self.nAttackRangeY = 48
    self.nAttackType = self.originNAttackType or self.nAttackType

    self.isOnlySkillLocked = false

    self:unlockOugisButtons()
    self.isArmored = false

    -- tanpa array = kembalikan ke animasi bawaan slot itu
    self:setAction('nattack')
    self:setAction('idle')
    self:setAction('walk')

    if self.state ~= 'DEAD' then
        self.state = 'WALK'
        self:idle()
    end
    self:baseResumeAction(dt)                      -- CharacterBase::resumeAction(dt)
end

-- setara  Hero *Tsunade::createClone(int cloneTime)
function Tsunade.createClone(self, cloneTime)
    local clone = self:createSummon('Slug')
    if clone then
        clone.isArmored = true
    end
    return clone
end

return Tsunade
