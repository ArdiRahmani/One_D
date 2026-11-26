#pragma once
#include <algorithm>
#include <cctype>
#include <cstring>
#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>
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
 * AI karakter yang ditulis di Lua (LuaAI): "unit X dijalankan oleh script lua/AI/<Y>.lua".
 *
 * Contoh: LuaAIRegistry::set("Yagura", "Tsunade") -> Provider::create("Yagura", ...)
 * membuat LuaHero yang menjalankan lua/AI/Tsunade.lua. Unit "Tsunade" tidak ikut berubah,
 * tetap memakai class C++ Tsunade (Tsunade.hpp), kecuali juga di-bind lewat LuaAI.
 *
 * Beda dengan AIRegistry (CloneAI): CloneAI mengalihkan ke class C++ yang sudah ada,
 * LuaAI mengalihkan ke script Lua. Kalau sebuah unit ada di keduanya, LuaAI menang
 * (kalau script gagal dimuat, Provider jatuh balik ke CloneAI / class C++ biasa).
 *
 * Diisi dari Lua lewat tabel global `LuaAI` (lihat Systems/LuaAISystem.hpp dan
 * lua/Hook/LuaAI.lua). name == script TIDAK dihapus (beda dengan AIRegistry), karena
 * "Tsunade" -> lua/AI/Tsunade.lua adalah binding yang sah.
 */
