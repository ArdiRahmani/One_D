#pragma once
#include <algorithm>
#include <cstring>
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * Daftar hero yang bisa dipakai game (pengganti kHeroList / kHeroNum lama).
 *
 * Isinya TIDAK di-hardcode di C++. Daftar diisi dari Lua lewat
 * lua/Hook/Heroes.lua (memanggil HeroRegistry.add / HeroRegistry.set,
 * lihat Systems/HeroSystem.hpp). Untuk menambah hero cukup edit file Lua itu.
 *
 * Catatan: pakai std::deque supaya pointer const char* yang sudah dibagikan
 * tidak invalid saat hero baru ditambahkan.
 */
namespace HeroRegistry
{
	inline std::deque<std::string> &store()
	{
		static std::deque<std::string> list;
		return list;
	}

	inline size_t count()
	{
		return store().size();
	}

	inline bool empty()
	{
		return store().empty();
	}

	inline const char *at(size_t index)
	{
		return index < store().size() ? store()[index].c_str() : nullptr;
	}

	inline bool contains(const char *name)
	{
		if (!name)
			return false;
		const auto &list = store();
		return std::find(list.begin(), list.end(), name) != list.end();
	}

	// Return false kalau nama kosong atau sudah terdaftar.
	inline bool add(const char *name)
	{
		if (!name || !*name || contains(name))
			return false;
		store().emplace_back(name);
		return true;
	}

	// Hanya dipanggil saat Lua memuat ulang daftar (sebelum game dimulai).
	inline void clear()
	{
		store().clear();
	}
} // namespace HeroRegistry

/**
 * Alias AI karakter (CloneAI): "karakter X memakai class/AI karakter Y".
 *
 * Contoh: AIRegistry::set("Madara", "Asuma") -> Provider::create("Madara", ...)
 * membuat `new Asuma()`, tapi ID/nama unit tetap "Madara" (resource, sprite, dan
 * data unit tetap dibaca dari nama "Madara").
 *
 * Diisi dari Lua lewat tabel global `CloneAI` (lihat Systems/ProviderSystem.hpp
 * dan lua/Hook/CloneAI.lua). Resolve hanya satu tingkat (tidak berantai), jadi
 * tidak mungkin terjadi loop: Madara=Asuma, Asuma=Kakashi -> Madara tetap Asuma.
 */
namespace AIRegistry
{
	inline std::unordered_map<std::string, std::string> &store()
	{
		static std::unordered_map<std::string, std::string> map;
		return map;
	}

	inline size_t count()
	{
		return store().size();
	}

	// Return false kalau input tidak valid. ai == name berarti alias dihapus.
	inline bool set(const std::string &name, const std::string &ai)
	{
		if (name.empty() || ai.empty())
			return false;
		if (name == ai)
		{
			store().erase(name);
			return true;
		}
		store()[name] = ai;
		return true;
	}

	inline bool remove(const std::string &name)
	{
		return store().erase(name) > 0;
	}

	inline void clear()
	{
		store().clear();
	}

	inline bool has(const std::string &name)
	{
		return store().find(name) != store().end();
	}

	// Nama AI yang dipakai `name`; kalau tidak ada alias, kembali ke `name` sendiri.
	inline const std::string &resolve(const std::string &name)
	{
		auto it = store().find(name);
		return it != store().end() ? it->second : name;
	}
} // namespace AIRegistry

/**
 * Alias AI monster/summon (CloneMon): "monster X memakai perilaku monster Y".
 *
 * Contoh: MonRegistry::set("Chibaku", "JibakuEX") -> CharacterBase::setMon("Chibaku")
 * membuat Monster dengan ID/nama "Chibaku" (resource, sprite, data tetap dari "Chibaku"),
 * tapi posisi spawn, mode attack/doAI, dan urutan z-order mengikuti cabang "JibakuEX".
 *
 * Diisi dari Lua lewat tabel global `CloneMon` (lihat Systems/MonsterSystem.hpp
 * dan lua/Hook/Monster.lua). Resolve hanya satu tingkat (tidak berantai).
 */
namespace MonRegistry
{
	inline std::unordered_map<std::string, std::string> &store()
	{
		static std::unordered_map<std::string, std::string> map;
		return map;
	}

	inline size_t count()
	{
		return store().size();
	}

