#include "holdem_card.h"
#include "game_manager.h"

holdem_card::holdem_card(int layer, int num, bool is_rotated, std::string info)
{
    //thread_sync sync;

    is_rotated_ = is_rotated;

    num_ = num;

    auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
    cocos2d::Vec2 origin = cocos2d::Director::getInstance()->getVisibleOrigin();

    cocos2d::Vec2 middle_pos(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);

    pivot_ = middle_pos - cocos2d::Vec2(150, 150);
    
    sprite_ = cocos2d::Sprite::create("card_frame.png");

    sprite_->setScale(0.8f);
    sprite_->setRotation(-20);

    game_mgr->scene_->addChild(sprite_, 0);
    sprite_->setPosition(pivot_);

    char temp[3] = "";
    sprintf(temp, "%d", num_);
    std::string number = temp;
    
    label_num_ = cocos2d::Label::createWithTTF(number, "fonts/arial.ttf", 30, cocos2d::Size::ZERO, cocos2d::TextHAlignment::CENTER);
    label_num_->setColor(cocos2d::Color3B::BLACK);
    label_num_->setPosition(pivot_);
    label_num_->setRotation(-20);

    int y = 0;

    if (is_rotated_)
    {
        auto hide_action = cocos2d::Hide::create();
        label_num_->runAction(hide_action);
       
        y = 180;
    }

    game_mgr->scene_->addChild(label_num_, layer);
    
    auto rotate_action = cocos2d::RotateTo::create(2, cocos2d::Vec3(0, y, 0));
    sprite_->runAction(rotate_action);

    auto rotate_action_2 = cocos2d::RotateTo::create(2, cocos2d::Vec3(0, y, 0));
    label_num_->runAction(rotate_action_2);
}

void holdem_card::show()
{
    auto rotate_action = cocos2d::RotateBy::create(2, cocos2d::Vec3(0, 180, 0));
    sprite_->runAction(rotate_action);

    auto show_action = cocos2d::Show::create();
    label_num_->runAction(show_action);

    auto rotate_action_2 = cocos2d::RotateBy::create(2, cocos2d::Vec3(0, 180, 0));
    label_num_->runAction(rotate_action_2);
}

void holdem_card::move_action(cocos2d::Vec2 destination, float time)
{
    destination -= pivot_;

    auto sprite_action = cocos2d::MoveBy::create(time, destination);
    auto sprite_action2 = cocos2d::EaseSineInOut::create(sprite_action);
    sprite_->runAction(sprite_action2);

    auto label_num_action = cocos2d::MoveBy::create(time, destination);
    auto label_num_action2 = cocos2d::EaseSineInOut::create(label_num_action);
    label_num_->runAction(label_num_action2);
}

void holdem_card::release()
{
    cocos2d::Vec2 destination(-100, 240);

    auto sprite_action = cocos2d::MoveTo::create(1, destination);
    auto sprite_action2 = cocos2d::EaseSineInOut::create(sprite_action);
    sprite_->runAction(sprite_action2);

    auto label_num_action = cocos2d::MoveTo::create(1, destination);
    auto label_num_action2 = cocos2d::EaseSineInOut::create(label_num_action);
    label_num_->runAction(label_num_action2);
}