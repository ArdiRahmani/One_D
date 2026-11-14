--
-- CloneAI: karakter memakai AI/class karakter lain
--
-- Format:  [nama_unit] = nama_AI
--
-- Contoh: Madara = 'Asuma' artinya saat game membuat unit "Madara", C++ memakai
-- class Asuma (skill + logika AI Asuma), setara dengan:
--     is("Asuma") ptr = new Asuma();      -- tapi untuk nama "Madara"
-- Nama unit tetap "Madara", jadi sprite / data / HP bar tetap dibaca dari
-- Resources/Unit/Madara... (folder & data unit Madara harus tersedia).
--
-- Aturan:
--   * nama_AI harus nama class native di Provider.hpp (Asuma, Kakashi, Naruto, ...).
--     Kalau tidak dikenal, game jatuh ke DefaultAI (diam di tempat) dan menulis log.
--   * Tidak berantai: Madara = 'Asuma' dan Asuma = 'Kakashi' -> Madara tetap Asuma.
--   * Agar Madara ikut hero acak / lawan COM, tambahkan juga 'Madara' di
--     lua/Hook/Heroes.lua.
--
-- Bisa juga dipanggil dari modul Lua lain sebelum match mulai:
--   CloneAI.set('Madara', 'Asuma')
--   CloneAI.set('Madara', nil)      -- hapus alias
--   CloneAI.resolve('Madara')       -- 'Asuma'
--
local CloneAIHook = {}

local list = {
    Temari = 'Pakura', 
}

if not CloneAI then
    log('[CloneAI] tabel native CloneAI tidak ditemukan, alias AI tidak terkirim ke C++')
else
    CloneAI.setAll(list)
end

return CloneAIHook
