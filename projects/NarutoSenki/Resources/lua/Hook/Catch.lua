--
-- CloneCatch: master peluru/summon diperlakukan seperti master lain pada hit "ct_hit" (tangkap)
--
-- Format:  [nama_master] = nama_master_acuan
--
-- Contoh: Guy = 'Itachi' artinya saat peluru/summon milik master "Guy" mengenai
-- target dengan hit "ct_hit", target diperlakukan seperti ditangkap oleh Itachi:
--     hardHurt(3000, ...)  lalu target ditarik ke posisi attacker (+2, -2)
-- Nama master tetap "Guy" di bagian lain game.
--
-- Cabang yang bisa dituju (blok Itachi/Chiyo di CharacterBase):
--   Itachi : hardHurt 3000
--   Chiyo  : hardHurt 2000
--
-- Aturan:
--   * Nama master acuan peka huruf besar/kecil dan harus persis 'Itachi' atau 'Chiyo'.
--     Nama lain tidak mengaktifkan cabang apa pun (target tidak ditangkap).
--   * Tidak berantai: Guy = 'Itachi' dan Itachi = 'Chiyo' -> Guy tetap Itachi.
--   * Hanya berlaku untuk blok Itachi/Chiyo. Cabang Shikamaru dan Nagato tidak terpengaruh.
--   * Master "Itachi" / "Chiyo" yang asli tetap bekerja seperti biasa.
--
-- Bisa juga dipanggil dari modul Lua lain sebelum match mulai:
--   CloneCatch.set('Guy', 'Itachi')
--   CloneCatch.set('Guy', nil)      -- hapus alias
--   CloneCatch.resolve('Guy')       -- 'Itachi'
--
local CatchHook = {}

local list = {
    Guy = 'Itachi',
}

if not CloneCatch then
    log('[CloneCatch] tabel native CloneCatch tidak ditemukan, alias tangkapan tidak terkirim ke C++')
else
    CloneCatch.setAll(list)
end

return CatchHook
