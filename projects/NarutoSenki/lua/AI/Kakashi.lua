--
-- AI Kakashi versi Lua (port 1:1 dari Classes/Core/Shinobi/Kakashi.hpp)
--
-- Dipakai oleh unit yang di-bind di lua/Hook/LuaAI.lua, mis. Madara = 'Kakashi'.
-- Unit "Kakashi" sendiri tetap memakai Kakashi.hpp selama tidak didaftarkan di tabel itu.
--
-- Daftar API `self` / `target`: lua/AI/README.md
--
-- CATATAN PORT:
--   * Nama frame ikon HUD ("Kakashi_skill1_1.png") di-hardcode seperti C++. Kalau script ini dipakai
--     unit lain, ganti prefix lewat PREFIX di bawah.
--   * Butuh binding tambahan di LuaHero.hpp: revealEnemies, setSkillButtonFrame, setPosition,
--     setAnchorPoint, isFlipped, isSticking, sAttackValue1, spcAttackValue1.
--
local abs = math.abs

local PREFIX = 'Kakashi'

local Kakashi = {}

-- setara  void Kakashi::perform()
function Kakashi.perform(self)
    self:clearTarget()                    -- _mainTarget = nullptr
    self:findHeroHalf()

    self:tryUseGear6()
    self:tryBuyGear('Gear06', 'Gear05', 'Gear02')

    if self:needBackToTowerToRestoreHP() or self:needBackToDefendTower() then
        return
    end

    local target = self:target()
    if target and target.isNotFlog then
        local sx, sy = self:distance()    -- getDistanceToTarget()

        if self.isFreeState then
            if self.canSkill3 then
                self:changeSide(sx, sy)
                self:attack('SKILL3')
                return

            elseif self.canSkill2 and target.def < 5000 then
                self:changeSide(sx, sy)
                self:attack('SKILL2')
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
                if abs(sx) > 64 or abs(sy) > 32 then
                    self:walk(sx, sy)
                    return
                end

                if self.canOugis2 and not self.isControlled and self.isOugis2Game
                    and target.def < 5000 and not target.isArmored
                    and target.state ~= 'KNOCKDOWN' and not target.isSticking then
                    self:changeSide(sx, sy)
                    self:attack('OUGIS2')
                elseif self.canOugis1 and not self.isControlled and target.def < 5000 then
                    self:changeSide(sx, sy)
                    self:attack('OUGIS1')
                elseif self.canSkill1 and target.def < 5000 then
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
            if self.canSkill3 and target.isFlog then
                self:changeSide(sx, sy)
                self:attack('SKILL3')
            elseif self.canSkill1 and target.isFlog then
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

-- setara  void Kakashi::changeAction()   (aktif saat Kakashi masuk mode Sharingan)
function Kakashi.changeAction(self)
    self:setAction('skill1', 'spc1', 10, false, true)
    self:setAction('skill2', 'spc2', 10, false, true)
    self:setAction('idle',   'spc3', 5,  true,  false)
    self.tempAttackValue1 = self.sAttackValue1     -- setTempAttackValue1(getSAttackValue1())
    self.sAttackValue1 = self.spcAttackValue1      -- setSAttackValue1(getSpcAttackValue1())

    -- musuh yang sedang invisible dalam jangkauan jadi terlihat lagi
    self:revealEnemies()

    if self.isPlayer then
        self:setSkillButtonFrame(1, PREFIX .. '_skill1_1.png')
        self:setSkillButtonFrame(2, PREFIX .. '_skill2_1.png')
    end
end

-- setara  void Kakashi::resumeAction(float dt)
function Kakashi.resumeAction(self, dt)
    -- tanpa array = kembalikan ke animasi bawaan slot itu
    self:setAction('idle')
    self:setAction('skill1')
    self:setAction('skill2')

    self.sAttackValue1 = self.tempAttackValue1

    if self.isPlayer then
        self:setSkillButtonFrame(1, PREFIX .. '_skill1.png')
        self:setSkillButtonFrame(2, PREFIX .. '_skill2.png')
    end
    self:baseResumeAction(dt)                      -- CharacterBase::resumeAction(dt)
end

-- setara  Hero *Kakashi::createClone(int cloneTime)
function Kakashi.createClone(self, cloneTime)
    local clone = self:createSummon('DogWall')
    if clone then
        clone:setPosition(self.x + (self.isFlipped and -56 or 56), self.y)
        clone:setAnchorPoint(0.5, 0.1)
        clone.isArmored = true
    end
    return clone
end

return Kakashi
