--
-- AI Hinata versi Lua (port 1:1 dari Classes/Core/Shinobi/Hinata.hpp)
--
-- Dipakai oleh unit yang di-bind di lua/Hook/LuaAI.lua, mis. Mei = 'Hinata'.
-- Unit "Hinata" sendiri tetap memakai Hinata.hpp selama tidak didaftarkan di tabel itu.
--
-- Hinata.hpp tidak meng-override changeAction / resumeAction / createClone, jadi script ini
-- hanya berisi perform().
--
-- Daftar API `self` / `target`: lua/AI/README.md
--
-- CATATAN PORT (beda semantik C++ vs Lua):
--   * Di Lua angka 0 itu TRUTHY. `_skillUPBuffValue` (uint) di C++ bernilai true jika != 0,
--     jadi ditulis `self.skillUPBuffValue ~= 0`.
--   * Butuh binding tambahan di LuaHero.hpp: skillUPBuffValue, isSticking.
--
local abs = math.abs

local Hinata = {}

-- jalan mendekat target; return true kalau masih harus jalan (belum dalam jangkauan)
local function approach(self, sx, sy, rx, ry)
    if abs(sx) > rx or abs(sy) > ry then
        if self.canGear00 then
            self:useGear('Gear00')
        end
        self:walk(sx, sy)
        return true
    end
    return false
end

-- setara  void Hinata::perform()
function Hinata.perform(self)
    self:clearTarget()                    -- _mainTarget = nullptr
    self:findHeroHalf()

    self:tryBuyGear('Gear00', 'Gear01', 'Gear02')

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
                if approach(self, sx, sy, 48, 32) then return end
                self:changeSide(sx, sy)
                self:attack('OUGIS2')
                return

            elseif self.canOugis1 and not self.isControlled and target.def < 5000 then
                if approach(self, sx, sy, 48, 32) then return end
                self:changeSide(sx, sy)
                self:attack('OUGIS1')
                return

            elseif self.canSkill1 then
                self:changeSide(sx, sy)
                self:attack('SKILL1')
                return

            elseif (self.canSkill2 or self.canSkill3) and target.def < 5000
                and self.skillUPBuffValue ~= 0 then
                if approach(self, sx, sy, 48, 32) then return end
                if self.canSkill2 then
                    self:changeSide(sx, sy)
                    self:attack('SKILL2')
                    return
                end
                if self.canSkill3 then
                    self:changeSide(sx, sy)
                    self:attack('SKILL3')
                    return
                end
                -- (C++: jatuh ke bawah tanpa return -> lanjut ke pencarian flog/tower)

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
            if self.canSkill2 and target.isFlog and self.isBaseDanger then
                self:changeSide(sx, sy)
                self:attack('SKILL2')
            elseif self.canSkill3 and target.isFlog and self.isBaseDanger then
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

return Hinata
