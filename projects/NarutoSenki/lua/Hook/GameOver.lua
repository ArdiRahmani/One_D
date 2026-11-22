--
-- GameOver: pengaturan layar hasil akhir (GameOver::listResult)
--
-- Pengganti daftar `getName() == ...` yang dulu hard-coded di GameOver.cpp.
-- C++ membaca data ini lewat tabel native `GameOver` (lihat Classes/Systems/GameOverSystem.hpp).
--
-- Tiga bagian:
--
--   flipX   Daftar unit yang gambar Img/Half/<n>_half.png-nya dibalik horizontal.
--           Format: array nama unit.
--           Setara  half->setFlipX(true)  untuk unit tersebut.
--
--   path    Templat path file half. "{}" diganti nama hero sumber (unit sendiri atau hasil `half`).
--           Default 'Img/Half/{}_half.png'  ->  Img/Half/Naruto_half.png
--           Ganti kalau folder / akhiran namanya berbeda, mis. 'Img/Half2/{}.png'.
--
--   file    Path file half LENGKAP per unit (mengalahkan `path` dan `half`).
--           Format: [unit] = 'path/relatif/Resources.png'
--           Contoh: Madara = 'Img/Half/Custom/madara_big.png'
--
--   half    Unit yang meminjam gambar half milik hero lain.
--           Format: [unit] = 'hero_sumber'
--           Contoh: SageJiraiya = 'Jiraiya'  ->  Img/Half/Jiraiya_half.png dipakai
--           (bukan Img/Half/SageJiraiya_half.png).
--           Kalau file tujuan tidak ada, game memakai gambar unit sendiri dan menulis log.
--           Kunci flipX tetap memakai nama UNIT (bukan nama hero sumber).
--
--   defaultScale  Skala half UMUM untuk semua unit yang tidak punya entri di `scale`.
--           Default 1.0. Ubah ke 0.8 untuk mengecilkan half semua hero sekaligus.
--
--   scale   Skala half khusus per unit (half->setScale); mengalahkan defaultScale.
--           Format: [unit] = angka > 0, mis. Chiyo = 1.0. Posisi tetap rata kanan.
--
--   record  Unit yang kemenangan & waktu terbaiknya dicatat di baris CharRecord hero lain.
--           Format: [unit] = 'nama_catatan'
--           Contoh: SageNaruto = 'Naruto'  ->  menang pakai SageNaruto menambah catatan Naruto.
--
-- Aturan:
--   * Satu langkah saja (tidak berantai). Unit yang tidak terdaftar memakai namanya sendiri.
--   * Data C++ lama sudah DIHAPUS dari GameOver.cpp: kalau file ini tidak dimuat, tidak ada
--     gambar yang di-flip dan semua unit dicatat dengan namanya sendiri.
--
-- Bisa juga dipanggil dari modul Lua lain sebelum match mulai:
--   GameOver.setFlipX('Madara')            -- tambah ke daftar flipX
--   GameOver.setFlipX('Madara', false)     -- keluarkan dari daftar flipX
--   GameOver.isFlipX('Konan')              -- true
--   GameOver.setPath('Img/Half/{}_half.png')            -- ganti templat path half
--   GameOver.setFile('Madara', 'Img/Half/Custom/madara_big.png')
--   GameOver.resolvePath('SageJiraiya')    -- 'Img/Half/Jiraiya_half.png' (kalau half di-bind)
--   GameOver.setDefaultScale(0.8)          -- semua half jadi 0.8 (kecuali yang ada di scale)
--   GameOver.setScale('Madara', 0.9)       -- half Madara diskalakan 0.9
--   GameOver.setHalf('Madara', 'Obito')    -- Madara memakai Img/Half/Obito_half.png
--   GameOver.setRecord('Madara', 'Obito')  -- kemenangan Madara dicatat di baris Obito
--   GameOver.resolveRecord('SageNaruto')   -- 'Naruto'
--
local GameOverHook = {}

local config = {
    flipX = {
        'Konan', 'Karin', 'Suigetsu', 'Hidan', 'Tobirama', 'Tsunade', 'Kankuro',
        'SageJiraiya', 'Minato', 'Tobi', 'Lee', 'RockLee', 'Hinata', 'Asuma',
        'Chiyo', 'Kisame',
    },

    -- templat path file half, "{}" = nama hero sumber
    path = 'Img/Half/{}_half.png',

    -- [unit] = path file half lengkap (opsional)
    file = {
        -- Madara = 'Img/Half/Custom/madara_big.png',
    },

    -- [unit] = hero sumber gambar half
    half = {
        -- SageJiraiya = 'Jiraiya',
    },

    -- skala half umum, dipakai semua unit yang tidak ada di `scale` di bawah
    defaultScale = 1.0,

    -- [unit] = skala khusus half->setScale (mengalahkan defaultScale)
    scale = {
        -- Chiyo = 0.8,
    },

    -- [unit] = nama baris CharRecord
    record = {
        SageNaruto     = 'Naruto',
        RikudoNaruto   = 'Naruto',
        SageJiraiya    = 'Jiraiya',
        ImmortalSasuke = 'Sasuke',
        RockLee        = 'Lee',
        Nagato         = 'Pain',
        Guy8Gates      = 'Guy',
    },
}

if not GameOver then
    log('[GameOver] tabel native GameOver tidak ditemukan, pengaturan layar hasil tidak terkirim ke C++')
else
    GameOver.setAll(config)
end

return GameOverHook
