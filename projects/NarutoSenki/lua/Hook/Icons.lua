--
-- Icons: binding gambar unit (_rp, _rpf, _CutIn, _avator, _small) ke gambar hero lain
--
-- Format:  jenis = { [hero_sumber] = unit_yang_memakai }   (unit bisa string atau array string)
--
-- Contoh: rpf = { Guy = 'Guy8Gates' } artinya saat unit Guy8Gates membunuh / terbunuh,
-- laporan kill memakai frame
--     Guy_rpf.png   (bukan Guy8Gates_rpf.png)
-- Arahnya sama dengan Assets / Transform: kiri = hero asal, kanan = bentuk yang meminjam gambarnya.
-- Tiap jenis punya tabel sendiri, jadi rp dan rpf bisa di-bind terpisah.
--
-- Jenis (tidak peka huruf besar/kecil)  ->  frame yang dicari:
--   rp       <n>_rp.png       ikon kecil laporan kill (pembunuh maupun korban)
--   rpf      <n>_rpf.png      ikon besar laporan kill
--   CutIn    <n>_CutIn.png    potret cut-in saat ougi
--   Avator   <n>_avator.png   tombol avatar di HUD (nama frame asli di game huruf kecil)
--   small    <n>_small.png    ikon kecil di layar pilih hero dan layar hasil akhir
--
-- Aturan:
--   * Frame tujuan (mis. Guy_rpf.png) harus sudah ada di plist yang ter-load. Kalau tidak
--     ada, game memakai frame unit sendiri (Guy8Gates_rpf.png) dan menulis log, tidak crash.
--   * Tidak berantai: kalau A di-bind ke B dan B di-bind ke C, A tetap memakai gambar B.
--   * Unit yang tidak terdaftar tetap memakai gambar namanya sendiri.
--
-- Bisa juga dipanggil dari modul Lua lain sebelum match mulai:
--   Icons.add('rpf', 'Guy', 'Guy8Gates')    -- Guy8Gates_rpf.png -> Guy_rpf.png
--   Icons.remove('rpf', 'Guy8Gates')
--   Icons.resolve('rpf', 'Guy8Gates')       -- 'Guy'
--   Icons.frame('rpf', 'Guy8Gates')         -- 'Guy_rpf.png'
--
local IconsHook = {}

local list = {
    rp = {
        Chiyo = 'Guy',
    },
    rpf = {
        Chiyo = 'SageHashirama',
        Tobi = 'TobiMasked',
    },
    CutIn = {
        -- Guy = 'Guy8Gates',
    },
    Avator = {
        HashiramaET = 'SageHashirama',
    },
    small = {
        -- Hashirama = 'SageHashirama',
    },
}

if not Icons then
    log('[Icons] tabel native Icons tidak ditemukan, binding gambar tidak terkirim ke C++')
else
    Icons.setAll(list)
end

return IconsHook