namespace LuaAIRegistry
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

	inline bool set(const std::string &name, const std::string &script)
	{
		if (name.empty() || script.empty())
			return false;
		store()[name] = script;
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

	// Nama script milik `name`; string kosong kalau tidak ada.
	inline std::string get(const std::string &name)
	{
		auto it = store().find(name);
		return it != store().end() ? it->second : std::string();
	}
} // namespace LuaAIRegistry

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
 * AI monster dari XML (fallback kalau tidak ada alias Lua CloneMon / CloneMonAI).
 *
 * Monster boleh menulis nama AI-nya langsung di Resources/Unit/Mon/<Nama>.xml, di data
 * action <Idle> (item pertama, dibaca ke Monster::_xmlAIType):
 *     <p type="attackType">Jibaku</p>
 * Maka monster itu memakai AI/cabang "Jibaku" (spawn di CharacterBase::setMon DAN
 * per-tick di Monster::setAI), sementara nama/sprite/resource tetap milik monster itu.
 *
 * Prioritas: alias Lua (CloneMon / CloneMonAI) > attackType XML > nama monster sendiri (default).
 * Kalau monster punya alias di Lua, XML dilewati. Kalau tidak ada alias Lua, dipakai attackType XML.
 * Kalau XML juga tidak ada, jatuh ke default (nama monster sendiri).
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
		// 1) Lua (CloneMon)
		if (MonRegistry::has(name))
			return MonRegistry::resolve(name);
		// 2) XML (attackType)
		if (!isGenericAttackType(xmlAttackType))
			return xmlAttackType;
		// 3) Default
		return name;
	}

	// AI per-tick (Monster::setAI). Mengembalikan referensi ke salah satu argumen.
	inline const std::string &resolveTick(const std::string &name, const std::string &xmlAttackType)
	{
		// 1) Lua (CloneMonAI)
		if (MonAIRegistry::has(name))
			return MonAIRegistry::resolve(name);
		// 2) XML (attackType)
		if (!isGenericAttackType(xmlAttackType))
			return xmlAttackType;
		// 3) Default
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
 * Alias tipe buff (Buffs): tipe buff baru = perilaku tipe buff bawaan.
 *
 *   alias -> tipe bawaan, mis. h2Buff -> hBuff
 *
 * Tipe bawaan yang dikenal CharacterBase::setBuff: hBuff, sBuff, rsBuff, hsBuff, dcBuff, cBuff,
 * tBuff, t2Buff, gBuff, stBuff, GroupHeal, GroupBuff (+ efek visual di setBuffEffect:
 * hBuff, sBuff, hsBuff, tBuff, t2Buff, dcBuff, jdBuff, bmBuff, dhBuff).
 * Tipe yang ditulis di data aksi (attackType skill) dan tidak terdaftar dipakai apa adanya.
 *
 * Diisi dari Lua lewat tabel global `Buffs` (Systems/BuffSystem.hpp, lua/Hook/Buffs.lua).
 */
namespace BuffRegistry
{
	inline std::unordered_map<std::string, std::string> &store()
	{
		static std::unordered_map<std::string, std::string> map;
		return map;
	}

	inline size_t count() { return store().size(); }

	inline bool set(const std::string &alias, const std::string &base)
	{
		if (alias.empty() || base.empty())
			return false;
		if (alias == base)
		{
			store().erase(alias);
			return true;
		}
		store()[alias] = base;
		return true;
	}

	inline bool remove(const std::string &alias) { return store().erase(alias) > 0; }

	inline bool has(const std::string &alias) { return store().find(alias) != store().end(); }

	// Efek visual buff buatan Lua (Buffs.effect): animasi frame "<prefix>01..NN" yang menempel di unit.
	struct EffectDef
	{
		std::string prefix;
		size_t frames = 1;
		unsigned fps = 10;
		bool loop = true;
		float anchorX = 0.5f;
		float anchorY = 0.0f;
		float offsetX = 0.0f;
		float offsetY = 0.0f;
	};

	inline std::unordered_map<std::string, EffectDef> &effects()
	{
		static std::unordered_map<std::string, EffectDef> map;
		return map;
	}

	inline void clear()
	{
		store().clear();
		effects().clear();
	}

	inline bool setEffect(const std::string &name, const EffectDef &def)
	{
		if (name.empty() || def.prefix.empty() || def.frames == 0)
			return false;
		effects()[name] = def;
		return true;
	}

	inline bool removeEffect(const std::string &name) { return effects().erase(name) > 0; }

	inline const EffectDef *effectOf(const std::string &name)
	{
		auto it = effects().find(name);
		return it != effects().end() ? &it->second : nullptr;
	}

	// Tipe bawaan untuk `type`; ikuti rantai alias (maks 8 langkah, anti-loop). Tidak terdaftar -> `type`.
	inline std::string resolve(const std::string &type)
	{
		std::string cur = type;
		for (int i = 0; i < 8; ++i)
		{
			auto it = store().find(cur);
			if (it == store().end())
				break;
			cur = it->second;
		}
		return cur;
	}
} // namespace BuffRegistry

/**
 * Flag perilaku per karakter (UnitFlags): pengganti daftar nama hard-coded di CharacterBase,
 * supaya hero baru (mis. Madara yang memakai AI Hashirama.lua) bisa ikut perilaku yang sama.
 *
 *   HoldBuff : CharacterBase::setBuff TIDAK menjadwalkan resumeAction otomatis setelah buffStayTime
 *              (mode berakhir lewat event setActionResume di XML aksi). Setara dengan daftar
 *              `getName() != Hashirama/Hiruzen/Suigetsu/Jugo/Kisame/...` di CharacterBase::setBuff.
 *   LockWalk : CharacterBase::walk TIDAK menggerakkan AI (bukan player) yang sedang NATTACK saat
 *              _isOnlySkillLocked. Setara dengan `getName() == Suigetsu/Jugo/Hashirama/Hiruzen/Kisame`
 *              di CharacterBase::walk.
 *
 * Ini TAMBAHAN: nama yang sudah ditulis di C++ tetap berlaku, flag hanya menambah.
 * Diisi dari Lua lewat tabel global `UnitFlags` (Systems/UnitFlagSystem.hpp, lua/Hook/UnitFlags.lua).
 */
namespace UnitFlagRegistry
{
	enum Flag : unsigned
	{
		HoldBuff = 1u << 0,
		LockWalk = 1u << 1,
		// Perilaku CharacterBase::healBuff (GroupBuff / GroupHeal / hBuff), per tipe:
		HealGroup  = 1u << 2, // seperti Karin  : heal HP semua rekan hero dalam jangkauan
		HealChakra = 1u << 3, // seperti Chiyo  : tambah chakra semua rekan hero (kecuali dirinya)
		HealSelf   = 1u << 4, // seperti Sakura/Tsunade : heal diri sendiri
		HealFlog   = 1u << 5  // seperti Slug   : heal pasukan (flog) satu tim dalam jangkauan
	};

	inline std::unordered_map<std::string, unsigned> &store()
	{
		static std::unordered_map<std::string, unsigned> map;
		return map;
	}

	inline size_t count() { return store().size(); }

	inline bool add(const std::string &name, unsigned flags)
	{
		if (name.empty() || flags == 0)
			return false;
		store()[name] |= flags;
		return true;
	}

	inline bool remove(const std::string &name) { return store().erase(name) > 0; }

	inline void clear() { store().clear(); }

	inline unsigned get(const std::string &name)
	{
		auto it = store().find(name);
		return it != store().end() ? it->second : 0u;
	}

	inline bool has(const std::string &name, Flag f) { return (get(name) & f) != 0; }
} // namespace UnitFlagRegistry

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

/**
 * Binding gambar HERO (Icons): _rp.png, _rpf.png, _CutIn.png (plus _avator / _small).
 * Aturannya sama dengan half.png di GameOverRegistry, tetapi gambarnya tetap diambil
 * dari plist (SpriteFrameCache), bukan dari file.
 *
 * Tiap jenis gambar punya pengaturan sendiri (rp tidak ikut berubah kalau rpf diganti):
 *   frame        : templat nama frame di plist, "{}" diganti nama hero sumber.
 *                  Default "{}_rp.png" / "{}_rpf.png" / "{}_CutIn.png".
 *   file         : unit -> nama frame LENGKAP di plist (mengalahkan frame + bind).
 *   bind         : unit -> hero sumber. bind["Madara"] = "Obito" -> Madara memakai
 *                  Obito_rpf.png (arah SAMA dengan GameOver.half: [unit] = sumber).
 *   defaultScale : skala sprite untuk SEMUA unit yang tidak punya entri di `scale`. Default 1.0.
 *   scale        : unit -> skala sprite (setScale); mengalahkan defaultScale.
 *
 * Skala hanya dipakai untuk rp, rpf, dan CutIn. Avator dan small cuma mendukung bind/file/frame.
 *
 * Khusus hero: nama banner kill-streak (DoubleKill, KillingSpree, ..., MonsterKill) dan Tower
 * ditolak dari binding, jadi KillingSpree_rpf.png dkk tidak pernah ikut terganti.
 *
 * Khusus rpf (nama hero di laporan kill, digambar sebagai TEKS) ada `text` = TextStyle:
 *   font, size, color, glow (aktif/tidak, warna, blur, intensitas), strip (kata yang dibuang
 *   dari nama, mis. "Sage" / "Rikudo": SageNaruto -> Naruto), names (unit -> teks tampil).
 *
 * Satu langkah saja (tidak berantai). Unit yang tidak terdaftar memakai namanya sendiri.
 * Diisi dari Lua lewat tabel global `Icons` (lihat Systems/IconSystem.hpp dan lua/Hook/Icons.lua).
 * Pencarian frame (termasuk fallback bila frame tujuan tidak ada) ada di iconFrameName()
 * pada Utils/Cocos2dxHelper.hpp.
 */
namespace IconRegistry
{
	enum Kind
	{
		Rp,		// <n>_rp.png      ikon kecil laporan kill
		Rpf,	// <n>_rpf.png     ikon besar laporan kill
		CutIn,	// <n>_CutIn.png   potret saat ougi / cut-in
		Avator, // <n>_avator.png  tombol avatar di HUD
		Small,	// <n>_small.png   ikon kecil layar pilih / hasil akhir
		KindCount
	};

	// Nama jenis di Lua (tidak peka huruf besar/kecil), akhiran frame sebenarnya, dan
	// apakah skala dipakai untuk jenis ini.
	struct KindInfo
	{
		const char *key;
		const char *suffix;
		bool scalable;
	};

	inline const KindInfo &info(Kind k)
	{
		static const KindInfo table[KindCount] = {
			{"rp", "rp", true},
			{"rpf", "rpf", true},
			{"cutin", "CutIn", true},
			{"avator", "avator", false},
			{"small", "small", false},
		};
		return table[k];
	}

	inline bool findKind(const std::string &key, Kind &out)
	{
		std::string lower = key;
		std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return (char)std::tolower(c); });
		for (int i = 0; i < KindCount; i++)
		{
			if (lower == info((Kind)i).key)
			{
				out = (Kind)i;
				return true;
			}
		}
		return false;
	}

	// Nama yang BUKAN hero: Tower dan banner kill-streak (createSPCReport di HudLayer).
	inline bool isReservedName(const std::string &name)
	{
		static const char *kReserved[] = {
			"Tower", "DoubleKill", "KillingSpree", "Unstoppable", "Godlike", "TripeKill",
			"Domination", "Rampage", "Holyshit", "MonsterKill", "slain", "destroy"};
		for (const char *r : kReserved)
		{
			if (name == r)
				return true;
		}
		return false;
	}

	inline bool isHeroUnit(const std::string &name) { return !name.empty() && !isReservedName(name); }

	struct Config
	{
		AliasMap bind;								// unit -> hero sumber
		std::unordered_map<std::string, std::string> file;	// unit -> nama frame lengkap
		std::string frameTemplate;						// "{}" = nama hero sumber
		float defaultScale = 1.0f;
		std::unordered_map<std::string, float> scale;	// unit -> skala
	};

	// Gaya TEKS nama hero di laporan kill (khusus jenis Rpf). Diisi dari Lua: Icons.rpf.text / Icons.setText.
	struct RGB
	{
		unsigned char r = 255, g = 255, b = 255;
	};

	struct TextStyle
	{
		std::string font = "Fonts/Roboto-Medium.ttf"; // path file .ttf di Resources
		float size = 21.0f;
		RGB color = {255, 255, 255};
		bool glow = true;
		RGB glowColor = {34, 138, 32};
		float glowBlur = 5.0f;
		float glowIntensity = 0.8f;				  // 0..1
		std::vector<std::string> strip;			  // potongan kata yang dibuang dari nama, mis. "Sage", "Rikudo"
		std::unordered_map<std::string, std::string> names; // unit -> teks tampil (mengalahkan semuanya)
	};

	inline TextStyle &textStyle()
	{
		static TextStyle t;
		return t;
	}

	inline std::string defaultTemplate(Kind k) { return std::string("{}_") + info(k).suffix + ".png"; }

	inline Config &config(Kind k)
	{
		static Config configs[KindCount];
		static bool inited = false;
		if (!inited)
		{
			for (int i = 0; i < KindCount; i++)
				configs[i].frameTemplate = defaultTemplate((Kind)i);
			inited = true;
		}
		return configs[k];
	}

	// Ganti "{}" pertama di templat dengan `name`. Templat tanpa "{}" dipakai apa adanya.
	inline std::string fillTemplate(const std::string &tpl, const std::string &name)
	{
		std::string out = tpl;
		size_t pos = out.find("{}");
		if (pos != std::string::npos)
			out.replace(pos, 2, name);
		return out;
	}

	// Setelan jenis `k` dikembalikan ke bawaan (templat bawaan, skala 1.0, tanpa binding).
	inline void clear(Kind k)
	{
		Config &c = config(k);
		c.bind.clear();
		c.file.clear();
		c.frameTemplate = defaultTemplate(k);
		c.defaultScale = 1.0f;
		c.scale.clear();
		if (k == Rpf)
			textStyle() = TextStyle();
	}

	inline void clearAll()
	{
		for (int i = 0; i < KindCount; i++)
			clear((Kind)i);
	}

	inline size_t count()
	{
		size_t n = 0;
		for (int i = 0; i < KindCount; i++)
		{
			Config &c = config((Kind)i);
			n += c.bind.count() + c.file.size() + c.scale.size();
			if (c.frameTemplate != defaultTemplate((Kind)i))
				n++;
			if (c.defaultScale != 1.0f)
				n++;
		}
		n += textStyle().strip.size() + textStyle().names.size();
		return n;
	}

	// unit memakai gambar hero `source`. source == unit berarti binding dihapus.
	// Return false kalau input kosong atau salah satunya bukan hero (Tower / banner kill-streak).
	inline bool setBind(Kind k, const std::string &unit, const std::string &source)
	{
		if (!isHeroUnit(unit) || !isHeroUnit(source))
			return false;
		return config(k).bind.set(unit, source);
	}

	inline bool removeBind(Kind k, const std::string &unit) { return config(k).bind.remove(unit); }

	inline bool setFile(Kind k, const std::string &unit, const std::string &frame)
	{
		if (!isHeroUnit(unit) || frame.empty())
			return false;
		config(k).file[unit] = frame;
		return true;
	}

	inline bool removeFile(Kind k, const std::string &unit) { return config(k).file.erase(unit) > 0; }

	inline bool setTemplate(Kind k, const std::string &tpl)
	{
		if (tpl.empty())
			return false;
		config(k).frameTemplate = tpl;
		return true;
	}

	inline bool setDefaultScale(Kind k, float v)
	{
		if (v <= 0)
			return false;
		config(k).defaultScale = v;
		return true;
	}

	inline bool setScale(Kind k, const std::string &unit, float v)
	{
		if (!isHeroUnit(unit) || v <= 0)
			return false;
		config(k).scale[unit] = v;
		return true;
	}

	inline bool removeScale(Kind k, const std::string &unit) { return config(k).scale.erase(unit) > 0; }

	// Ada binding / file untuk unit ini?
	inline bool has(Kind k, const std::string &unit)
	{
		Config &c = config(k);
		return c.bind.has(unit) || c.file.count(unit) > 0;
	}

	// Nama hero sumber gambar untuk `unit` (= unit sendiri kalau tidak ada binding). Salinan.
	inline std::string resolve(Kind k, const std::string &unit) { return std::string(config(k).bind.resolve(unit)); }

	// Nama frame bawaan unit sendiri (fallback kalau frame hasil binding tidak ada di plist).
	inline std::string ownFrame(Kind k, const std::string &unit) { return fillTemplate(defaultTemplate(k), unit); }

	// Nama frame yang diinginkan Lua untuk `unit` (belum dicek ada/tidaknya di plist).
	// Unit non-hero selalu memakai frame sendiri.
	inline std::string frameFor(Kind k, const std::string &unit)
	{
		if (!isHeroUnit(unit))
			return ownFrame(k, unit);
		Config &c = config(k);
		auto it = c.file.find(unit);
		if (it != c.file.end())
			return it->second;
		return fillTemplate(c.frameTemplate, std::string(c.bind.resolve(unit)));
	}

	// Skala sprite untuk `unit`. Jenis tanpa dukungan skala (avator, small) dan unit non-hero = 1.0.
	inline float scaleOf(Kind k, const std::string &unit)
	{
		if (!info(k).scalable || !isHeroUnit(unit))
			return 1.0f;
		Config &c = config(k);
		auto it = c.scale.find(unit);
		return it != c.scale.end() ? it->second : c.defaultScale;
	}

	// Hapus semua potongan kata `strip` dari `name` (peka huruf besar/kecil). Kalau hasilnya
	// kosong, nama asli dipakai supaya laporan kill tidak pernah menampilkan teks kosong.
	inline std::string stripWords(const std::string &name, const std::vector<std::string> &words)
	{
		std::string out = name;
		for (const std::string &w : words)
		{
			if (w.empty())
				continue;
			size_t pos = 0;
			while ((pos = out.find(w, pos)) != std::string::npos)
				out.erase(pos, w.size());
		}
		return out.empty() ? name : out;
	}

	// Teks nama hero di laporan kill (rpf). Urutan:
	//   1. names[unit] dari Lua (dipakai apa adanya)
	//   2. nama dari frame hasil file / bind / frame (tanpa ".png" dan "_rpf"), lalu dibuang `strip`
	inline std::string displayName(const std::string &unit)
	{
		const TextStyle &t = textStyle();
		auto it = t.names.find(unit);
		if (it != t.names.end() && !it->second.empty())
			return it->second;

		std::string name = frameFor(Rpf, unit);
		const std::string kPng = ".png";
		const std::string kSuffix = "_rpf";
		if (name.size() > kPng.size() && name.compare(name.size() - kPng.size(), kPng.size(), kPng) == 0)
			name.erase(name.size() - kPng.size());
		if (name.size() > kSuffix.size() && name.compare(name.size() - kSuffix.size(), kSuffix.size(), kSuffix) == 0)
			name.erase(name.size() - kSuffix.size());
		return isHeroUnit(unit) ? stripWords(name, t.strip) : name;
	}
} // namespace IconRegistry

