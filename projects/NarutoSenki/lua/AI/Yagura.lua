--
-- AI Yagura: pakai semua logika Tsunade.lua, tapi createClone-nya membuat klon "YClone".
--
--   LuaAI.set('Yagura', 'Yagura')     -- lua/Hook/LuaAI.lua
--   CloneAI.set('YClone', 'Slug')     -- lua/Hook/CloneAI.lua  (YClone memakai AI Slug.hpp)
--
-- Klon dibuat saat C++ memanggil setClone() (event <e type="setClone"> di XML aksi Yagura).
-- `cloneTime` = nomor klon ke-berapa (0, 1, 2, ...), sama seperti contoh Kankuro.
--
local Base = require 'AI.Tsunade'

local Yagura = {}
for k, v in pairs(Base) do Yagura[k] = v end

function Yagura.createClone(self, cloneTime)
    -- setara  createCloneHero<Slug>("YClone")  -> Role::Clone
    local clone = self:createClone('AnimalPath')
    if not clone then return nil end

    clone.isArmored = true                 -- clone->_isArmored = true;
    -- self:addMonster(clone)              -- _monsterArray.push_back(clone)  (gaya Kankuro)
    -- self:lockSkill5Button()             -- lockSkill5Button();
    -- self.skillChangeBuffValue = 100     -- _skillChangeBuffValue = 100;   (gaya Nagato)
    return clone
end

return Yagura
