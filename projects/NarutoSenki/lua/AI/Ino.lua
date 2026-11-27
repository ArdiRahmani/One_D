--
-- AI Ino versi Lua (port 1:1 dari Classes/Core/Shinobi/Ino.hpp)
--
-- Dipakai oleh unit yang di-bind di lua/Hook/LuaAI.lua, mis. Mei = 'Ino'.
-- Unit "Ino" sendiri tetap memakai Ino.hpp selama tidak didaftarkan di tabel itu.
--
-- Daftar API `self` / `target`: lua/AI/README.md
--
-- CATATAN PORT:
--   * Di Lua angka 0 itu TRUTHY. `if (_skillChangeBuffValue)` di C++ -> `self.skillChangeBuffValue ~= 0`.
--   * Ino::dead() hanya memanggil CharacterBase::dead() (sisanya komentar/TODO), jadi tidak ada hook dead.
--   * Loop possession (Shintenshin) di resumeAction / setActionResume dikerjakan C++ lewat
--     self:releaseControlledHeroes(idleAfter) -- lengkap dengan GUARD "controller harus bernama Ino"
--     supaya korban stun Kabuto tidak ikut tertukar tim.
--       - resumeAction    -> releaseControlledHeroes(true)   (urutan Ino::resumeAction)
--       - setActionResume -> releaseControlledHeroes(false)  (urutan Ino::setActionResume)
--   * GUARD itu membandingkan nama controller dengan "Ino": untuk unit lain selain "Ino" possession
--     tidak akan dilepas oleh script ini kecuali C++ disesuaikan.
--   * Butuh binding: canItem1, setItem, cancelResumeAction, releaseControlledHeroes (LuaHero.hpp).
--
local abs = math.abs

local Ino = {}

-- setara  void Ino::perform()
function Ino.perform(self)
    self:clearTarget()                    -- _mainTarget = nullptr
    self:findHeroHalf()

    if self.skillChangeBuffValue ~= 0 then
        return
    end

    self:tryUseGear6()
    self:tryBuyGear('Gear06', 'Gear05', 'Gear01')

    if self:needBackToTowerToRestoreHP() or self:needBackToDefendTower() then
        return
    end

    if self.maxHp - self.hp >= 3000 and self.coin >= 50
        and not self.isHealing and self.canItem1 and self.isArmored then
        self:setItem('Item1')
    end

    local target = self:target()
    if target and target.isNotFlog then
        local sx, sy = self:distance()    -- getDistanceToTarget()

        if self.isFreeState then
            if self.canOugis2 and not self.isControlled and self.isOugis2Game then
                if abs(sx) > 96 or abs(sy) > 16 then
                    self:walk(sx, sy)
                    return
                else
                    self:changeSide(sx, sy)
                    self:attack('OUGIS2')
                end
                return

            elseif target.def < 5000 and (self.canSkill3 or self.canSkill2) then
                if abs(sx) > 96 or abs(sy) > 16 then
                    self:walk(sx, sy)
                    return
                end

                if self.canSkill2 then
                    self:changeSide(sx, sy)
                    self:attack('SKILL2')
                elseif self.canSkill3 then
                    self:changeSide(sx, sy)
                    self:attack('SKILL3')
                end
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
                if abs(sx) > 32 or abs(sy) > 32 then
                    self:walk(sx, sy)
                    return
                end

                if self.canOugis1 and not self.isControlled and target.def < 5000 then
                    self:changeSide(sx, sy)
                    self:attack('OUGIS1')
                elseif self.canSkill1 then
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
            if target.isFlog and self.canSkill1 then
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

-- setara  void Ino::resumeAction(float dt)   (berakhirnya Shintenshin)
function Ino.resumeAction(self, dt)
    if not self.isArmored then
        return
    end

    self:releaseControlledHeroes(true)

    self.isArmored = false
    self:baseResumeAction(dt)                      -- CharacterBase::resumeAction(dt)
end

-- setara  void Ino::setActionResume()
function Ino.setActionResume(self)
    self:cancelResumeAction()                      -- unschedule(schedule_selector(Ino::resumeAction))

    self:releaseControlledHeroes(false)

    self.isArmored = false
end

return Ino
