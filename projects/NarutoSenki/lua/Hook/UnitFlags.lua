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
--   healGroup   CharacterBase::healBuff (GroupBuff/GroupHeal/hBuff) bekerja seperti KARIN:
--               heal HP semua rekan hero dalam jangkauan.
--   healChakra  ... seperti CHIYO: tambah chakra semua rekan hero dalam jangkauan (kecuali dirinya).
--   healSelf    ... seperti SAKURA / TSUNADE: heal diri sendiri.
--   healFlog    ... seperti SLUG (Katsuyu): heal pasukan (flog) satu tim dalam jangkauan.
--   Contoh:  Fukai = 'healChakra'   -- Fukai (clone/pemakai) pakai GroupBuff gaya Chiyo
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
    -- Madara = { 'holdBuff', 'lockWalk' },
    -- GroupBuff = (Chiyo, Fukai)  ->
    -- Fukai = 'healChakra',
    -- GroupHeal = (Karin, X)      ->  X = 'healGroup'
    -- Heal self = (Tsunade, X)    ->  X = 'healSelf'
    -- Heal flog = (Slug, X)       ->  X = 'healFlog'
}

if not UnitFlags then
    log('[UnitFlags] tabel native UnitFlags tidak ditemukan, flag tidak terkirim ke C++')
else
    UnitFlags.setAll(list)
end

return UnitFlagsHook
