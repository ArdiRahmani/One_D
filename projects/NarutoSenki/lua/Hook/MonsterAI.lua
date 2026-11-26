--
-- CloneMonAI: monster/summon memakai AI per-tick (Monster::setAI) monster lain
--
-- Format:  [nama_monster] = nama_AI
--
-- Contoh: Shakuton = 'Rocket' artinya monster "Shakuton" berjalan dengan logika
-- Monster::setAI milik "Rocket" (mengejar target, stepOn saat tidak ada target, dst).
-- Nama unit tetap "Shakuton" (sprite / data / resource tetap dari Resources/Unit/Mon/Shakuton...).
-- Setara dengan:
--     if (charName == "Rocket") { ... }   -- tapi untuk monster bernama "Shakuton"
--
-- Beda dengan CloneMon (lua/Hook/Monster.lua):
--   * CloneMon    -> perilaku SPAWN di CharacterBase::setMon (posisi, mode attack, z-order)
--   * CloneMonAI  -> perilaku per-tick di Monster::setAI (cara bergerak / menyerang)
-- Keduanya bisa dipakai bersamaan untuk monster yang sama.
--
-- AI yang tersedia di Monster::setAI:
--   Kage        : menunggu musuh dekat lalu memanggil KageHand
--   Mouse       : mengejar hero saja
--   Spider, ClayBird : mengejar hero lalu tower; stepOn saat tidak ada target
--   FutonSRK, FutonSRK2 : mengejar target lalu dealloc saat kena player/COM
--   Traps       : diam, menyerang saat target dalam jangkauan
--   Mine        : diam, meledak saat hero/COM mendekat
--   Dogs, Yominuma, Tsukuyomi : tidak berjalan, hanya menyerang di jarak dekat
--   Rocket, Hayashi, Mokujin  : AI standar + stepOn saat tidak ada target
--   (nama lain) : AI standar (cari hero/flog, jalan, serang, idle)
--
-- Aturan:
--   * nama_AI bebas; nama yang tidak punya cabang khusus memakai AI standar, tidak error.
--   * Tidak berantai: Shakuton = 'Rocket' dan Rocket = 'Mine' -> Shakuton tetap Rocket.
--   * Alias berlaku untuk seluruh isi Monster::setAI, jadi cocok dipasangkan dengan
--     monster yang memang dijalankan via doAI().
--
-- Bisa juga dipanggil dari modul Lua lain sebelum match mulai:
--   CloneMonAI.set('Shakuton', 'Rocket')
--   CloneMonAI.set('Shakuton', nil)      -- hapus alias
--   CloneMonAI.resolve('Shakuton')       -- 'Rocket'
--
local MonsterAIHook = {}

local list = {
    Hayashi = 'Rocket',
    Mokujin = 'Rocket',
    FakeShino = 'ClayBird', 
    PaperBom = 'Mine',
}

if not CloneMonAI then
    log('[CloneMonAI] tabel native CloneMonAI tidak ditemukan, alias AI monster tidak terkirim ke C++')
else
    CloneMonAI.setAll(list)
end

return MonsterAIHook