/**
 * Pengaturan layar hasil akhir (GameOver): gambar half.png, daftar flipX, dan nama catatan
 * (CharRecord) per karakter. Pengganti daftar `getName() == ...` yang dulu hard-coded di
 * GameOver::listResult().
 *
 *   flipX  : unit yang gambar half-nya dibalik horizontal (setFlipX(true)).
 *            Setara `if (name == Konan || name == Karin || ...) half->setFlipX(true);`
 *   half   : unit -> hero sumber gambar Img/Half/<sumber>_half.png.
 *            Contoh: half["SageJiraiya"] = "Jiraiya" -> Img/Half/Jiraiya_half.png.
 *            Kalau file tujuan tidak ada, kembali ke Img/Half/<unit>_half.png.
 *   path   : templat path file half, "{}" diganti nama hero sumber.
 *            Default "Img/Half/{}_half.png".
 *   file   : unit -> path file half LENGKAP (mengalahkan path + half), mis.
 *            file["Madara"] = "Img/Half/Custom/madara_big.png".
 *   defaultScale : skala half untuk SEMUA unit yang tidak punya entri di `scale`. Default 1.0.
 *   scale  : unit -> skala gambar half (half->setScale); mengalahkan defaultScale.
 *            Contoh: scale["Chiyo"] = 0.8. Posisi (rata kanan) ikut menyesuaikan skala.
 *   record : unit -> nama baris CharRecord (jumlah menang & waktu terbaik) di database.
 *            Contoh: record["SageNaruto"] = "Naruto" -> kemenangan SageNaruto dicatat di "Naruto".
 *
 * Satu langkah saja (tidak berantai). Unit yang tidak terdaftar memakai namanya sendiri.
 * Diisi dari Lua lewat tabel global `GameOver` (lihat Systems/GameOverSystem.hpp dan
 * lua/Hook/GameOver.lua).
 */