	// Return false kalau input tidak valid. ai == name berarti alias dihapus.
	inline bool set(const std::string &name, const std::string &ai)
	{
		if (name.empty() || ai.empty())
			return false;
		if (name == ai)
		{
			store().erase(name);
			return true;
		}
		store()[name] = ai;
		return true;
	}

	inline bool remove(const std::string &name)
	{
		return store().erase(name) > 0;
	}

	inline void clear()
	{
		store().clear();
	}

	inline bool has(const std::string &name)
	{
		return store().find(name) != store().end();
	}

	// Nama AI yang dipakai `name`; kalau tidak ada alias, kembali ke `name` sendiri.
	inline const std::string &resolve(const std::string &name)
	{
		auto it = store().find(name);
		return it != store().end() ? it->second : name;
	}
} // namespace MonRegistry

/**
 * Alias AI per-tick monster (CloneMonAI): "monster X memakai Monster::setAI milik monster Y".
 *
 * Contoh: MonAIRegistry::set("Shakuton", "Rocket") -> Monster::setAI() untuk monster
 * bernama "Shakuton" berjalan dengan logika AI "Rocket" (kejar target, stepOn saat tidak
 * ada target, dst), tapi nama/resource/sprite unit tetap "Shakuton".
 *
 * Beda dengan CloneMon (MonRegistry) yang hanya mengubah cabang spawn di CharacterBase::setMon.
 * Keduanya bisa dipakai bersamaan.
 *
 * Diisi dari Lua lewat tabel global `CloneMonAI` (lihat Systems/MonsterAISystem.hpp
 * dan lua/Hook/MonsterAI.lua). Resolve hanya satu tingkat (tidak berantai).
 */
namespace MonAIRegistry
{
	inline std::unordered_map<std::string, std::string> &store()
	{
		static std::unordered_map<std::string, std::string> map;
		return map;
	}

	inline size_t count()
	{
		return store().size();
	}

	// Return false kalau input tidak valid. ai == name berarti alias dihapus.
	inline bool set(const std::string &name, const std::string &ai)
	{
		if (name.empty() || ai.empty())
			return false;
		if (name == ai)
		{
			store().erase(name);
			return true;
		}
		store()[name] = ai;
		return true;
	}

	inline bool remove(const std::string &name)
	{
		return store().erase(name) > 0;
	}

	inline void clear()
	{
		store().clear();
	}

	inline bool has(const std::string &name)
	{
		return store().find(name) != store().end();
	}

	// Nama AI yang dipakai `name`; kalau tidak ada alias, kembali ke `name` sendiri.
	inline const std::string &resolve(const std::string &name)
	{
		auto it = store().find(name);
		return it != store().end() ? it->second : name;
	}
} // namespace MonAIRegistry

/**
 * AI monster dari XML (alternatif selain Lua CloneMon / CloneMonAI).
 *
 * Monster boleh menulis nama AI-nya langsung di Resources/Unit/Mon/<Nama>.xml, di data
 * action <Idle> (item pertama, dibaca ke Monster::_xmlAIType):
 *     <p type="attackType">Jibaku</p>
 * Maka monster itu memakai AI/cabang "Jibaku" (spawn di CharacterBase::setMon DAN
 * per-tick di Monster::setAI), sementara nama/sprite/resource tetap milik monster itu.
 *
 * Prioritas: attackType XML > alias Lua (CloneMon / CloneMonAI) > nama monster sendiri.
 * (XML = data milik monster itu sendiri, jadi menang atas daftar global di Lua.)
 * attackType generik ("nAttack", "aAttack", atau kosong) TIDAK dianggap nama AI, jadi
 * monster lama yang XML-nya masih "nAttack" berperilaku persis seperti sebelumnya.
 */
namespace MonXmlAI
{
	inline bool isGenericAttackType(const std::string &attackType)
	{
		return attackType.empty() || attackType == "nAttack" || attackType == "aAttack";
	}

	// AI spawn (CharacterBase::setMon). Mengembalikan referensi ke salah satu argumen.
	inline const std::string &resolveSpawn(const std::string &name, const std::string &xmlAttackType)
	{
		if (!isGenericAttackType(xmlAttackType))
			return xmlAttackType;
		if (MonRegistry::has(name))
			return MonRegistry::resolve(name);
		return name;
	}

