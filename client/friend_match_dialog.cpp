#include "friend_match_dialog.h"
#include "game_manager.h"
#include "network_manager.h"
#include "channel_session.h"
#include <ui\UIButton.h>

using namespace cocos2d;

cocos2d::ui::Text* timer_text;
float friend_match_timer;

Scene* friend_match_dialog::createScene()
{
    auto scene = Scene::create();

    auto layer = friend_match_dialog::create();

    scene->addChild(layer);

    return scene;
}

bool friend_match_dialog::init()
{
    if (!LayerColor::initWithColor(Color4B(0, 0, 0, 0)))
        return false;

    this->setCascadeOpacityEnabled(true);

    this->setOpacity(0);
    this->runAction(Sequence::create(FadeIn::create(1.0f), NULL));

    auto winSize = CCDirector::sharedDirector()->getWinSizeInPixels();
    auto visibleSize = Director::getInstance()->getVisibleSize();

    auto background = Sprite::create("dialog_background.png");
    background->setAnchorPoint(Vec2(0, 0));
    background->setScale(1.28f);
    this->addChild(background, 0);


    auto match_title = ui::Text::create(" You've been challen\nged to a game by\n" + game_mgr->friend_match_id_ + "?", "fonts/D2Coding.ttf", 20);
    match_title->setTextHorizontalAlignment(cocos2d::TextHAlignment::CENTER);
    match_title->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 + 65));
    match_title->setTextColor(Color4B::BLACK);
    this->addChild(match_title, 1);

    timer_text = ui::Text::create("10", "fonts/D2Coding.ttf", 40);
    timer_text->setTextHorizontalAlignment(cocos2d::TextHAlignment::CENTER);
    timer_text->setPosition(Vec2(visibleSize.width / 2 + 10, visibleSize.height / 2));
    timer_text->setTextColor(Color4B::RED);
    this->addChild(timer_text, 1);
   

    auto accept_button = ui::Button::create("button2_normal.png", "button2_pressed.png");

    accept_button->setTitleText("Accept");
    accept_button->setTitleFontName("fonts/D2Coding.ttf");
    accept_button->setTitleFontSize(15);
    accept_button->setTitleColor(Color3B::RED);
    accept_button->setAnchorPoint(Vec2(0.5f, 0.5f));
    accept_button->setScale(1.0f, 1.0f);
    accept_button->setPosition(Vec2(visibleSize.width / 2 + 10, visibleSize.height / 2 - 40));

    accept_button->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type) {
        if (type == ui::Widget::TouchEventType::ENDED) {
            game_mgr->accept_friend_match_ = true;

            game_mgr->lobby_scene_->hide_friend_match_pop_up();

            game_mgr->get_scheduler()->performFunctionInCocosThread(
                CC_CALLBACK_0(
                    lobby_scene::match_game,
                    game_mgr->lobby_scene_
                )
            );
        }
    });
    
    this->addChild(accept_button, 1);


    auto refuse_button = ui::Button::create("button2_normal.png", "button2_pressed.png");

    refuse_button->setTitleText("Refuse");
    refuse_button->setTitleFontName("fonts/D2Coding.ttf");
    refuse_button->setTitleFontSize(15);
    refuse_button->setTitleColor(Color3B::BLACK);
    refuse_button->setScale(1.0f, 1.0f);
    refuse_button->setAnchorPoint(Vec2(0.5f, 0.5f));
    refuse_button->setPosition(Vec2(visibleSize.width / 2 + 10, visibleSize.height / 2 - 80));

    refuse_button->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type) {
        if (type == ui::Widget::TouchEventType::ENDED) {
            network_lobby->send_packet_play_friend_game_rel(
                channel_server::packet_play_friends_game_rel_req_type_DENY,
                game_mgr->friend_match_id_
            );

            game_mgr->lobby_scene_->hide_friend_match_pop_up();
        }
    });

    this->addChild(refuse_button, 1);

    friend_match_timer = 0;

    scheduleUpdate();

    return true;
}

void friend_match_dialog::update(float delta)
{
    if (friend_match_timer < 10)
    {
        friend_match_timer += delta;

        char temp[10] = "";
        sprintf(temp, "%d\0", (int)(10 - friend_match_timer));
        timer_text->setText(temp);
    }
    else
    {
        network_lobby->send_packet_play_friend_game_rel(
            channel_server::packet_play_friends_game_rel_req_type_DENY,
            game_mgr->friend_match_id_
        );

        game_mgr->lobby_scene_->hide_friend_match_pop_up();
    }
}