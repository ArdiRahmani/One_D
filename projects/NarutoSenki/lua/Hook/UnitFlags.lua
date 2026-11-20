--
-- UnitFlags: flag perilaku per karakter yang di C++ ditulis sebagai daftar nama (hard-coded)
--
-- Format:  [nama_unit] = 'flag'   atau   [nama_unit] = { 'flag1', 'flag2' }
--
-- Flag:
--   holdBuff  CharacterBase::setBuff tidak menjadwalkan resumeAction otomatis setelah buffStayTime.
--             Mode berakhir lewat event setActionResume di XML aksi (hook `setActionResume`
--             di lua/AI/Hashirama.lua). Setara masuk daftar
--             `getName() != Hashirama/Hiruzen/Suigetsu/Jugo/Kisame/...` di CharacterBase::setBuff.
--   lockWalk  CharacterBase::walk tidak menggerakkan AI (bukan player) yang sedang NATTACK saat
--             _isOnlySkillLocked. Setara `getName() == Hashirama/Hiruzen/Suigetsu/Jugo/Kisame`
--             di CharacterBase::walk.
--
-- Ini TAMBAHAN di atas nama yang sudah ada di C++ (Hashirama, Hiruzen, dst tidak perlu didaftarkan).
-- Gunanya untuk hero baru, mis. Madara yang memakai AI Hashirama.lua:
--     Madara = { 'holdBuff', 'lockWalk' },
--
-- Bisa juga dipanggil dari modul Lua lain sebelum match mulai:
--   UnitFlags.set('Madara', 'holdBuff', 'lockWalk')
--   UnitFlags.has('Madara', 'lockWalk')     -- true
--   UnitFlags.set('Madara', nil)            -- hapus semua flag unit
--
local UnitFlagsHook = {}

local list = {
--    Utakata = { 'holdBuff', 'lockWalk' },
}

if not UnitFlags then
    log('[UnitFlags] tabel native UnitFlags tidak ditemukan, flag tidak terkirim ke C++')
else
    UnitFlags.setAll(list)
end

return UnitFlagsHook