	// AI per-tick (Monster::setAI). Mengembalikan referensi ke salah satu argumen.
	inline const std::string &resolveTick(const std::string &name, const std::string &xmlAttackType)
	{
		if (!isGenericAttackType(xmlAttackType))
			return xmlAttackType;
		if (MonAIRegistry::has(name))
			return MonAIRegistry::resolve(name);
		return name;
	}
} // namespace MonXmlAI

/**
 * Peta alias generik "nama -> nama lain" (satu tingkat, tidak berantai).
 * Dipakai BulletRegistry dan CatchRegistry di bawah; jembatan Lua-nya ada di
 * Systems/AliasBridge.hpp.
 */
class AliasMap
{
public:
	size_t count() const { return _map.size(); }

	// Return false kalau input tidak valid. alias == name berarti alias dihapus.
	bool set(const std::string &name, const std::string &alias)
	{
		if (name.empty() || alias.empty())
			return false;
		if (name == alias)
		{
			_map.erase(name);
			return true;
		}
		_map[name] = alias;
		return true;
	}

	bool remove(const std::string &name) { return _map.erase(name) > 0; }
	void clear() { _map.clear(); }
	bool has(const std::string &name) const { return _map.find(name) != _map.end(); }

	// Nama yang dipakai `name`; kalau tidak ada alias, kembali ke `name` sendiri.
	// PERHATIAN: yang dikembalikan bisa referensi ke argumen `name`; simpan sebagai
	// std::string (salinan) kalau argumen yang diberikan adalah objek sementara.
	const std::string &resolve(const std::string &name) const
	{
		auto it = _map.find(name);
		return it != _map.end() ? it->second : name;
	}

private:
	std::unordered_map<std::string, std::string> _map;
};

/**
 * Alias peluru (CloneBullet): "peluru X memakai cabang spawn peluru Y".
 *
 * Contoh: BulletRegistry::set("Sanpu", "TentenSRK") -> CharacterBase::setBullet("Sanpu")
 * membuat Bullet dengan ID/nama "Sanpu" (sprite, data, resource tetap dari "Sanpu"),
 * tapi posisi spawn, skala, dan gerakan (setEaseIn/setMove/attack) mengikuti cabang
 * "TentenSRK".
 *
 * Diisi dari Lua lewat tabel global `CloneBullet` (lihat lua/Hook/Bullet.lua).
 */
namespace BulletRegistry
{
	inline AliasMap &map()
	{
		static AliasMap m;
		return m;
	}
} // namespace BulletRegistry

/**
 * Alias tangkapan "ct_hit" (CloneCatch): "master X diperlakukan seperti master Y".
 *
 * Contoh: CatchRegistry::set("Guy", "Itachi") -> di CharacterBase::onHurt untuk hit
 * "ct_hit", peluru/summon milik master "Guy" memakai cabang Itachi
 * (hardHurt 3000 + target ditarik ke posisi attacker), padahal nama master tetap "Guy".
 * Cabang yang bisa dituju: Itachi (3000) dan Chiyo (2000).
 *
 * Diisi dari Lua lewat tabel global `CloneCatch` (lihat lua/Hook/Catch.lua).
 */
namespace CatchRegistry
{
	inline AliasMap &map()
	{
		static AliasMap m;
		return m;
	}
} // namespace CatchRegistry

/**
 * Daftar transformasi karakter (Transform): "karakter X berubah menjadi Y".
 *
 * Contoh: TransformRegistry::set("Guy", "Guy8Gates") -> saat CharacterBase::setTransform()
 * dipanggil (key "setTransform" di plist aksi) oleh unit bernama "Guy", unit dipanggil ulang
 * lewat setID("Guy8Gates", _role, _group), setara dengan
 *     if (getName() == "Guy") setID("Guy8Gates", _role, _group);
 *
 * Diisi dari Lua lewat tabel global `Transform` (lihat Systems/TransformSystem.hpp
 * dan lua/Hook/Transform.lua). Satu langkah saja (tidak berantai); untuk rantai
 * (Naruto -> SageNaruto -> RikudoNaruto) cukup daftarkan tiap langkahnya.
 */
namespace TransformRegistry
{
	inline std::unordered_map<std::string, std::string> &store()
	{
		static std::unordered_map<std::string, std::string> map;
		return map;
	}

	inline size_t count()
	{
		return store().size();
	}

