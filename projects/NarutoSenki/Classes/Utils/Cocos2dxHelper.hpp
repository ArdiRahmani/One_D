#pragma once
#include "cocos2d.h"
#include "../../../scripting/lua/cocos2dx_support/CCLuaEngine.h"
#include "Utils/CCDeprecated.h"
#include "Data/UnitDefines.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_MAC
#include <format>
#else
#define FMT_HEADER_ONLY
#define FMT_CONSTEVAL
#include "fmt/core.h"

using namespace fmt;
#endif

using namespace cocos2d;
using namespace std;

#define nameof(varType) #varType
#define typeof(varType) typeid(varType).name()

#define to_uint(str) strtoul(str, nullptr, 10)
#define to_int(str) atoi(str)

#define RETURN_FALSE_IF(var) \
	if (var)                 \
		return false;

#define RETURN_TRUE_IF(var) \
	if (var)                \
		return true;

#define get_luastack LuaEngine::defaultEngine()->getLuaStack()
#define lua_getL auto L = LuaEngine::defaultEngine()->getLuaStack()->getLuaState();

namespace LuaBridge
{
static inline bool isReady()
{
	return LuaEngine::defaultEngine() != nullptr && LuaEngine::defaultEngine()->getLuaStack() != nullptr;
}

static inline bool hasGlobalFunction(lua_State *L, const char *funcName)
{
	lua_getglobal(L, funcName);
	if (!lua_isfunction(L, -1))
	{
		lua_pop(L, 1);
		return false;
	}
	return true;
}

static inline bool callGlobalNoArgs(const char *funcName, const char *context = nullptr)
{
	if (!isReady())
	{
		CCLOG("[LuaBridge] Lua engine not ready when calling %s", funcName ? funcName : "<null>");
		return false;
	}

	lua_getL;
	if (!funcName || !hasGlobalFunction(L, funcName))
	{
		CCLOG("[LuaBridge] Lua global function not found: %s (context: %s)",
			  funcName ? funcName : "<null>",
			  context ? context : "none");
		return false;
	}

	if (lua_pcall(L, 0, 0, 0) != LUA_OK)
	{
		const char *error = lua_tostring(L, -1);
		CCLOG("[LuaBridge] Failed to call %s (context: %s), error: %s",
			  funcName,
			  context ? context : "none",
			  error ? error : "<no error>");
		lua_pop(L, 1);
		return false;
	}

	return true;
}
} // namespace LuaBridge

#define lua_call_func(func_name) LuaBridge::callGlobalNoArgs(func_name, __FUNCTION__)

#define lua_call_init_func lua_call_handler_auto

#define lua_call_handler_auto                         \
	if (m_nScriptHandler != 0)                        \
	{                                                 \
		int handler = m_nScriptHandler;               \
		auto pStack = get_luastack;                   \
		pStack->executeFunctionByHandler(handler, 0); \
		m_nScriptHandler = 0; /* Only called once */  \
	}

#define lua_call_handler(handler) \
	auto pStack = get_luastack;   \
	pStack->executeFunctionByHandler(handler, 0);

/**
 * Sprite helpers
 */

#define FULL_SCREEN_SPRITE(__SPRITE__) \
	__SPRITE__->setScaleX(winSize.width / __SPRITE__->getContentSize().width);

static inline void addSprites(const string &plistName)
{
	SpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile(plistName.c_str());
}

static inline void removeSprites(const string &plistName)
{
	SpriteFrameCache::sharedSpriteFrameCache()->removeSpriteFramesFromFile(plistName.c_str());
}

static inline SpriteFrame *getSpriteFrame(const string &name)
{
	return SpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(name.c_str());
}

/**
 * Nama frame gambar hero dengan memperhitungkan binding Lua (IconRegistry / tabel `Icons`).
 *   iconFrameName(IconRegistry::Rpf, "Madara")  ->  "Obito_rpf.png"   (kalau bind Madara = 'Obito')
 *                                               ->  "Madara_rpf.png"  (kalau tidak ada binding)
 * Urutan: file (frame lengkap) > bind + templat frame > frame unit sendiri. Kalau frame hasil
 * binding tidak ada di SpriteFrameCache, kembali ke frame unit sendiri supaya
 * Sprite::createWithSpriteFrameName tidak crash. Unit non-hero (Tower, banner kill-streak)
 * selalu memakai frame sendiri.
 */
static inline string iconFrameName(IconRegistry::Kind kind, const string &unit)
{
	const string own = IconRegistry::ownFrame(kind, unit);
	if (!IconRegistry::isHeroUnit(unit))
		return own;

	const string wanted = IconRegistry::frameFor(kind, unit);
	if (wanted == own)
		return own;
	if (getSpriteFrame(wanted) != nullptr)
		return wanted;

	CCLOG("[Icons] frame %s tidak ada, pakai %s", wanted.c_str(), own.c_str());
	return own;
}

/**
 * Skala sprite gambar hero dari Lua (scale per unit, kalau tidak ada: defaultScale jenisnya,
 * bawaan 1.0). Hanya rp, rpf, dan CutIn; jenis lain selalu 1.0.
 */
static inline float iconScale(IconRegistry::Kind kind, const string &unit)
{
	return IconRegistry::scaleOf(kind, unit);
}

/**
 * Sprite gambar hero (frame dari plist + skala) sesuai binding Lua.
 * Ukuran tampil = getContentSize() * getScale().
 */
static inline Sprite *createIconSprite(IconRegistry::Kind kind, const string &unit)
{
	Sprite *sprite = Sprite::createWithSpriteFrameName(iconFrameName(kind, unit).c_str());
	if (sprite != nullptr)
		sprite->setScale(iconScale(kind, unit));
	return sprite;
}

#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_MAC

template <class... _Types>
static inline SpriteFrame *getSpriteFrame(std::format_string<_Types...> _Fmt, _Types &&..._Args)
{
	return getSpriteFrame(std::format(_Fmt, std::forward<_Types>(_Args)...));
}

#else

// Use fmt lib for platforms without C++20 std::format support

template <class... _Types>
static inline SpriteFrame *getSpriteFrame(fmt::format_string<_Types...> _Fmt, _Types &&..._Args)
{
	return getSpriteFrame(fmt::format(_Fmt, _Args...));
}

#endif
