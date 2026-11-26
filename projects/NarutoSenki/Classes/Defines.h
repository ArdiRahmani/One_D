#pragma once
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include "cocos2d.h"
#include "SimpleAudioEngine.h"

#include "Data/Types.h"
#include "Data/UnitDefines.h"
#include "Data/Version.h"
#include "Enums/Audio.hpp"
#include "Enums/Character.h"
#include "Utils/Utils.h"

using namespace cocos2d;
using namespace CocosDenshion;
using namespace std;

#define winSize Director::sharedDirector()->getWinSize()
#define random(x) (rand() % x)
// Seed hanya sekali. Sebelumnya srand(time(0)) dipanggil di setiap random, sehingga
// beberapa random dalam detik yang sama menghasilkan angka yang sama.
inline void seedRandOnce()
{
	static const bool seeded = (srand((uint32_t)time(0)), true);
	(void)seeded;
}
#define setRand() seedRandOnce()

#define is_same(__Str1, __Str2) (strcmp(__Str1, __Str2) == 0)
#define to_cstr(var) (std::to_string(var).c_str())
