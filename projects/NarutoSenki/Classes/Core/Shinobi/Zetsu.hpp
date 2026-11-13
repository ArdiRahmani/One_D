#pragma once
#include "Hero.hpp"
#include "Kuchiyose/WhiteZetsu.hpp"

class Zetsu : public Hero
{
	void perform() override
	{
	if (getName() == HeroEnum::Zetsu2)
    {
        perform_Zetsu2();
        return;
    }
		_mainTarget = nullptr;
		findHeroHalf();

		tryBuyGear(GearType::Gear00, GearType::Gear04, GearType::Gear08);

		if (needBackToTowerToRestoreHP())
			return;

		if (_mainTarget && (battleCondiction >= 0 || _isCanOugis1 || _isCanOugis2))
		{
			Vec2 moveDirection;
			Vec2 sp = getDistanceToTarget();

			if (isFreeState())
			{
				if (_isCanOugis2 && !_isControlled && getGameLayer()->_isOugis2Game && !_powerUPBuffValue && _mainTarget->getDEF() < 5000 && !_mainTarget->_isArmored && _mainTarget->getState() != State::KNOCKDOWN && !_mainTarget->_isSticking)
				{
					if (abs(sp.x) > 32 || abs(sp.y) > 32)
					{
						if (_isCanGear00)
							useGear(GearType::Gear00);

						moveDirection = sp.getNormalized();
						walk(moveDirection);
						return;
					}

					changeSide(sp);
					attack(OUGIS2);
					return;
				}
				else if (_isCanOugis1 && !_isControlled && _mainTarget->getDEF() < 5000 && !_powerUPBuffValue)
				{
					changeSide(sp);
					attack(OUGIS1);
					return;
				}
				else if (_isCanSkill1 && _isArmored)
				{
					attack(SKILL1);
					return;
				}
				else if (enemyCombatPoint > friendCombatPoint && abs(enemyCombatPoint - friendCombatPoint) > 3000 && !_isHealing && !_isControlled)
				{
					if (abs(sp.x) < 160)
						stepBack2();
					else
						idle();
					return;
				}
				else if (abs(sp.x) < 128)
				{
					if (abs(sp.x) > 32 || abs(sp.y) > 32)
					{
						if (_isCanGear00)
							useGear(GearType::Gear00);

						moveDirection = sp.getNormalized();
						walk(moveDirection);
						return;
					}

					if (_isCanSkill3 && !_powerUPBuffValue && !_isArmored && getHpPercent() > 0.5f)
					{
						changeSide(sp);
						attack(SKILL3);
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

		if (battleCondiction >= 0)
		{
			_mainTarget = nullptr;
			if (notFindFlogHalf())
			{
				findTowerHalf();
			}
		}
		else
		{
			_mainTarget = nullptr;
			findTowerHalf();
		}

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
				if (_isCanSkill1 && _isArmored)
				{
					attack(SKILL1);
				}
				else if (_mainTarget->isTower() && _isCanSkill3 && !_powerUPBuffValue && !_isArmored)
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

    void perform_Zetsu2()
	{
		_mainTarget = nullptr;
		findHeroHalf();

		tryUseGear6();
		tryBuyGear(GearType::Gear00, GearType::Gear08, GearType::Gear02);

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
					if (abs(sp.x) > 96 || abs(sp.y) > 16)
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
				else if (_isCanSkill1 && _mainTarget->getDEF() < 5000)
				{
					if ((abs(sp.x) > 128 || abs(sp.y) > 32))
					{
						moveDirection = sp.getNormalized();
						walk(moveDirection);
						return;
					}
					changeSide(sp);
					attack(SKILL1);
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

				if (_isCanSkill2 && _mainTarget->getDEF() < 5000)
				{
					if (abs(sp.x) > 96 || abs(sp.y) > 16)
					{
						moveDirection = sp.getNormalized();
						walk(moveDirection);
						return;
					}
					changeSide(sp);
					attack(SKILL2);
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
	if (getName() == HeroEnum::Zetsu)
		{
		if (_skillChangeBuffValue == 17)
		{
			_isArmored = false;

			if (isPlayer())
			{
				getGameLayer()->getHudLayer()->skill1Button->setLock();
				getGameLayer()->getHudLayer()->skill2Button->unLock();
				getGameLayer()->getHudLayer()->skill3Button->unLock();
			}

			setIdleAction(createAnimation(skillSPC1Array, 5, true, false));
			setWalkAction(createAnimation(skillSPC2Array, 10, true, false));
			setNAttackAction(createAnimation(skillSPC3Array, 10, false, true));
			if (_hpBar)
			{
				_hpBar->setPositionY(65);
			}
			_skillChangeBuffValue = 0;
		}
		else if (_skillChangeBuffValue == 18)
		{
			for (auto mo : _monsterArray)
			{
				if (mo->getName() == SummonEnum::Akamaru)
				{
					mo->attack(SKILL2);
				}
			}

			_powerUPBuffValue = 360;
			setNAttackValue(getNAttackValue() + _powerUPBuffValue);
			setNAttackAction(createAnimation(skillSPC4Array, 10, false, true));
		}
	}
}

	void resumeAction(float dt) override
	{
	if (getName() == HeroEnum::Zetsu)
		{
		if (_powerUPBuffValue)
		{
			setNAttackValue(getNAttackValue() - _powerUPBuffValue);
			setNAttackAction(createAnimation(skillSPC3Array, 10, false, true));
			_powerUPBuffValue = 0;
		}

		if (isFreeState())
		{
			_state = State::WALK;
			idle();
		}
		CharacterBase::resumeAction(dt);
	}
}
	void setActionResume() override
	{
	if (getName() == HeroEnum::Zetsu)
		{
		if (_powerUPBuffValue)
		{
			unschedule(schedule_selector(Zetsu::resumeAction));
			setNAttackValue(getNAttackValue() - _powerUPBuffValue);
			_powerUPBuffValue = 0;
		}
		_isArmored = true;
		setIdleAction(createAnimation(idleArray, 5, true, false));
		setWalkAction(createAnimation(walkArray, 10, true, false));
		setNAttackAction(createAnimation(nattackArray, 10, false, true));

		if (isPlayer())
		{
			getGameLayer()->getHudLayer()->skill1Button->unLock();
			getGameLayer()->getHudLayer()->skill2Button->setLock();
			getGameLayer()->getHudLayer()->skill3Button->setLock();
		}

		if (_hpBar)
		{
			_hpBar->setPositionY(getHeight());
		}

		if (isFreeState())
		{
			_state = State::WALK;
			idle();
		}

		_skillChangeBuffValue = 0;
	}
}

	Hero *createClone(int cloneTime) override
{
    if (getName() == HeroEnum::Zetsu)
    {
        auto clone = createCloneHero<WhiteZetsu>(SummonEnum::WhiteZetsu);
        clone->_isArmored = true;
        _monsterArray.push_back(clone);
        return clone;
    }
}

private:
	void tryLockSkillButton()
	{
		if (getName() == HeroEnum::Zetsu)
		{
		if (isPlayer())
		{
			getGameLayer()->getHudLayer()->skill2Button->setLock();
			getGameLayer()->getHudLayer()->skill3Button->setLock();
	}
   }
 }
};
