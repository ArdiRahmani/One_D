--
-- Assets: aset tambahan (plist sprite + suara) yang ikut di-load / di-unload bersama karakter
--
-- Format:  [nama_karakter] = { jenis = 'NamaAset' atau { 'A', 'B' }, ... }
--
-- Contoh: Hashirama = { ninja = 'SageHashirama', skill = 'SageHashirama', ogg = 'SageHashirama' } artinya
--   LoadLayer::perloadCharIMG("Hashirama") (saat loading, hero acak, dll) menjalankan:
--     addSprites(mk_ninja_plist("SageHashirama"));     -- Unit/Ninja/SageHashirama/SageHashirama.plist
--     addSprites(mk_skill_plist("SageHashirama"));     -- Unit/Ninja/SageHashirama/SageHashirama_Skill.plist
--     KTools::prepareFileOGG(HeroEnum::SageHashirama);
--   LoadLayer::unloadCharIMG(unit Hashirama) menjalankan kebalikannya:
--     removeSprites(...) x2;  KTools::prepareFileOGG(HeroEnum::SageHashirama, true);
--   setara dengan  if (name == HeroEnum::Hashirama) { ... }  di C++.
--
-- Jenis yang tersedia (nilai string atau array string):
--   ninja      Unit/Ninja/<n>/<n>.plist
--   skill      Unit/Ninja/<n>/<n>_Skill.plist
--   kugutsu    Unit/Kugutsu/<n>/<n>.plist
--   kuchiyose  Unit/Kuchiyose/<n>/<n>.plist
--   guardian   Unit/Guardian/<n>/<n>.plist
--   flog       Unit/Flog/<n>.plist
--   plist      path plist mentah relatif Resources, mis. 'Unit/Ninja/KaHashiramaa/KaHashiramaa_Skill2.plist'
--   ogg        KTools::prepareFileOGG(<n>) saat load, prepareFileOGG(<n>, true) saat unload
--
-- Aturan:
--   * Dijalankan SETELAH percabangan bawaan di C++ (Jiraiya, Naruto, dst tetap jalan).
--     Jangan daftarkan aset yang sudah di-load oleh percabangan bawaan, nanti dobel.
--   * Plist yang filenya tidak ada dilewati dan ditulis ke log, tidak crash.
--   * Saat unload, hanya berlaku untuk unit karakter asli (clone / summon / guardian
--     dilewati, sama seperti unloadCharIMG bawaan).
--
-- Bisa juga dipanggil dari modul Lua lain sebelum match mulai:
--   Assets.set('Hashirama', { ninja = 'SageHashirama', skill = 'SageHashirama', ogg = 'SageHashirama' })
--   Assets.add('Hashirama', 'kuchiyose', 'Toad')
--   Assets.remove('Hashirama')
--
local AssetsHook = {}

local list = {
    Hashirama = {
        ninja = 'SageHashirama',
        skill = 'SageHashirama',
        ogg = 'SageHashirama',
    },
    Nagato = {
        ninja = 'AnimalPath',
        ninja = 'AsuraPath',
        ninja = 'NarakaPath',
    },
    Guy = {
        ninja = 'Guy8Gates',
        skill = 'Guy8Gates',
        ogg = 'Guy8Gates',
    },
}

if not Assets then
    log('[Assets] tabel native Assets tidak ditemukan, aset karakter tidak terkirim ke C++')
else
    Assets.setAll(list)
end

return AssetsHook
