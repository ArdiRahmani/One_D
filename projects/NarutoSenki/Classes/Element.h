#pragma once
#include "CharacterBase.h"
#include "GameLayer.h"
#include "HPBar.h"

class Monster : public CharacterBase
{
public:
	Monster();
	~Monster();

	bool init();
	void initAction();
	void setID(const string &name, Role role, Group group);
	void setHPbar();
	void changeHPbar();
	void setDirectMove(int length, float delay, bool isReverse);
	void setEaseIn(int length, float delay);
	void setDirectMoveBy(int length, float delay);

	CREATE_FUNC(Monster);

	// attackType dari action <Idle> di Unit/Mon/<nama>.xml (mis. "Jibaku", "Snake").
	// Dipakai sebagai nama AI (lihat MonXmlAI). Beda dengan _nAttackType (action nAttack, "aAttack").
	const string &getXmlAIType() const { return _xmlAIType; }

	// Array aksi tambahan untuk <e type="setContinue"> (Spc1 / Spc2 / Skill3).
	// Hanya terisi kalau Unit/Mon/<nama>.xml punya aksi tersebut (indeks 10 / 17 / 18).
	CCArray *getContinueArray(const string &actionName) override;

protected:
	// Indeks: 0-4 = skill1..skill5 (XML 8..12), 5-9 = spc1..spc5 (XML 17,18,19,16,15)
	CCArray *_contArrays[10] = {};
	string _xmlAIType;
	void dealloc();
	void setAI(float dt);
	void setResume();
};

#include "Core/Tower/Tower.hpp"

#include "Core/Warrior/Flog.hpp"

#include "Core/Projectile/Bullet.hpp"
