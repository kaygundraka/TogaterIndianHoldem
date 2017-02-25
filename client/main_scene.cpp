#include "main_scene.h"
#include "SimpleAudioEngine.h"
#include "ui\UILayout.h"
#include "ui\UITextField.h"
#include "ui\UIButton.h"
#include "holdem_card.h"
#include "game_manager.h"
#include "network_manager.h"
#include "chat_session.h"
#include "logic_session.h"
#include "channel_session.h"
#include <SimpleAudioEngine.h>

USING_NS_CC;

ui::TextField* room_chat_field;

Scene* main_scene::createScene()
{
    auto scene_ = Scene::create();

    auto layer = main_scene::create();

    scene_->addChild(layer);

    return scene_;
}

bool main_scene::init()
{
    if (!Layer::init())
        return false;

#pragma region Init_UI
    if (game_mgr->bgm_)
        CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("game_bgm.mp3", true);
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    Vec2 middle_pos(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);

    auto back = ui::Button::create("button2_normal.png", "button2_pressed.png");

    back->setTitleText("Give Up");
    back->setTitleFontName("fonts/D2Coding.ttf");
    back->setTitleFontSize(20);
    back->setTitleColor(Color3B::BLACK);
    back->setScale(1.0f, 1.0);
    back->setAnchorPoint(Vec2(0.5, 0.5));
    back->setPosition(Vec2(visibleSize.width - 100, 100));

    auto background = Sprite::create("background.jpg");
    background->setScale(0.8f);
    background->setPosition(middle_pos);
    this->addChild(background, 0);

    auto label_info_1 = cocos2d::Label::createWithTTF("Public Card", "fonts/arial.ttf", 10);
    label_info_1->setColor(cocos2d::Color3B::WHITE);
    label_info_1->setPosition(middle_pos + cocos2d::Vec2(-180, -35));
    this->addChild(label_info_1, 1);

    auto label_info_2 = cocos2d::Label::createWithTTF("Public Card", "fonts/D2Coding.ttf", 10);
    label_info_2->setColor(cocos2d::Color3B::WHITE);
    label_info_2->setPosition(middle_pos + cocos2d::Vec2(-80, -35));
    this->addChild(label_info_2, 1);

    auto label_info_3 = cocos2d::Label::createWithTTF("Opponent Card", "fonts/D2Coding.ttf", 10);
    label_info_3->setColor(cocos2d::Color3B::WHITE);
    label_info_3->setPosition(middle_pos + cocos2d::Vec2(-130, 85));
    this->addChild(label_info_3, 1);

    auto bet_coin_user = cocos2d::Label::createWithTTF("Bet : 0", "fonts/D2Coding.ttf", 17);
    bet_coin_user->setColor(cocos2d::Color3B::ORANGE);
    bet_coin_user->setPosition(cocos2d::Vec2(visibleSize.width - 100, 150));
    this->addChild(bet_coin_user, 1);

    auto opponent_info = cocos2d::Label::createWithTTF("ID : Temp\nWin : 0, Defeat : 0\nRating : 1200\nBet : 0", "fonts/D2Coding.ttf", 17);
    opponent_info->setColor(cocos2d::Color3B::RED);
    opponent_info->setPosition(cocos2d::Vec2(visibleSize.width - 100, visibleSize.height - 90));
    this->addChild(opponent_info, 1);

    auto card_pack = Sprite::create("card_pack.png");
    card_pack->setScale(1.0f);
    card_pack->setPosition(middle_pos - Vec2(150, 110));
    this->addChild(card_pack, 1);

    auto label_card_pack = cocos2d::Label::createWithTTF("Server\nCamp", "fonts/D2Coding.ttf", 15, Size::ZERO, cocos2d::TextHAlignment::CENTER);
    label_card_pack->setColor(cocos2d::Color3B::BLACK);
    label_card_pack->setPosition(middle_pos - Vec2(155, 110));
    this->addChild(label_card_pack, 2);

    auto chat_background = Sprite::create("in_game_chat_frame.png");
    chat_background->setAnchorPoint(Vec2(0, 0));
    chat_background->setPosition(Vec2(0, 0));
    this->addChild(chat_background, 4);

    room_chat_field = ui::TextField::create("Input Chat Here", "fonts/D2Coding.ttf", 15);
    room_chat_field->setMaxLength(15);
    room_chat_field->setColor(cocos2d::Color3B::BLACK);
    room_chat_field->setMaxLength(true);
    room_chat_field->setAnchorPoint(Vec2(0, 0));
    room_chat_field->setPosition(Vec2(9, 5));
    //room_chat_field->setCursorEnabled(true);

    this->addChild(room_chat_field, 5);

    auto chat_list = ui::ListView::create();
    chat_list->setDirection(ui::ListView::Direction::VERTICAL);
    chat_list->setClippingEnabled(true);
    chat_list->setTouchEnabled(true);
    chat_list->setContentSize(Size(130, 400));
    chat_list->setAnchorPoint(Vec2(0, 1));
    chat_list->setBounceEnabled(false);
    chat_list->setScrollBarEnabled(true);
    chat_list->setScrollBarPositionFromCorner(Vec2(0, 0));
    chat_list->setItemsMargin(2.0f);
    chat_list->setPosition(Vec2(15, visibleSize.height - 50));
    this->addChild(chat_list, 5);

    auto chat_button = ui::Button::create("button3_normal.png", "button3_pressed.png");

    chat_button->setTitleText("");
    chat_button->setTitleFontName("fonts/D2Coding.ttf");
    chat_button->setTitleFontSize(20);
    chat_button->setTitleColor(Color3B::BLACK);
    chat_button->setScale(0.6f, 0.6f);
    chat_button->setAnchorPoint(Vec2(0.5, 0.5));
    chat_button->setPosition(Vec2(143, 14));

    auto bet_button = ui::Button::create("bet_button_normal.png", "bet_button_pressed.png", "bet_button_disable.png");
    bet_button->setTitleText("BET");
    bet_button->setTitleFontSize(20);
    bet_button->setEnabled(false);
    bet_button->setPosition(Vec2(visibleSize.width - 100, visibleSize.height / 2));

#pragma endregion

#pragma region Listener Event Settings
    auto listener = EventListenerTouchOneByOne::create();

    listener->setSwallowTouches(true);

    listener->onTouchBegan = CC_CALLBACK_2(main_scene::on_touch_begin, this);
    listener->onTouchMoved = CC_CALLBACK_2(main_scene::on_touch_moved, this);
    listener->onTouchCancelled = CC_CALLBACK_2(main_scene::on_touch_cancelled, this);
    listener->onTouchEnded = CC_CALLBACK_2(main_scene::on_touch_ended, this);

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    bet_button->addTouchEventListener([&](Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
        switch (type)
        {
        case ui::Widget::TouchEventType::ENDED:
            if (is_end == false)
                game_mgr->betting();
            break;
        }
    });

    this->addChild(bet_button);

    chat_button->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type) {
        switch (type)
        {
        case ui::Widget::TouchEventType::ENDED:

            std::string chat_str = room_chat_field->getString();
            std::string target_id = "";
            std::string real_str = "";

            bool is_whisper = false;

            if (chat_str.find("/w ") != std::string::npos)
            {
                for (int j = 3; j < chat_str.size(); j++)
                {
                    if (chat_str[j] != ' ')
                        target_id += chat_str[j];
                    else
                    {
                        is_whisper = true;
                        break;
                    }
                }
            }

            if (is_whisper)
            {
                network_chat->send_packet_chat_whisper(
                    network_mgr->get_player_id(),
                    target_id,
                    real_str
                );
            }
            else
            {
                network_chat->send_packet_chat_room(
                    network_mgr->get_player_id(),
                    chat_str
                );
            }

            room_chat_field->setText("");
            break;
        }
    });

    this->addChild(chat_button, 6);

    back->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type) {
        switch (type)
        {
        case ui::Widget::TouchEventType::ENDED:
            if (is_end == false)
                network_logic->send_packet_disconnect_room_ntf();
            break;
        }
    });

    auto keyboard_listener = EventListenerKeyboard::create();
    keyboard_listener->onKeyPressed = CC_CALLBACK_2(main_scene::on_key_pressed, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboard_listener, this);

    this->addChild(back, 1);