namespace GameOverRegistry
{
	inline std::unordered_set<std::string> &flipSet()
	{
		static std::unordered_set<std::string> s;
		return s;
	}

	inline AliasMap &halfMap()
	{
		static AliasMap m;
		return m;
	}

	inline std::unordered_map<std::string, float> &scaleMap()
	{
		static std::unordered_map<std::string, float> m;
		return m;
	}

	inline const char *defaultHalfPath() { return "Img/Half/{}_half.png"; }

	inline std::string &pathTemplate()
	{
		static std::string t = defaultHalfPath();
		return t;
	}

	inline std::unordered_map<std::string, std::string> &fileMap()
	{
		static std::unordered_map<std::string, std::string> m;
		return m;
	}

	// Ganti "{}" pertama di templat dengan `name`. Templat tanpa "{}" dipakai apa adanya.
	inline std::string fillTemplate(const std::string &tpl, const std::string &name)
	{
		std::string out = tpl;
		size_t pos = out.find("{}");
		if (pos != std::string::npos)
			out.replace(pos, 2, name);
		return out;
	}

	// Path file half yang diinginkan Lua untuk `unit` (belum dicek ada/tidaknya).
	inline std::string halfPath(const std::string &unit)
	{
		auto it = fileMap().find(unit);
		if (it != fileMap().end())
			return it->second;
		return fillTemplate(pathTemplate(), std::string(halfMap().resolve(unit)));
	}