	// Return false kalau input tidak valid. to == name berarti transformasi dihapus.
	inline bool set(const std::string &name, const std::string &to)
	{
		if (name.empty() || to.empty())
			return false;
		if (name == to)
		{
			store().erase(name);
			return true;
		}
		store()[name] = to;
		return true;
	}

	inline bool remove(const std::string &name)
	{
		return store().erase(name) > 0;
	}

	inline void clear()
	{
		store().clear();
	}

	inline bool has(const std::string &name)
	{
		return store().find(name) != store().end();
	}

	// Nama hasil transformasi `name`; kalau tidak terdaftar, kembali ke `name` sendiri.
	inline const std::string &resolve(const std::string &name)
	{
		auto it = store().find(name);
		return it != store().end() ? it->second : name;
	}
} // namespace TransformRegistry

/**
 * Aset tambahan per karakter (Assets): plist sprite + file suara yang ikut di-load /
 * di-unload bersama karakter tersebut.
 *
 * Contoh: AssetRegistry::add("Guy", AssetRegistry::Ninja, "Guy8Gates") ->
 *   LoadLayer::perloadCharIMG("Guy")   => addSprites("Unit/Ninja/Guy8Gates/Guy8Gates.plist")
 *   LoadLayer::unloadCharIMG(unit Guy) => removeSprites("Unit/Ninja/Guy8Gates/Guy8Gates.plist")
 * setara dengan
 *     if (name == HeroEnum::Guy) { addSprites(mk_ninja_plist("Guy8Gates")); ... }
 *
 * Diisi dari Lua lewat tabel global `Assets` (lihat Systems/AssetSystem.hpp dan
 * lua/Hook/Assets.lua). Entri dijalankan sesuai urutan didaftarkan, SETELAH percabangan
 * bawaan C++ di LoadLayer.
 */
namespace AssetRegistry
{
	enum Kind
	{
		Ninja,	  // Unit/Ninja/<n>/<n>.plist
		Skill,	  // Unit/Ninja/<n>/<n>_Skill.plist
		Kugutsu,  // Unit/Kugutsu/<n>/<n>.plist
		Kuchiyose, // Unit/Kuchiyose/<n>/<n>.plist
		Guardian, // Unit/Guardian/<n>/<n>.plist
		Flog,	  // Unit/Flog/<n>.plist
		Plist,	  // path plist mentah (relatif Resources)
		Ogg		  // KTools::prepareFileOGG(<n>)
	};

	struct Entry
	{
		Kind kind;
		std::string value;
	};

	inline std::unordered_map<std::string, std::vector<Entry>> &store()
	{
		static std::unordered_map<std::string, std::vector<Entry>> map;
		return map;
	}

	inline bool add(const std::string &name, Kind kind, const std::string &value)
	{
		if (name.empty() || value.empty())
			return false;
		auto &list = store()[name];
		for (const auto &e : list)
			if (e.kind == kind && e.value == value)
				return true; // sudah ada, jangan dobel
		list.push_back({kind, value});
		return true;
	}

	inline bool remove(const std::string &name)
	{
		return store().erase(name) > 0;
	}

	inline void clear()
	{
		store().clear();
	}

	inline bool has(const std::string &name)
	{
		auto it = store().find(name);
		return it != store().end() && !it->second.empty();
	}

	inline size_t count()
	{
		return store().size();
	}

	inline const std::vector<Entry> *find(const std::string &name)
	{
		auto it = store().find(name);
		return it != store().end() ? &it->second : nullptr;
	}

	// Path plist untuk entri (string kosong untuk Ogg).
	inline std::string plistPath(const Entry &e)
	{
		const std::string &n = e.value;
		switch (e.kind)
		{
		case Ninja:
			return "Unit/Ninja/" + n + "/" + n + ".plist";
		case Skill:
			return "Unit/Ninja/" + n + "/" + n + "_Skill.plist";
		case Kugutsu:
			return "Unit/Kugutsu/" + n + "/" + n + ".plist";
		case Kuchiyose:
			return "Unit/Kuchiyose/" + n + "/" + n + ".plist";
		case Guardian:
			return "Unit/Guardian/" + n + "/" + n + ".plist";
		case Flog:
			return "Unit/Flog/" + n + ".plist";
		case Plist:
			return n;
		default:
			return "";
		}
	}
} // namespace AssetRegistry
