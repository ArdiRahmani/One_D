--
-- LuaAI: karakter memakai AI yang DITULIS DI LUA (lua/AI/<Script>.lua)
--
-- Format:  [nama_unit] = nama_script
--
-- Contoh: Yagura = 'Tsunade' artinya saat game membuat unit "Yagura", C++ membuat LuaHero
-- yang menjalankan lua/AI/Tsunade.lua (perform / changeAction / resumeAction / createClone).
-- Unit "Tsunade" TIDAK ikut berubah: tetap memakai class C++ Tsunade (Tsunade.hpp),
-- selama 'Tsunade' tidak didaftarkan di tabel ini.
--
-- Nama unit tetap "Yagura", jadi sprite / data / HP bar tetap dibaca dari
-- Resources/Unit/Ninja/Yagura... (folder & data unit Yagura harus tersedia).
--
-- Beda dengan CloneAI (lua/Hook/CloneAI.lua):
--   * CloneAI -> memakai class C++ yang sudah ada  (Madara = 'Asuma')
--   * LuaAI   -> memakai script Lua                (Yagura = 'Tsunade'  => lua/AI/Tsunade.lua)
-- Kalau sebuah unit ada di keduanya, LuaAI menang. Kalau script gagal dimuat (file tidak ada /
-- error sintaks / tidak ada fungsi perform), game menulis log "[LuaAI] ..." lalu jatuh balik
-- ke CloneAI / class C++ biasa (atau DefaultAI kalau tidak ada).
--
-- Nama script = nama file di lua/AI (tanpa .lua), boleh dipakai banyak unit sekaligus:
--   Yagura = 'Tsunade', Mei = 'Tsunade'
--
-- Aturan:
--   * Agar Yagura ikut hero acak / lawan COM, tambahkan juga 'Yagura' di lua/Hook/Heroes.lua.
--   * Aset tambahan (mis. plist Slug punya Tsunade) didaftarkan di lua/Hook/Assets.lua:
--         Yagura = { kuchiyose = 'Slug' }
--   * LuaAI hanya mengganti AI + hook lifecycle (perform, changeAction, resumeAction,
--     createClone). Logika C++ yang membandingkan NAMA unit (mis. CharacterBase::healBuff yang
--     hanya menyembuhkan kalau nama == Tsunade / Sakura) tetap berlaku per nama unit.
--
-- Bisa juga dipanggil dari modul Lua lain sebelum match mulai:
--   LuaAI.set('Yagura', 'Tsunade')
--   LuaAI.set('Yagura', nil)       -- hapus binding
--   LuaAI.get('Yagura')            -- 'Tsunade'
--   LuaAI.define('Dummy', { perform = function(self) self:idle() end })   -- tanpa file
--   LuaAI.set('Mei', 'Dummy')
--   LuaAI.reload('Tsunade')        -- muat ulang script untuk unit yang dibuat sesudahnya
--
-- Daftar API `self` ada di lua/AI/README.md.
--
local LuaAIHook = {}

local list = {
    Yagura = 'Yagura',   
    -- Madara = 'Hashirama',   -- lua/AI/Hashirama.lua (port Hashirama.hpp, termasuk SageHashirama)
    TobiMasked = 'TobiMasked',   -- lua/AI/Yagura.lua (= Tsunade.lua + klon YClone)
}

if not LuaAI then
    log('[LuaAI] tabel native LuaAI tidak ditemukan, binding AI Lua tidak terkirim ke C++')
else
    LuaAI.setAll(list)
end

return LuaAIHook