#pragma endregion

#pragma region GameManager Member Settings
    is_end = false;
    end_timer = 0;

    game_mgr->scene_ = this;
    game_mgr->scheduler_[game_manager::ROOM] = this->getScheduler();

    game_mgr->room_chat_list_ = chat_list;
    game_mgr->set_scene_status(game_manager::SCENE_TYPE::ROOM);

    game_mgr->user_ = new player();
    game_mgr->opponent_ = new player();

    game_mgr->user_->init(true);
    game_mgr->opponent_->init(false);

    game_mgr->bet_button_ = bet_button;

    game_mgr->user_bet_text_ = bet_coin_user;
    game_mgr->opponent_info_text_ = opponent_info;

    opponent_info->setString(game_mgr->opponent_info_);

    network_logic->create();
    network_logic->connect(LOGIC_SERVER_IP, LOGIC_SERVER_PORT);

    network_logic->send_packet_enter_req(network_mgr->get_room_key(), network_mgr->get_player_key());
#pragma endregion 

    scheduleUpdate();

    return true;
}

void main_scene::update(float delta)
{
    if (is_end)
    {
        if (end_timer < 10)
            end_timer += delta;

        if (end_timer > 10)
        {
            this->end();
            end_timer = 10;
        }
    }
}

void main_scene::show_result(std::string msg)
{
    auto end_info = cocos2d::Label::createWithTTF(msg, "fonts/D2Coding.ttf", 30);
    end_info->enableShadow();
    end_info->setHorizontalAlignment(cocos2d::TextHAlignment::CENTER);
    end_info->setColor(cocos2d::Color3B::RED);
    end_info->setPosition(cocos2d::Vec2(400, 240));
    this->addChild(end_info, 10);

    is_end = true;
}

