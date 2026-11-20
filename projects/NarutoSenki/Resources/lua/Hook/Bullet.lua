--
-- CloneBullet: peluru memakai cabang spawn (CharacterBase::setBullet) peluru lain
--
-- Format:  [nama_peluru] = nama_cabang
--
-- Contoh: Sanpu = 'TentenSRK' artinya saat karakter memanggil setBullet("Sanpu"),
-- peluru "Sanpu" muncul dengan posisi, skala, dan gerakan milik "TentenSRK"
-- (scale 0.8, offset 32 px di depan, setEaseIn(224, 2.0), attack NAttack).
-- Nama peluru tetap "Sanpu" (sprite / data / resource tetap dari Resources/Unit/... Sanpu).
--
-- Cabang yang dikenal CharacterBase::setBullet:
--   PaperSrk, PaperSpear   : lurus pelan (setMove 192), offset depan 32 / 68
--   HugeSRK                : peluru besar + 2 peluru tambahan (setBulletGroup), setEaseIn 224/5.0
--   FlyKnife, TentenSRK    : pisau terbang, setEaseIn 224/2.0
--   HiraishinKunai         : kunai hiraishin, setEaseIn 196/2.0
--   Shintenshin            : jurus Ino, anchor bawah, setEaseIn 196/2.0
--   (nama lain)            : peluru standar (setMove 192, durasi 2.0)
--
-- Aturan:
--   * Nama cabang TIDAK peka huruf besar/kecil: 'TentenSrk' sama dengan 'TentenSRK'.
--   * Nama cabang yang tidak dikenal memakai cabang peluru standar, tidak error.
--   * Tidak berantai: Sanpu = 'TentenSRK' dan TentenSRK = 'HugeSRK' -> Sanpu tetap TentenSRK.
--   * Hanya mengubah cabang di setBullet; logika lain yang memeriksa nama peluru
--     (mis. di Bullet.hpp) tetap memakai nama aslinya.
--
-- Bisa juga dipanggil dari modul Lua lain sebelum match mulai:
--   CloneBullet.set('Sanpu', 'TentenSRK')
--   CloneBullet.set('Sanpu', nil)      -- hapus alias
--   CloneBullet.resolve('Sanpu')       -- 'TentenSRK'
--
local BulletHook = {}

local list = {
    Sanpu = 'TentenSRK',
}

if not CloneBullet then
    log('[CloneBullet] tabel native CloneBullet tidak ditemukan, alias peluru tidak terkirim ke C++')
else
    CloneBullet.setAll(list)
end

return BulletHook
