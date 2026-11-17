#pragma once
#include "Hero.hpp"

class Obito : public Hero
{
	// Obito

	void perform() override
	{
	if (getName() == HeroEnum::ObitoRikudo)
    {
        perform_RikudoObito();
        return;
    }
		_mainTarget = nullptr;
		findHeroHalf();

		tryUseGear6();
		tryBuyGear(GearType::Gear06, GearType::Gear08, GearType::Gear01);

		if (needBackToTowerToRestoreHP() ||
			needBackToDefendTower())
			return;

		if (_mainTarget && _mainTarget->isNotFlog())
		{
			Vec2 moveDirection;
			Vec2 sp = getDistanceToTarget();

			if (isFreeState())
			{
				if (_isCanOugis2 && !_isControlled && getGameLayer()->_isOugis2Game && !_isArmored)
				{
					changeSide(sp);
					attack(OUGIS2);
					return;
				}
				else if (enemyCombatPoint > friendCombatPoint && abs(enemyCombatPoint - friendCombatPoint) > 3000 && !_isHealing && !_isArmored && !_isControlled)
				{
					if (abs(sp.x) < 160)
						stepBack2();
					else
						idle();
					return;
				}
				else if (_isCanSkill3 && !_isTaunt && !_isArmored)
				{
					changeSide(sp);
					attack(SKILL3);
					return;
				}
				else if (abs(sp.x) < 128)
				{
					if (abs(sp.x) > 96 || abs(sp.y) > 32)
					{
						moveDirection = sp.getNormalized();
						walk(moveDirection);
						return;
					}
					else if ((abs(sp.x) > 32 || abs(sp.y) > 32) && !_isCanSkill1 && !_isCanSkill2)
					{
						moveDirection = sp.getNormalized();
						walk(moveDirection);
						return;
					}

					if (_isCanOugis1 && !_isControlled && _mainTarget->getDEF() < 5000 && !_isArmored)
					{
						if (abs(sp.x) > 32 || abs(sp.y) > 32)
						{
							moveDirection = sp.getNormalized();
							walk(moveDirection);
							return;
						}
						changeSide(sp);
						attack(OUGIS1);
					}
					else if (_isCanSkill2 && !_isArmored && _mainTarget->getDEF() < 5000)
					{
						changeSide(sp);
						attack(SKILL2);
					}
					else if (_isCanSkill1 && !_isArmored && _mainTarget->getDEF() < 5000)
					{
						changeSide(sp);
						attack(SKILL1);
					}
					else
					{
						changeSide(sp);
						attack(NAttack);
					}

					return;
				}
			}
		}
		_mainTarget = nullptr;
		if (notFindFlogHalf())
			findTowerHalf();

		if (_mainTarget)
		{
			Vec2 moveDirection;
			Vec2 sp = getDistanceToTarget();

			if (abs(sp.x) > 32 || abs(sp.y) > 32)
			{
				moveDirection = sp.getNormalized();
				walk(moveDirection);
				return;
			}

			if (isFreeState())
			{
				if (_isCanSkill1 && !_isArmored && _mainTarget->isFlog())
				{
					changeSide(sp);
					attack(SKILL1);
				}
				else if (_isCanSkill2 && isBaseDanger && !_isArmored && _mainTarget->isFlog())
				{
					changeSide(sp);
					attack(SKILL2);
				}
				else if (_isCanSkill3 && !_isArmored && _mainTarget->isFlog())
				{
					changeSide(sp);
					attack(SKILL3);
				}
				else
				{
					changeSide(sp);
					attack(NAttack);
				}
			}

			return;
		}

		checkHealingState();
	}

	void changeAction() override
	{
	if (getName() == HeroEnum::Obito)
		{
			_isTaunt = true;
			setHurtAction(createAnimation(skillSPC5Array, 10, false, true));

		}
}
	void resumeAction() override
	{
	if (getName() == HeroEnum::Obito)
		{
		if (_isTaunt)
		{
			_isTaunt = false;
			setHurtAction(createAnimation(hurtArray, 10, false, true));
		}
		CharacterBase::resumeAction(dt);
	}
}
	void setActionResume() override
	{
	if (getName() == HeroEnum::Obito)
		{
		if (_isTaunt)
		{
			unschedule(schedule_selector(Obito::resumeAction));
			_isTaunt = false;
			setHurtAction(createAnimation(hurtArray, 10, false, true));
			_skillChangeBuffValue = 0;
		}
	}
}
	// Obito Rikudo

