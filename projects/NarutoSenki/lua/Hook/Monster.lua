--
-- CloneMon: monster/summon memakai perilaku (AI spawn) monster lain
--
-- Format:  [nama_monster] = nama_AI
--
-- Contoh: Chibaku = 'JibakuEX' artinya saat C++ memanggil
--     setMon("Chibaku")
-- Monster dibuat dengan nama "Chibaku" (sprite / data / resource tetap dibaca
-- dari Resources/Unit/Chibaku...), tetapi posisi spawn, mode attack / doAI,
-- dan z-order-nya memakai cabang "JibakuEX" di CharacterBase::setMon, setara dengan
--     else if (monName == "JibakuEX") { ... }   -- tapi untuk nama "Chibaku"
--
-- Aturan:
--   * nama_AI harus nama monster yang punya cabang khusus di setMon
--     (Jibaku, JibakuEX, Suiji, Kaiten, ...). Kalau tidak dikenal, monster
--     memakai perilaku bawaan (masuk _monsterArray + doAI), tidak error.
--   * Tidak berantai: Chibaku = 'JibakuEX' dan JibakuEX = 'Jibaku' -> Chibaku tetap JibakuEX.
--   * Berlaku juga untuk setMon yang dipicu dari plist aksi (key "setMon"),
--     karena semuanya lewat CharacterBase::setMon.
--
-- Bisa juga dipanggil dari modul Lua lain sebelum match mulai:
--   CloneMon.set('Chibaku', 'JibakuEX')
--   CloneMon.set('Chibaku', nil)      -- hapus alias
--   CloneMon.resolve('Chibaku')       -- 'JibakuEX'
--
local MonsterHook = {}

local list = {
    FakeSageHashirama = 'JibakuEX',
    Jinpachi = 'Jibaku',
    Cyclone = 'JibakuEX',
    Typhoon = 'Jibaku',
    Tiger8Gates = 'Hasan',
    TigerStream = 'Jibaku',
    Trunk = 'SansyoGreen',
    
}

if not CloneMon then
    log('[CloneMon] tabel native CloneMon tidak ditemukan, alias AI monster tidak terkirim ke C++')
else
    CloneMon.setAll(list)
end

return MonsterHook