void main_scene::end()
{
    if (game_mgr->bgm_)
        CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("main_bgm.mp3", true);

    game_mgr->send_friend_match_ = false;
    game_mgr->accept_friend_match_ = false;
    game_mgr->friend_list_->removeAllItems();

    network_logic->destroy();
    network_lobby->create();

    game_mgr->release_singleton();

    if (!network_lobby->is_run())
    {
        this->getScheduler()->performFunctionInCocosThread(
            CC_CALLBACK_0(
                channel_session::connect,
                network_lobby,
                CHANNEL_SERVER_IP, CHANNEL_SEFVER_PORT
            )
        );

        this->getScheduler()->performFunctionInCocosThread(
            CC_CALLBACK_0(
                channel_session::send_packet_join_req,
                network_lobby,
                network_mgr->get_player_key(),
                network_mgr->get_player_id()
            )
        );
    }

    this->getScheduler()->performFunctionInCocosThread(
        CC_CALLBACK_0(
            chat_session::send_packet_leave_match_ntf,
            network_chat
        )
    );

    is_end = false;
    end_timer = 0;
  
    Director::getInstance()->popScene();
    Director::getInstance()->popScene();

    game_mgr->set_scene_status(game_manager::SCENE_TYPE::LOBBY);
}

void main_scene::on_key_pressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event)
{
    if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ENTER)
    {
        if (room_chat_field->isFocused())
        {
            std::string chat_str = room_chat_field->getString();
            std::string target_id = "";
            std::string real_str = "";

            bool is_whisper = false;

            if (chat_str.find("/w ") != std::string::npos)
            {
                for (int j = 3; j < chat_str.size(); j++)
                {
                    if (chat_str[j] != ' ')
                        target_id += chat_str[j];
                    else
                    {
                        real_str = chat_str.substr(j + 1, chat_str.size() - j - 1);
                        is_whisper = true;
                        break;
                    }
                }
            }

            if (is_whisper)
            {
                network_chat->send_packet_chat_whisper(
                    network_mgr->get_player_id(),
                    target_id,
                    real_str
                );
            }
            else
            {
                network_chat->send_packet_chat_room(
                    network_mgr->get_player_id(),
                    chat_str
                );
            }

            room_chat_field->setText("");
        }
    }
}

void main_scene::on_touch_ended(cocos2d::Touch* touch, Event *unused_event)
{
    auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
    cocos2d::Vec2 origin = cocos2d::Director::getInstance()->getVisibleOrigin();

    cocos2d::Vec2 middle_pos(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);
    middle_pos += cocos2d::Vec2(100 + rand() % 100, rand() % 100);

    bool move_coin = false;

    if (touch->getLocation().x >= 210 && touch->getLocation().x <= 220 + game_mgr->user_->get_coin_size() * 10)
    {
        if (touch->getLocation().y >= 0 && touch->getLocation().y <= 60)
        {
            game_mgr->user_->bet_coin();

            move_coin = true;
        }
    }

    if (touch->getLocation().x >= middle_pos.x - 100 && touch->getLocation().x <= middle_pos.x + 100)
    {
        if (touch->getLocation().y >= middle_pos.y - 100 && touch->getLocation().y <= middle_pos.y + 100)
        {
            game_mgr->user_->restore_coin();

            move_coin = true;
        }
    }

    if (move_coin)
    {
        int bet_size = game_mgr->user_->get_bet_coin_size();
        char temp[20] = "";
        sprintf(temp, "Bet : %d", bet_size);
        
        std::string bet_text = temp;
        game_mgr->user_bet_text_->setString(bet_text);
    }
}

void main_scene::menu_close_callback(Ref* sender)
{
    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

bool main_scene::on_touch_begin(cocos2d::Touch *touch, cocos2d::Event *unused_event) { return true; }
void main_scene::on_touch_moved(cocos2d::Touch *touch, cocos2d::Event *unused_event) {}
void main_scene::on_touch_cancelled(cocos2d::Touch *touch, cocos2d::Event *unused_event) {}