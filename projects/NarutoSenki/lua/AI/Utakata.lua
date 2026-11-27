local abs = math.abs

local Utakata = {}

-- setara  void Utakata::perform()
function Utakata.perform(self)
self:clearTarget()                    -- _mainTarget = nullptr
    self:findHeroHalf()

    self:tryBuyGear('Gear00', 'Gear02', 'Gear08')

    if self:needBackToTowerToRestoreHP() or self:needBackToDefendTower() then
        return
    end

    local target = self:target()
    if target and target.isNotFlog then
        local sx, sy = self:distance()    -- getDistanceToTarget()

        if self.isFreeState then
            if self.canOugis2 and not self.isControlled and self.isOugis2Game and not self.isArmored
                and target.hp < 10000 and not target.canGear06 then
                if abs(sx) < 32 then
                    self:stepBack()
                    return
                elseif abs(sx) > 96 or abs(sy) > 32 then
                    if self.canGear00 then
                        self:useGear('Gear00')
                    end
                    self:walk(sx, sy)
                    return
                end

                self:changeSide(sx, sy)
                self:attack('OUGIS2')
                return

            elseif self.canOugis1 and not self.isControlled and target.def < 5000 and not self.isArmored then
                if abs(sx) > 48 or abs(sy) > 16 then
                    if self.canGear00 then
                        self:useGear('Gear00')
                    end
                    self:walk(sx, sy)
                    return
                end
                self:changeSide(sx, sy)
                self:attack('OUGIS1')
                return
            
            elseif self.canSkill1 and target.def < 5000 and not self.isArmored then      -- !_skillChangeBuffValue
                self:changeSide(sx, sy)
                self:attack('SKILL1')
                return

            elseif self.canSkill3 and target.def < 5000 and not self.isArmored
                and self.skillChangeBuffValue == 0 then      -- !_skillChangeBuffValue
                self:changeSide(sx, sy)
                self:attack('SKILL3')
                return

            elseif self.enemyCombatPoint > self.friendCombatPoint
                and abs(self.enemyCombatPoint - self.friendCombatPoint) > 3000
                and not self.isHealing and not self.isControlled then
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
                end

                if self.canSkill2 and target.def < 5000 and not self.isArmored then
                    if abs(sx) < 32 then
                        self:stepBack()
                        return
                    end
                    self:changeSide(sx, sy)
                    self:attack('SKILL2')

                else
                    if abs(sx) > 32 or abs(sy) > 32 then
                        self:walk(sx, sy)
                        return
                    else
                        self:changeSide(sx, sy)
                        self:attack('NAttack')
                    end
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
            if target.isFlog and self.canSkill1 and not self.isArmored then
                self:changeSide(sx, sy)
                self:attack('SKILL1')
            elseif self.canSkill2 and target.def < 5000 and not self.isArmored
                and target.isFlog and self.isBaseDanger then
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

function Utakata.changeAction(self)

    self.isArmored = true
    self.nAttackRangeX = 16
    self.nAttackRangeY = 48
    self.tempAttackValue1 = self.nAttackValue      -- setTempAttackValue1(getNAttackValue())
    self.nAttackValue = 560

    self.isOnlySkillLocked = true

    self:setAction('idle',    'spc1', 5,  true,  false)
    self:setAction('walk',    'spc2', 10, true,  false)
    self:setAction('nattack', 'spc3', 10, false, true)

    self.walkSpeed = 320
    self.originSpeed = 320                         -- _originSpeed = 320
    self:lockOugisButtons()
end

-- setara  void Utakata::setActionResume()
function Utakata.setActionResume(self)

    self.isArmored = false
    self.isOnlySkillLocked = false
    if self.tempAttackValue1 > 0 then              -- hasTempAttackValue1()
        self.nAttackValue = self.tempAttackValue1
        self.tempAttackValue1 = 0
    end

    -- tanpa array = kembalikan ke animasi bawaan slot itu
    self:setAction('idle')
    self:setAction('walk')
    self:setAction('nattack')

    self:unlockOugisButtons()

    self.skillChangeBuffValue = 0

    self.walkSpeed = 224
    self.originSpeed = 224
end


return Utakata
