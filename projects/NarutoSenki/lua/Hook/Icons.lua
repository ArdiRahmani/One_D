--
-- Icons: binding gambar HERO _rp.png, _rpf.png, _CutIn.png (plus _avator, _small)
--
-- Aturannya sama dengan GameOver (half.png): templat nama, nama lengkap per unit, binding
-- ke hero lain, skala umum, dan skala per unit. Bedanya, gambar TETAP diambil dari plist
-- (SpriteFrameCache), bukan dari file, jadi yang diatur adalah NAMA FRAME di plist.
-- C++ membaca data ini lewat tabel native `Icons` (lihat Classes/Systems/IconSystem.hpp).
--
-- Jenis (tidak peka huruf besar/kecil)  ->  frame bawaan:
--   rp       {}_rp.png       ikon kecil laporan kill (pembunuh maupun korban)
--   rpf      {}_rpf.png      ikon besar laporan kill
--   CutIn    {}_CutIn.png    potret cut-in saat ougi
--   Avator   {}_avator.png   tombol avatar di HUD            (tanpa skala)
--   small    {}_small.png    ikon kecil layar pilih / hasil  (tanpa skala)
--
-- Tiap jenis punya lima bagian sendiri, jadi rp, rpf, dan CutIn bisa diatur terpisah:
--
--   frame         Templat nama frame. "{}" diganti nama hero sumber (unit sendiri atau hasil `bind`).
--                 Default '{}_rpf.png'  ->  Naruto_rpf.png
--
--   file          Nama frame LENGKAP per unit (mengalahkan `frame` dan `bind`).
--                 Format: [unit] = 'NamaFrame.png'
--                 Contoh: Madara = 'MadaraEdo_rpf.png'
--
--   bind          Unit yang meminjam gambar hero lain. Arahnya SAMA dengan GameOver.half:
--                 [unit] = 'hero_sumber'
--                 Contoh: Madara = 'Obito'  ->  Madara memakai Obito_rpf.png
--                 (bukan Madara_rpf.png). Kalau frame tujuan tidak ada di plist, game
--                 memakai frame unit sendiri dan menulis log, tidak crash.
--
--   defaultScale  Skala sprite UMUM untuk semua unit yang tidak punya entri di `scale`.
--                 Default 1.0 (ukuran asli). Hanya rp, rpf, dan CutIn.
--
--   scale         Skala sprite khusus per unit (setScale); mengalahkan defaultScale.
--                 Format: [unit] = angka > 0, mis. Chiyo = 0.8
--
-- Aturan:
--   * Khusus hero. Banner kill-streak (DoubleKill, KillingSpree, Unstoppable, Godlike,
--     TripeKill, Domination, Rampage, Holyshit, MonsterKill) dan Tower ditolak: file
--     KillingSpree_rpf.png dkk tidak pernah ikut terganti.
--   * SkillLayer: kalau <n>_full.png tidak ada di plist, gambar besar hero memakai
--     Img/Half/<n>_half.png (skala di HALF_AS_FULL_SCALE pada lua/ui/SkillLayer.lua, default 1.0).
--   * Satu langkah saja (tidak berantai). Unit yang tidak terdaftar memakai namanya sendiri.
--   * CutIn: kalau frame <n>_CutIn.png tidak ada di plist, game memakai Img/Half/<n>_half.png
--     (ikut pengaturan GameOver.lua: half / file / path). Skala CutIn tetap berlaku.
--   * Skala dipakai juga untuk lebar laporan kill, jadi susunan ikon tetap rapat.
--   * Isi bawaan di bawah = perilaku asli game (tanpa binding, skala 1.0).
--
-- Bisa juga dipanggil dari modul Lua lain sebelum match mulai:
--   Icons.setBind('rpf', 'Madara', 'Obito')       -- Madara memakai Obito_rpf.png
--   Icons.setBind('rpf', 'Madara', nil)           -- hapus binding
--   Icons.setFile('rp', 'Madara', 'MadaraEdo_rp.png')
--   Icons.setFrame('CutIn', '{}_CutIn.png')       -- ganti templat nama frame
--   Icons.setDefaultScale('rpf', 0.9)             -- semua rpf jadi 0.9 (kecuali yang ada di scale)
--   Icons.setScale('rpf', 'Chiyo', 0.8)           -- rpf Chiyo diskalakan 0.8
--   Icons.getScale('rpf', 'Chiyo')                -- 0.8
--   Icons.resolve('rpf', 'Madara')                -- 'Obito'
--   Icons.frame('rpf', 'Madara')                  -- 'Obito_rpf.png' (atau frame sendiri kalau tidak ada)
--
local IconsHook = {}

local config = {
    rp = {
        frame = '{}_rp.png',
        file = {
            -- Madara = 'MadaraEdo_rp.png',
        },
        bind = {
            -- Madara = 'Obito',
        },
        defaultScale = 1.0,
        scale = {
            -- Chiyo = 0.8,
        },
    },
    rpf = {
        frame = '{}_rpf.png',
        file = {},
        bind = {},
        defaultScale = 1.0,
        scale = {},
    },
    CutIn = {
        frame = '{}_CutIn.png',
        file = {},
        bind = {},
        defaultScale = 1.0,
        scale = {},
    },
    -- Avator dan small hanya mendukung frame / file / bind (tanpa skala)
    Avator = {
        frame = '{}_avator.png',
        file = {},
        bind = {},
    },
    small = {
        frame = '{}_small.png',
        file = {},
        bind = {},
    },
}

if not Icons then
    log('[Icons] tabel native Icons tidak ditemukan, binding gambar tidak terkirim ke C++')
else
    Icons.setAll(config)
end

return IconsHook
