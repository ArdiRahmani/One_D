--
-- AI Sakura versi Lua (port 1:1 dari Classes/Core/Shinobi/Sakura.hpp)
--
-- Dipakai oleh unit yang di-bind di lua/Hook/LuaAI.lua, mis. Mei = 'Sakura'.
-- Unit "Sakura" sendiri tetap memakai Sakura.hpp selama tidak didaftarkan di tabel itu.
--
-- Daftar API `self` / `target`: lua/AI/README.md
--
-- CATATAN PORT:
--   * Di Lua angka 0 itu TRUTHY. `!_skillChangeBuffValue` di C++ ditulis `self.skillChangeBuffValue == 0`.
--   * Sakura::resumeAction di C++ TIDAK memanggil CharacterBase::resumeAction, jadi di sini juga tidak
--     memanggil baseResumeAction (sengaja sama persis).
--   * Sakura.hpp meng-override setActionResume() -> hook `setActionResume`.
--   * Butuh binding: battleCondiction, isSticking (LuaHero.hpp).
--   * Efek heal Sakura (CharacterBase::healBuff) di C++ dicek per NAMA unit (Tsunade/Sakura), jadi
--     untuk unit lain selain "Sakura" efek itu tidak ikut aktif.
--
local abs = math.abs

local Sakura = {}

-- setara  void Sakura::perform()
function Sakura.perform(self)
    self:clearTarget()                    -- _mainTarget = nullptr
    self:findHeroHalf()

    self:tryBuyGear('Gear00', 'Gear04', 'Gear08')

    if self:needBackToTowerToRestoreHP() then
        return
    end

    local target = self:target()
    if target and (self.battleCondiction >= 0 or self.canOugis1 or self.canOugis2) then
        local sx, sy = self:distance()    -- getDistanceToTarget()

        if self.isFreeState then
            if self.canOugis2 and not self.isControlled and self.isOugis2Game
                and self.skillChangeBuffValue == 0 and target.def < 5000 and not target.isArmored
                and target.state ~= 'KNOCKDOWN' and not target.isSticking then
                if abs(sx) > 48 or abs(sy) > 16 then
                    if self.canGear00 then
                        self:useGear('Gear00')
                    end
                    self:walk(sx, sy)
                    return
                end

                self:changeSide(sx, sy)
                self:attack('OUGIS2')
                return

            elseif self.canOugis1 and not self.isControlled and target.def < 5000
                and self.skillChangeBuffValue == 0 then
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

            elseif self.enemyCombatPoint > self.friendCombatPoint
                and abs(self.enemyCombatPoint - self.friendCombatPoint) > 3000
                and not self.isHealing and not self.isControlled then
                if abs(sx) < 160 then
                    self:stepBack2()
                else
                    self:idle()
                end
                return

            elseif self.hpPercent < 0.9 and self.canSkill1 then
                self:attack('SKILL1')
                return

            elseif abs(sx) < 128 then
                if (abs(sx) > 64 or abs(sy) > 32) and (not self.canSkill2 or target.def < 5000) then
                    self:walk(sx, sy)
                    return
                elseif abs(sx) > 32 or abs(sy) > 32 then
                    self:walk(sx, sy)
                    return
                end

                if self.canSkill2 and self.skillChangeBuffValue == 0 and target.def < 5000 then
                    self:changeSide(sx, sy)
                    self:attack('SKILL2')
                elseif self.canSkill3 and self.skillChangeBuffValue == 0 and self.hpPercent > 0.5 then
                    self:changeSide(sx, sy)
                    self:attack('SKILL3')
                else
                    self:changeSide(sx, sy)
                    self:attack('NAttack')
                end
                return
            end
        end
    end

    if self.battleCondiction >= 0 then
        self:clearTarget()
        if self:notFindFlogHalf() then
            self:findTowerHalf()
        end
    else
        self:clearTarget()
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
            if self.canSkill1 and self.hpPercent < 0.9 then
                self:changeSide(sx, sy)
                self:attack('SKILL1')
            elseif target.isFlog and self.canSkill2 then
                self:changeSide(sx, sy)
                self:attack('SKILL2')
            elseif target.isTower and self.canSkill3 and self.skillChangeBuffValue == 0 then
                self:changeSide(sx, sy)
                self:attack('SKILL3')
            else
                self:changeSide(sx, sy)
                self:attack('NAttack')
            end
        end
        return
    end

    self:checkHealingState()
end

-- setara  void Sakura::changeAction()
function Sakura.changeAction(self)
    self.nAttackValue = self.nAttackValue + 200

    self.nAttackRangeX = 16
    self.nAttackRangeY = 48

    self:setAction('nattack', 'spc1', 10, false, true)
end

-- setara  void Sakura::resumeAction(float dt)   (tanpa CharacterBase::resumeAction, sama seperti C++)
function Sakura.resumeAction(self, dt)
    self.nAttackValue = self.nAttackValue - 200
    self.nAttackRangeX = 16
    self.nAttackRangeY = 48
    self:setAction('nattack')                      -- tanpa array = animasi bawaan
end

-- setara  void Sakura::setActionResume()
function Sakura.setActionResume(self)
    self.nAttackValue = self.nAttackValue - 200
    self.nAttackRangeX = 16
    self.nAttackRangeY = 48
    self.skillChangeBuffValue = 0
end

return Sakura