	// Path bawaan unit sendiri (fallback kalau halfPath() tidak ada filenya).
	inline std::string ownHalfPath(const std::string &unit) { return fillTemplate(defaultHalfPath(), unit); }

	inline float &defaultScale()
	{
		static float s = 1.0f;
		return s;
	}

	inline float halfScale(const std::string &unit)
	{
		auto it = scaleMap().find(unit);
		return it != scaleMap().end() ? it->second : defaultScale();
	}

	inline AliasMap &recordMap()
	{
		static AliasMap m;
		return m;
	}

	inline bool isFlipX(const std::string &unit) { return flipSet().count(unit) > 0; }

	// Nama hero sumber gambar half (= unit sendiri kalau tidak ada binding). Mengembalikan salinan.
	inline std::string halfName(const std::string &unit) { return std::string(halfMap().resolve(unit)); }

	// Nama baris CharRecord (= unit sendiri kalau tidak ada binding). Mengembalikan salinan.
	inline std::string recordName(const std::string &unit) { return std::string(recordMap().resolve(unit)); }

	inline size_t count() { return flipSet().size() + fileMap().size() + scaleMap().size() + halfMap().count() + recordMap().count(); }

	inline void clear()
	{
		flipSet().clear();
		halfMap().clear();
		scaleMap().clear();
		defaultScale() = 1.0f;
		fileMap().clear();
		pathTemplate() = defaultHalfPath();
		recordMap().clear();
	}
} // namespace GameOverRegistry
