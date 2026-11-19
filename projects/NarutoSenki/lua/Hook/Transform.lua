--
-- Transform: karakter berubah menjadi karakter lain saat setTransform dipicu
--
-- Format:  [nama_karakter] = nama_hasil_transformasi
--
-- Contoh: Hashirama = 'SageHashirama' artinya saat C++ menjalankan
--     CharacterBase::setTransform()   (key "setTransform" di plist aksi)
-- pada unit bernama "Hashirama", unit diganti lewat setID("SageHashirama", _role, _group),
-- setara dengan
--     if (getName() == HeroEnum::Hashirama) setID(HeroEnum::SageHashirama, _role, _group);
--
-- Aturan:
--   * Nama hasil harus punya resource/data unit sendiri (Resources/Unit/SageHashirama...),
--     sama seperti SageNaruto, RockLee, dll.
--   * Satu langkah saja. Untuk rantai (Naruto -> SageNaruto -> RikudoNaruto)
--     daftarkan tiap langkah: Naruto = 'SageNaruto', SageNaruto = 'RikudoNaruto'.
--   * Entri di sini dicek lebih dulu; kalau nama tidak terdaftar, C++ memakai
--     percabangan bawaan di setTransform (Naruto, Jiraiya, Lee, dst).
--
-- Bisa juga dipanggil dari modul Lua lain sebelum match mulai:
--   Transform.set('Hashirama', 'SageHashirama')
--   Transform.set('Hashirama', nil)       -- hapus transformasi
--   Transform.get('Hashirama')            -- 'SageHashirama'
--
local TransformHook = {}

local list = {
    Hashirama = 'SageHashirama',
    Guy = 'Guy8Gates',
    Zetsu = 'Zetsu2', 
    Obito = 'ObitoRikudo', 
}

if not Transform then
    log('[Transform] tabel native Transform tidak ditemukan, transformasi tidak terkirim ke C++')
else
    Transform.setAll(list)
end

return TransformHook
