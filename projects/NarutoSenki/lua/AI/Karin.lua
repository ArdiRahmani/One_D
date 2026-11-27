--
-- AI Karin versi Lua (port 1:1 dari Classes/Core/Shinobi/Karin.hpp)
--
-- Dipakai oleh unit yang di-bind di lua/Hook/LuaAI.lua, mis. Mei = 'Karin'.
-- Unit "Karin" sendiri tetap memakai Karin.hpp selama tidak didaftarkan di tabel itu.
--
-- Daftar API `self` / `target`: lua/AI/README.md
--
-- CATATAN PORT:
--   * `!_buffStartTime` (float) di C++ -> `self.buffStartTime == 0`  (0 itu TRUTHY di Lua).
--   * Di bagian kedua perform() C++ TIDAK me-reset _mainTarget (beda dengan hero lain), jadi di sini
--     juga tidak ada clearTarget() sebelum notFindFlogHalf().
--   * onSetTrap: jebakan "Kusuri" dibuat lewat self:createTrap(tipe, offsetX, offsetY)
--     (hook `onSetTrap` ditambahkan di LuaHero.hpp).
--   * Butuh binding: buffStartTime, isSticking, createTrap, hook onSetTrap.
--
local abs = math.abs

local Karin = {}

-- setara  void Karin::perform()
function Karin.perform(self)
    self:clearTarget()                    -- _mainTarget = nullptr
    self:findHeroHalf()

    self:tryUseGear6()
    self:tryBuyGear('Gear06', 'Gear01', 'Gear07')

    if self:needBackToTowerToRestoreHP() or self:needBackToDefendTower() then
        return
    end

    local target = self:target()
    if target and target.isNotFlog then
        local sx, sy = self:distance()    -- getDistanceToTarget()

        if self.isFreeState then
            if self.canOugis2 and not self.isControlled and self.isOugis2Game
                and target.def < 5000 and not target.isArmored
                and target.state ~= 'KNOCKDOWN' and not target.isSticking then
                if abs(sx) > 96 or abs(sy) > 16 then
                    self:walk(sx, sy)
                    return
                end
                self:changeSide(sx, sy)
                self:attack('OUGIS2')
                return

            elseif self.hpPercent < 0.9 and self.canOugis1 and not self.isControlled
                and self.buffStartTime == 0 and not self.isHealing then
                self:changeSide(sx, sy)
                self:attack('OUGIS1')
                return

            elseif self.canSkill3 and target.def < 5000 then
                if abs(sx) > 128 or abs(sy) > 16 then
                    self:walk(sx, sy)
                    return
                end

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
                if self.canSkill2 then
                    self:changeSide(sx, sy)
                    self:attack('SKILL2')
                elseif self.canSkill1 then
                    self:changeSide(sx, sy)
                    self:attack('SKILL1')
                else
                    if abs(sx) > 32 or abs(sy) > 32 then
                        self:walk(sx, sy)
                        return
                    end
                    self:changeSide(sx, sy)
                    self:attack('NAttack')
                end
                return
            end
        end
    end

    -- (C++: tidak ada _mainTarget = nullptr di sini)
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
            if self.canSkill1 and target.isFlog then
                self:changeSide(sx, sy)
                self:attack('SKILL1')
            elseif self.canSkill2 and target.isFlog and self.isBaseDanger then
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

-- setara  void Karin::onSetTrap(const string &trapType)
function Karin.onSetTrap(self, trapType)
    if trapType == 'Kusuri' then
        local flip = self.isFlipped
        for i = 0, 2 do
            if i == 0 then
                self:createTrap(trapType, flip and -24 or 24, -24)
            elseif i == 1 then
                self:createTrap(trapType, flip and 24 or -24, -24)
            else
                self:createTrap(trapType, 0, 24)
            end
        end
    end
end

return Karin
