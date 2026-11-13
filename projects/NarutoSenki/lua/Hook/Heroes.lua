--
-- Hero roster
--
-- Satu-satunya tempat mendaftarkan hero (pengganti kHeroList di UnitDefines.h).
-- C++ membaca daftar ini lewat tabel native `HeroRegistry`
-- (lihat Classes/Systems/HeroSystem.hpp) untuk:
--   * hero acak / lawan COM di semua mode
--   * pembuatan baris catatan (CharRecord) di database
--
-- Menambah hero: cukup tambahkan namanya di daftar `roster` di bawah
-- (atau panggil Heroes.add('Nama') dari modul Lua lain), tanpa menyentuh C++.
--
-- Catatan: nama harus sama dengan nama unit di Resources/Unit/... Kalau belum
-- ada class C++ khusus untuk hero itu, game memakai DefaultAI (diam di tempat).
--
local Heroes = {}

-- Urutan = urutan di daftar acak COM
local roster = {
    'Asuma', 'Chiyo', 'Choji', 'Deidara', 'Gaara', 'Guy', 'Hashirama', 'Hidan',
    'Hinata', 'Hiruzen', 'Itachi', 'Jiraiya', 'Jugo', 'Kakashi', 'Kakuzu',
    'Kankuro', 'Karin', 'Kiba', 'Kimimaro', 'Kisame', 'Konan', 'Lee',
    'Minato', 'Naruto', 'Neji', 'Ino', 'Orochimaru', 'Pain', 'Sai', 'Sakura',
    'Sasuke', 'Shikamaru', 'Shino', 'Suigetsu', 'Tenten', 'Tobi', 'Tobirama',
    'Tsunade'
}

local function hasNative()
    if not HeroRegistry then
        log('[Heroes] HeroRegistry (native) tidak ditemukan, ' ..
                'daftar hero tidak terkirim ke C++')
        return false
    end
    return true
end

-- Kirim seluruh daftar ke C++
local function sync()
    if hasNative() then HeroRegistry.set(roster) end
end

function Heroes.has(name)
    for _, v in ipairs(roster) do if v == name then return true end end
    return false
end

function Heroes.count() return #roster end

-- Salinan daftar (aman dimodifikasi pemanggil)
function Heroes.all()
    local copy = {}
    for i, v in ipairs(roster) do copy[i] = v end
    return copy
end

-- Tambah hero baru. Return true kalau berhasil ditambahkan.
function Heroes.add(name)
    if type(name) ~= 'string' or name == '' then
        log('[Heroes] nama hero tidak valid: %s', tostring(name))
        return false
    end
    if Heroes.has(name) then return false end

    roster[#roster + 1] = name
    if hasNative() then HeroRegistry.add(name) end
    return true
end

-- Hapus hero dari daftar. Return true kalau ada yang dihapus.
function Heroes.remove(name)
    for i, v in ipairs(roster) do
        if v == name then
            table.remove(roster, i)
            sync()
            return true
        end
    end
    return false
end

-- Peringatan kalau ada hero di roster yang tidak punya tombol di layar pilih
-- (ns.CharactersLayout). Hero seperti itu tetap bisa muncul sebagai COM acak.
function Heroes.checkLayout(layout)
    local inLayout = {}
    for _, v in ipairs(layout or {}) do inLayout[v] = true end

    local missing = {}
    for _, v in ipairs(roster) do
        if not inLayout[v] then missing[#missing + 1] = v end
    end
    if #missing > 0 then
        log('[Heroes] tidak ada di layar pilih: %s', table.concat(missing, ', '))
    end
end

sync()
log('[Heroes] %d hero terdaftar', #roster)

return Heroes
