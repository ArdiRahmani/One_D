--
-- Buffs: alias tipe buff -> tipe buff bawaan (dikirim ke C++ lewat tabel `Buffs`)
--
-- Format:  [alias] = tipe_bawaan
--
-- Contoh: h2Buff = 'hBuff' artinya skill yang attackType-nya "h2Buff" diproses persis seperti "hBuff"
-- oleh CharacterBase::setBuff / setBuffEffect / removeBuffEffect (heal berkala + efek hBuff).
--
-- Tipe bawaan:
--   hBuff      heal diri/rekan berkala (jenisnya diatur flag healGroup/healChakra/healSelf/healFlog di UnitFlags)
--   sBuff      naikkan damage skill 1-3 selama _attackRangeY detik
--   rsBuff     seperti sBuff (tanpa efek visual)
--   hsBuff     seperti sBuff + efek hsBuff + membongkar musuh yang bersembunyi di dekat
--   dcBuff     seperti sBuff + efek dcBuff
--   cBuff      ganti mode serangan/aksi (changeAction), lalu resumeAction
--   tBuff, t2Buff   ganti aksi + efek tBuff / t2Buff, lalu resumeAction
--   gBuff      ganti aksi (changeAction2)
--   stBuff     stealth (transparan/menghilang) selama _attackRangeY detik
--   GroupHeal, GroupBuff   tambah _healBuffValue dan jalankan healBuff tiap detik
--
-- Efek visual bawaan (Effect.cpp): hBuff, sBuff, hsBuff, tBuff, t2Buff, dcBuff, jdBuff, bmBuff, dhBuff
-- (juga Kagura, Hupo, dll). Alias otomatis memakai efek tipe bawaannya.
--
-- Efek visual sendiri untuk alias: tabel `effects` di bawah. Animasi = frame "<prefix>01..NN"
-- yang sudah ada di plist yang termuat (sama seperti hBuff_Effect_01.. dst).
--   prefix  (wajib) awalan nama frame      frames  jumlah frame        fps   kecepatan (10)
--   loop    true/false (default true)      anchorX/anchorY  (0.5, 0)   offsetX/offsetY  geser posisi
--
-- Bisa juga dipanggil dari modul Lua lain sebelum match mulai:
--   Buffs.set('h2Buff', 'hBuff')
--   Buffs.set('h2Buff', nil)          -- hapus alias
--   Buffs.get('h2Buff')               -- 'hBuff'
--   Buffs.resolve('h3Buff')           -- ikuti rantai alias
--
local BuffsHook = {}

local list = {
    -- h2Buff = 'hBuff',
    hamBuff = 'sBuff',
    -- dc2Buff = 'dcBuff',
    -- t3Buff = 'tBuff',
}

local effects = {
     hamBuff = { prefix = 'hamBuff_Effect_', frames = 10, fps = 10, loop = true },
}

if not Buffs then
    log('[Buffs] tabel native Buffs tidak ditemukan, alias buff tidak terkirim ke C++')
else
    Buffs.setAll(list)
    Buffs.effects(effects)
end

return BuffsHook
