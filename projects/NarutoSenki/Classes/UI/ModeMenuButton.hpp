#pragma once
#include "UI/GameModeLayer.h"

class ModeMenuButton : public Sprite, public CCTouchDelegate
{
private:
	Sprite *lockMask = nullptr;

public:
	PROP(GameModeLayer *, _gameModeLayer, Delegate);

	GameMode mode;
	bool useMask2;

	bool init(const string &szImage)
	{
		RETURN_FALSE_IF(!Sprite::initWithFile(szImage.c_str()));
		// initWithSpriteFrameName(szImage);
		setAnchorPoint(Vec2(0.5, 0.5));

		return true;
	}

	void onEnter()
	{
		Sprite::onEnter();
		Director::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, 10, true);
	}

	void onExit()
	{
		Sprite::onExit();
		Director::sharedDirector()->getTouchDispatcher()->removeDelegate(this);
	}

	CCRect getRect()
	{
		CCSize size = getContentSize();
		return CCRect(0, 0, size.width, size.height);
	}

	bool ccTouchBegan(Touch *touch, Event *event)
	{
		if (!isVisible())
			return false;
		return getRect().containsPoint(convertTouchToNodeSpace(touch));
	}

	void ccTouchEnded(Touch *touch, Event *event)
	{
		SimpleAudioEngine::sharedEngine()->playEffect("Audio/Menu/confirm.ogg");
		_gameModeLayer->selectMode(mode);
	}

	bool isLocked()
	{
		return lockMask != nullptr;
	}

	void lock()
	{
		if (lockMask == nullptr)
		{
			auto maskPath = useMask2 ? "GameMode/chain_mask2.png" : "GameMode/chain_mask.png";
			lockMask = Sprite::create(maskPath);
			// child dari tombol, jadi ikut tampil/sembunyi saat ganti halaman
			lockMask->setPosition(Vec2(getContentSize().width / 2, getContentSize().height / 2));
			addChild(lockMask, 1);
		}
	}

	void unlock()
	{
		if (lockMask)
		{
			removeChild(lockMask, true);
			lockMask = nullptr;
		}
	}

	static ModeMenuButton *create(const string &szImage)
	{
		ModeMenuButton *mb = new ModeMenuButton();
		if (mb && mb->init(szImage))
		{
			mb->autorelease();
			return mb;
		}
		else
		{
			delete mb;
			return nullptr;
		}
	}
};