	void perform_RikudoObito()
{
		_mainTarget = nullptr;
		findHeroHalf();

		tryUseGear6();
		tryBuyGear(GearType::Gear06, GearType::Gear08, GearType::Gear01);

		if (needBackToTowerToRestoreHP() ||
			needBackToDefendTower())
			return;

		if (_mainTarget && _mainTarget->isNotFlog())
		{
			Vec2 moveDirection;
			Vec2 sp = getDistanceToTarget();

			if (isFreeState())
			{
				if (_isCanOugis2 && !_isControlled && getGameLayer()->_isOugis2Game)
				{
					if (abs(sp.x) > 128 || abs(sp.y) > 16)
					{
						moveDirection = sp.getNormalized();
						walk(moveDirection);
						return;
					}
					changeSide(sp);
					attack(OUGIS2);
					return;
				}
				else if (_isCanOugis1 && !_isControlled && _mainTarget->getDEF() < 5000)
				{
					if (abs(sp.x) > 48 || abs(sp.y) > 32)
					{
						moveDirection = sp.getNormalized();
						walk(moveDirection);
						return;
					}
					changeSide(sp);
					attack(OUGIS1);
					return;
				}
				else if (_isCanSkill2 && _mainTarget->getDEF() < 5000)
				{
					if ((abs(sp.x) > 96 || abs(sp.y) > 32))
					{
						moveDirection = sp.getNormalized();
						walk(moveDirection);
						return;
					}
					changeSide(sp);
					attack(SKILL2);
					return;
				}
				else if (_isCanSkill3 && _mainTarget->getDEF() < 5000)
				{
					if ((abs(sp.x) > 96 || abs(sp.y) > 32))
					{
						moveDirection = sp.getNormalized();
						walk(moveDirection);
						return;
					}
					changeSide(sp);
					attack(SKILL3);
					return;
				}

				if (_isCanSkill1 && _mainTarget->getDEF() < 5000)
				{
					if (abs(sp.x) > 96 || abs(sp.y) > 16)
					{
						moveDirection = sp.getNormalized();
						walk(moveDirection);
						return;
					}
					changeSide(sp);
					attack(SKILL1);
					return;
				}
				else if (enemyCombatPoint > friendCombatPoint && abs(enemyCombatPoint - friendCombatPoint) > 3000 && !_isHealing && !_isControlled)
				{
					if (abs(sp.x) < 160)
					{
						stepBack2();
						return;
					}
					else
					{
						idle();
						return;
					}
				}
				else if (abs(sp.x) < 128)
				{
					if ((abs(sp.x) > 32 || abs(sp.y) > 32))
					{
						moveDirection = sp.getNormalized();
						walk(moveDirection);
						return;
					}

					if (_isCanOugis1 && !_isControlled && getHpPercent() < 0.9 && _mainTarget->getDEF() < 5000)
					{
						changeSide(sp);
						attack(OUGIS1);
					}
					else
					{
						changeSide(sp);
						attack(NAttack);
					}

					return;
				}
			}
		}

		_mainTarget = nullptr;
		if (notFindFlogHalf())
			findTowerHalf();

		if (_mainTarget)
		{
			Vec2 moveDirection;
			Vec2 sp = getDistanceToTarget();

			if (abs(sp.x) > 32 || abs(sp.y) > 32)
			{
				moveDirection = sp.getNormalized();
				walk(moveDirection);
				return;
			}

			if (isFreeState())
			{
				if (_isCanSkill2 && _mainTarget->isFlog() && isBaseDanger)
				{
					changeSide(sp);
					attack(SKILL2);
				}
				else if (_isCanSkill1 && _mainTarget->isFlog())
				{
					changeSide(sp);
					attack(SKILL1);
				}
				else
				{
					changeSide(sp);
					attack(NAttack);
				}
			}

			return;
		}

		checkHealingState();
	}
};
