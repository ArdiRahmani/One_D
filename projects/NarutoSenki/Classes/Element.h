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

protected:
	string _xmlAIType;
	void dealloc();
	void setAI(float dt);
	void setResume();
};

#include "Core/Tower/Tower.hpp"

#include "Core/Warrior/Flog.hpp"

#include "Core/Projectile/Bullet.hpp"
