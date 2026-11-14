--
-- LuaAI: hero memakai AI yang ditulis di Lua (lua/AI/<Nama>.lua)
--
-- Format:  [nama_class_AI] = tabel_handler
--
-- Contoh: Tsunade = require 'AI.Tsunade' artinya setiap Tsunade yang dibuat Provider::create()
-- memakai class LuaTsunade (Classes/Core/LuaHero/LuaTsunade.hpp), yang meneruskan
--     perform / changeAction / resumeAction / createClone
-- ke fungsi Lua di lua/AI/Tsunade.lua. Class C++ asli (Tsunade.hpp) tidak berubah.
--
-- Kembali ke C++ asli:   LuaAI.set('Tsunade', nil)   (sebelum match mulai)
--
-- Beda dengan CloneAI (lua/Hook/CloneAI.lua):
--   * CloneAI -> "unit X memakai class C++ milik Y"
--   * LuaAI   -> "class C++ Y dijalankan dari Lua"   (nama = nama class AI hasil resolve CloneAI)
--
-- Catatan: AI yang menggantikan class C++ hanya aktif untuk hero yang ada subclass-nya di
-- Classes/Core/LuaHero/. Saat ini: Tsunade.
--
local AIHook = {}

local B = require 'binding.hero'

local list = {
    Tsunade = 'AI.Tsunade',
}

if not LuaAI then
    log('[LuaAI] tabel native LuaAI tidak ditemukan, AI Lua tidak terkirim ke C++')
elseif not B.check() then
    log('[LuaAI] binding native belum lengkap, AI Lua tidak diaktifkan (C++ asli dipakai)')
else
    for ai, module in pairs(list) do
        local ok, handlers = pcall(require, module)
        if ok and type(handlers) == 'table' then
            LuaAI.set(ai, handlers)
        else
            log('[LuaAI] gagal memuat %s: %s (C++ asli dipakai)', module, tostring(handlers))
        end
    end
    log('[LuaAI] %d AI Lua terdaftar', LuaAI.count())
end

return AIHook
