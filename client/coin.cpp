#include "coin.h"
#include "game_manager.h"

coin::coin(int layer) : layer_(layer)
{
    sprite_ = cocos2d::Sprite::create("coin.png");
    auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
    sprite_->setPosition(cocos2d::Vec2(visibleSize.width / 2, visibleSize.height / 2));
    game_mgr->scene_->addChild(sprite_, layer_);
}

coin::~coin()
{
}

int coin::get_layer()
{
    return layer_;
}

void coin::touch()
{
    if (owner_ == USER)
    {
        if (is_betting)
            game_mgr->user_->restore_coin();
        else
            game_mgr->user_->bet_coin();
    }
    else
    {
        if (is_betting)
            game_mgr->user_->restore_coin();
        else
            game_mgr->user_->bet_coin();
    }
}

void coin::set_z_order(int i)
{
    sprite_->setLocalZOrder(i);
}

void coin::bet(bool is_betting)
{
    this->is_betting = is_betting;
}

void coin::set_player(OWNER type)
{
    owner_ = type;
}

cocos2d::Vec2 coin::get_position()
{
    return position;
}

void coin::move_by(cocos2d::Vec2 destination, float time)
{
    auto sprite_action = cocos2d::MoveBy::create(time, destination);
    auto sprite_action2 = cocos2d::EaseSineInOut::create(sprite_action);
    sprite_->runAction(sprite_action2);

    position = destination;
}

void coin::move_to(cocos2d::Vec2 destination, float time)
{
    sprite_->cleanup();

    auto sprite_action = cocos2d::MoveTo::create(time, destination);
    auto sprite_action2 = cocos2d::EaseSineInOut::create(sprite_action);
    
    sprite_->runAction(sprite_action2);

    position = destination;
}