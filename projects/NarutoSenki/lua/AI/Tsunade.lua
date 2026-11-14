--
-- AI Tsunade (implementasi Lua)
--
-- Port 1:1 dari Classes/Core/Shinobi/Tsunade.hpp (file C++ itu TIDAK diubah dan tetap menjadi
-- fallback engine). Seluruh pemanggilan ke engine lewat binding native (lihat binding/hero.lua
-- dan Classes/Systems/HeroBinding.hpp) -- file ini tidak mengandung kode binding.
--
-- C++ -> Lua : LuaTsunade (Classes/Core/LuaHero/LuaTsunade.hpp) memanggil keempat fungsi di bawah
--              lewat tabel `LuaAI` (didaftarkan di Hook/AI.lua):
--     perform(self)                -- Tsunade::perform()
--     changeAction(self)           -- Tsunade::changeAction()
--     resumeAction(self, dt)       -- Tsunade::resumeAction(float)
--     createClone(self, cloneTime) -- Tsunade::createClone(int)  (return Hero | nil)
-- Lua -> C++ : semua `self:xxx()` adalah method native pada userdata CharacterBase.
--
local B = require 'binding.hero'

local abs = math.abs
local Tsunade = {}

-- Ganti member C++ `string _originNAttackType`
local originNAttackType = B.newInstanceStore()

function Tsunade.perform(self)
    self:clearMainTarget()
    self:findHeroHalf()

    self:tryBuyGear(GearType.Gear03, GearType.Gear02, GearType.Gear04)

    if self:needBackToTowerToRestoreHP() or self:needBackToDefendTower() then
        return
    end

    if self:hasMainTarget() and self:mainTargetIsNotFlog() then
        local sx, sy = self:getDistanceToTarget()

        if self:isFreeState() then
            local defLow = self:getMainTargetDEF() < 5000
            local armored = self:getFlag('_isArmored')

            if self:getFlag('_isCanOugis2') and not self:getFlag('_isControlled')
                and self:getGameFlag('_isOugis2Game') and defLow and not armored then
                if abs(sx) > 64 or abs(sy) > 16 then
                    B.walkToward(self, sx, sy)
                    return
                else
                    self:changeSide(sx, sy)
                    self:attack(ABType.OUGIS2)
                end
                return
            elseif self:getFlag('_isCanSkill3') and defLow and not armored then
                self:changeSide(sx, sy)
                self:attack(ABType.SKILL3)
                return
            elseif self:getNum('enemyCombatPoint') > self:getNum('friendCombatPoint')
                and abs(self:getNum('enemyCombatPoint') - self:getNum('friendCombatPoint')) > 3000
                and not self:getFlag('_isHealing') and not self:getFlag('_isControlled') and not armored then
                if abs(sx) < 160 then
                    self:stepBack2()
                else
                    self:idle()
                end
                return
            elseif abs(sx) < 128 then
                if abs(sx) > 64 or abs(sy) > 32 then
                    B.walkToward(self, sx, sy)
                    return
                elseif abs(sx) > 32
                    or (abs(sy) > 32 and not self:getFlag('_isCanSkill2') and not armored
                        and not self:getFlag('_isCanOugis1')) then
                    B.walkToward(self, sx, sy)
                    return
                end
                if self:getFlag('_isCanGear03') then
                    self:useGear(GearType.Gear03)
                end
                -- useGear bisa mengubah stat: baca ulang, sama seperti C++ yang membaca member langsung
                defLow = self:getMainTargetDEF() < 5000
                armored = self:getFlag('_isArmored')
                if self:getFlag('_isCanOugis1') and not self:getFlag('_isControlled') and defLow and not armored then
                    self:changeSide(sx, sy)
                    self:attack(ABType.OUGIS1)
                elseif self:getFlag('_isCanSkill2') and not armored then
                    self:changeSide(sx, sy)
                    self:attack(ABType.SKILL2)
                    return
                elseif self:getFlag('_isCanSkill1') and not armored then
                    self:changeSide(sx, sy)
                    self:attack(ABType.SKILL1)
                else
                    self:changeSide(sx, sy)
                    self:attack(ABType.NAttack)
                end

                return
            end
        end
    end

    self:clearMainTarget()
    if self:notFindFlogHalf() then
        self:findTowerHalf()
    end

    if self:hasMainTarget() then
        local sx, sy = self:getDistanceToTarget()

        if abs(sx) > 32 or abs(sy) > 32 then
            B.walkToward(self, sx, sy)
            return
        end

        if self:isFreeState() then
            if self:getFlag('_isCanGear03') then
                self:useGear(GearType.Gear03)
            end
            if self:mainTargetIsFlog() and self:getFlag('_isCanSkill1')
                and not self:getFlag('_isArmored') and self:getFlag('isBaseDanger') then
                self:changeSide(sx, sy)
                self:attack(ABType.SKILL1)
            else
                self:changeSide(sx, sy)
                self:attack(ABType.NAttack)
            end
        end

        return
    end

    self:checkHealingState()
end

function Tsunade.changeAction(self)
    self:setNAttackValue(self:getNAttackValue() + 250)
    self:setFlag('_isOnlySkillLocked', true)
    self:setNum('_nAttackRangeX', 32)
    self:setNum('_nAttackRangeY', 48)
    originNAttackType.set(self, self:getNAttackType())
    self:setNAttackType(self:getSpcAttack3Type())

    self:setFlag('_isArmored', true)
    self:setNum('_healBuffValue', 3000)
    self:startHealBuff(1)

    self:setAnimation('idle', 'skillSPC1', 5, true, false)
    self:setAnimation('walk', 'skillSPC2', 10, true, false)
    self:setAnimation('nattack', 'skillSPC3', 10, false, true)

    self:lockOugisButtons()
end

function Tsunade.resumeAction(self, dt)
    self:setNAttackValue(self:getNAttackValue() - 250)
    self:setNum('_nAttackRangeX', 16)
    self:setNum('_nAttackRangeY', 48)
    self:setNAttackType(originNAttackType.get(self) or '')
    originNAttackType.set(self, nil)

    self:setFlag('_isOnlySkillLocked', false)

    self:unlockOugisButtons()
    self:setFlag('_isArmored', false)

    self:setAnimation('nattack', 'nattack', 10, false, true)
    self:setAnimation('idle', 'idle', 5, true, false)
    self:setAnimation('walk', 'walk', 10, true, false)

    if self:getState() ~= State.DEAD then
        self:setState(State.WALK)
        self:idle()
    end
    self:callBaseResumeAction(dt) -- CharacterBase::resumeAction(dt)
end

function Tsunade.createClone(self, cloneTime)
    local clone = self:createSummonHero('Slug', 'Slug') -- createSummonHero<Slug>(SummonEnum::Slug)
    if clone then
        clone:setFlag('_isArmored', true)
    end
    return clone
end

return Tsunade
