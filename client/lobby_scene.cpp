#include "lobby_scene.h"
#include <ui\UIButton.h>
#include <ui\UITextField.h>
#include <ui\UIText.h>
#include <array>
#include "main_scene.h"
#include "loading_scene.h"
#include "game_manager.h"
#include "network_manager.h"
#include "chat_session.h"
#include "channel_session.h"
#include "friend_match_dialog.h"
#include <SimpleAudioEngine.h>

using namespace cocos2d;

ui::TextField* chat_field;

Scene* lobby_scene::createScene()
{
    auto scene = Scene::create();

    auto layer = lobby_scene::create();

    scene->addChild(layer);

    return scene;
}

bool lobby_scene::init()
{
#pragma region Init UI
    //CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("main_bgm.mp3", true);

    if (!LayerColor::initWithColor(Color4B(0, 255, 0, 255)))
        return false;

    auto winSize = CCDirector::sharedDirector()->getWinSizeInPixels();
    auto visibleSize = Director::getInstance()->getVisibleSize();

    auto background = Sprite::create("lobby_background.png");
    background->setAnchorPoint(Vec2(0, 0));
    background->setScale(1.28f);
    this->addChild(background, 0);

    auto back = ui::Button::create("button2_normal.png", "button2_pressed.png");

    back->setTitleText("Back");
    back->setTitleFontName("fonts/D2Coding.ttf");
    back->setTitleFontSize(20);
    back->setTitleColor(Color3B::BLACK);
    back->setScale(1.62f, 1.9f);
    back->setAnchorPoint(Vec2(0, 1));
    back->setPosition(Vec2(25, visibleSize.height - 20));
    
    auto rank_match = ui::Button::create("button2_normal.png", "button2_pressed.png");

    rank_match->setTitleText("Rank Match");
    rank_match->setTitleFontName("fonts/D2Coding.ttf");
    rank_match->setTitleFontSize(20);
    rank_match->setTitleColor(Color3B::BLACK);
    rank_match->setScale(1.62f, 1.9f);
    rank_match->setAnchorPoint(Vec2(0, 1));
    rank_match->setPosition(Vec2(280, visibleSize.height - 20));
    
    auto add_friend = ui::Button::create("button_normal.png", "button_pressed.png");

    add_friend->setTitleText("Add");
    add_friend->setTitleFontName("fonts/D2Coding.ttf");
    add_friend->setTitleFontSize(20);
    add_friend->setTitleColor(Color3B::BLACK);
    add_friend->setScale(0.74f, 0.8f);
    add_friend->setAnchorPoint(Vec2(1, 1));
    add_friend->setPosition(Vec2(visibleSize.width - 146, 165));
    
    auto remove_friend = ui::Button::create("button_normal.png", "button_pressed.png");

    remove_friend->setTitleText("Remove");
    remove_friend->setTitleFontName("fonts/D2Coding.ttf");
    remove_friend->setTitleFontSize(20);
    remove_friend->setTitleColor(Color3B::BLACK);
    remove_friend->setScale(0.74f, 0.8f);
    remove_friend->setAnchorPoint(Vec2(1, 1));
    remove_friend->setPosition(Vec2(visibleSize.width - 33, 165));
    
    auto friend_match = ui::Button::create("button_normal.png", "button_pressed.png");

    friend_match->setTitleText("Friend Match");
    friend_match->setTitleFontName("fonts/D2Coding.ttf");
    friend_match->setTitleFontSize(20);
    friend_match->setTitleColor(Color3B::BLACK);
    friend_match->setScale(1.5f, 1.5f);
    friend_match->setAnchorPoint(Vec2(1, 1));
    friend_match->setPosition(Vec2(visibleSize.width - 33, 85));
    
    auto friend_list = ui::ListView::create();
    friend_list->setDirection(ui::ListView::Direction::VERTICAL);
    friend_list->setClippingEnabled(true);
    friend_list->setTouchEnabled(true);
    friend_list->setContentSize(Size(210, 240));
    friend_list->setAnchorPoint(Vec2(0.5, 1));
    friend_list->setBounceEnabled(false);
    friend_list->setScrollBarEnabled(true);
    friend_list->setScrollBarPositionFromCorner(Vec2(1, 0.5f));
    friend_list->setItemsMargin(2.0f);

    friend_list->setPosition(Vec2(visibleSize.width/2 + 255, visibleSize.height/2 + 170));

    auto friend_search_field = ui::TextField::create("Input Friend ID", "fonts/D2Coding.ttf", 20);
    friend_search_field->setMaxLength(10);
    friend_search_field->setMaxLengthEnabled(true);
    friend_search_field->setColor(cocos2d::Color3B::BLACK);
    friend_search_field->setPosition(Vec2(visibleSize.width / 2 + 255, visibleSize.height / 2 - 132));
    //friend_search_field->setCursorEnabled(true);
    this->addChild(friend_search_field, 1);

    auto chat_list = ui::ListView::create();
    chat_list->setDirection(ui::ListView::Direction::VERTICAL);
    chat_list->setClippingEnabled(true);
    chat_list->setTouchEnabled(true);
    chat_list->setContentSize(Size(210, 260));
    chat_list->setAnchorPoint(Vec2(0.5, 1));
    chat_list->setBounceEnabled(false);
    chat_list->setScrollBarEnabled(true);
    chat_list->setScrollBarPositionFromCorner(Vec2(1, 0.5f));
    chat_list->setItemsMargin(2.0f);
    chat_list->setPosition(Vec2(145, visibleSize.height - 160));

    auto label = ui::Text::create("--Welcome Our Game--", "fonts/D2Coding.ttf", 15);
    label->setTextColor(Color4B::BLACK);
    label->setTouchEnabled(true);
    chat_list->pushBackCustomItem(label);
       
    auto chat_button = ui::Button::create("button3_normal.png", "button3_pressed.png");

    chat_button->setTitleText("");
    chat_button->setTitleFontName("fonts/D2Coding.ttf");
    chat_button->setTitleFontSize(20);
    chat_button->setTitleColor(Color3B::BLACK);
    chat_button->setScale(1.0f, 0.6f);
    chat_button->setAnchorPoint(Vec2(1, 1));
    chat_button->setPosition(Vec2(255, 46));

    chat_field = ui::TextField::create("Chat Message", "fonts/D2Coding.ttf", 15);
    chat_field->setMaxLength(20);
    chat_field->setMaxLengthEnabled(true);
    chat_field->setTextHorizontalAlignment(cocos2d::TextHAlignment::LEFT);
    chat_field->setAnchorPoint(Vec2(0, 0.5));
    chat_field->setColor(cocos2d::Color3B::BLACK);
    chat_field->setPosition(Vec2(40, 33));
    //chat_field->setCursorEnabled(true);
    
    this->addChild(chat_field, 1);
    
    auto history = ui::Text::create("", "fonts/D2Coding.ttf", 20);
    history->setTextHorizontalAlignment(cocos2d::TextHAlignment::LEFT);
    history->setPosition(Vec2(visibleSize.width / 2 - 25, visibleSize.height / 2 - 140));
    history->setTextColor(Color4B::BLACK);
    this->addChild(history, 1);

    game_mgr->history_ = history;

#pragma endregion

#pragma region Listener Event Settings

    back->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type) {
        if (type == ui::Widget::TouchEventType::ENDED)
        {
            game_mgr->send_friend_match_ = false;
            game_mgr->accept_friend_match_ = false;
            game_mgr->friend_list_->removeAllItems();
            
            if (game_mgr->rating_image != nullptr)
            {
                game_mgr->rating_image->removeFromParent();
                game_mgr->rating_image = nullptr;
            }

            network_chat->destroy();
            network_lobby->destroy();

            Director::getInstance()->popScene();
        }
    });

    rank_match->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type) {
        if (type == ui::Widget::TouchEventType::ENDED) match_game();
    });

    add_friend->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type) {
        if (type == ui::Widget::TouchEventType::ENDED) {
            channel_server::basic_info info;
            info.set_id(game_mgr->friend_text_field->getString());

            network_lobby->send_packet_friend_req(
                channel_server::packet_friends_req_req_type::packet_friends_req_req_type_SEARCH,
                info
            );
        }
    });

    remove_friend->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type) {
        if (type == ui::Widget::TouchEventType::ENDED) {
            channel_server::basic_info info;
            info.set_id(game_mgr->friend_text_field->getString());

            network_lobby->send_packet_friend_req(
                channel_server::packet_friends_req_req_type::packet_friends_req_req_type_DEL,
                info
            );
        }
    });

    friend_match->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type) {
        if (type == ui::Widget::TouchEventType::ENDED) {
            game_mgr->send_friend_match_ = true;
            match_game();
        }
    });

    friend_list->addEventListener([this](Ref* sender, ui::ListView::EventType type)
    {
        if (type == ui::ListView::EventType::ON_SELECTED_ITEM_END) {

        }
    });

    chat_button->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type) {
        switch (type)
        {
        case ui::Widget::TouchEventType::ENDED:
        {
            std::string chat_str = chat_field->getString();
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
                network_chat->send_packet_chat_normal(
                    network_mgr->get_player_id(),
                    chat_str
                );
            }

            chat_field->setText("");
        }
        break;
        }
    });

    chat_list->addEventListener((ui::ListView::ccListViewCallback)CC_CALLBACK_2(lobby_scene::chat_event_listener, this));

    this->addChild(back, 1);
    this->addChild(rank_match, 1);
    this->addChild(add_friend, 1);
    this->addChild(remove_friend, 1);
    this->addChild(friend_match, 1);
    this->addChild(friend_list, 2);
    this->addChild(chat_list, 2);
    this->addChild(chat_button, 2);

    auto keyboard_listener = EventListenerKeyboard::create();
    keyboard_listener->onKeyPressed = CC_CALLBACK_2(lobby_scene::onKeyPressed, this);
    keyboard_listener->onKeyReleased = CC_CALLBACK_2(lobby_scene::onKeyReleased, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboard_listener, this);
    
#pragma endregion

#pragma region Init Manager Members

    game_mgr->lobby_scene_ = this;
    game_mgr->scheduler_[game_manager::LOBBY] = this->getScheduler();
    
    game_mgr->lobby_chat_list_ = chat_list;
    game_mgr->friend_list_ = friend_list;
    game_mgr->friend_text_field = friend_search_field;

    game_mgr->set_scene_status(game_manager::LOBBY);

    network_chat->create();
    network_chat->connect(CHAT_SERVER_IP, CHAT_SERVER_PORT);
    network_chat->send_packet_verify_req(
        network_mgr->get_player_key(),
        network_mgr->get_player_id()
    );

    network_lobby->create();
    network_lobby->connect(CHANNEL_SERVER_IP, CHANNEL_SEFVER_PORT);
    network_lobby->send_packet_join_req(
        network_mgr->get_player_key(), 
        network_mgr->get_player_id()
    );
#pragma endregion

    return true;
}

void lobby_scene::match_game()
{
    auto scene = loading_scene::createScene();
    Director::getInstance()->pushScene(TransitionFade::create(2, scene));
}

void lobby_scene::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event)
{
    if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ENTER)
    {
        if (chat_field->isFocused())
        {
            std::string chat_str = chat_field->getString();
            std::string target_id = "";
            std::string real_str = "";

            bool is_whisper = false;
            bool is_sound_option = false;

            if (chat_str.find("/sound off") != std::string::npos)
            {
                game_mgr->bgm_ = false;
                CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic();

                is_sound_option = true;
            }
            else if (chat_str.find("/sound on") != std::string::npos)
            {
                game_mgr->bgm_ = true;
                CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("main_bgm.mp3", true);

                is_sound_option = true;
            }
            else if (chat_str.find("/w ") != std::string::npos)
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

            if (is_whisper && !is_sound_option)
            {
                network_chat->send_packet_chat_whisper(
                    network_mgr->get_player_id(),
                    target_id,
                    real_str
                );
            }
            else if (!is_sound_option)
            {
                network_chat->send_packet_chat_normal(
                    network_mgr->get_player_id(),
                    chat_str
                );
            }

            chat_field->setText("");
        }
    }
}

void lobby_scene::show_friend_match_pop_up()
{
    auto friend_match_dialog_ = friend_match_dialog::create();
    
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch *touch, Event*event)->bool {
        return true;
    };

    auto dispatcher = Director::getInstance()->getEventDispatcher();
    dispatcher->addEventListenerWithSceneGraphPriority(listener, friend_match_dialog_);

    this->addChild(friend_match_dialog_, 10);//TransitionFade::create(1, (cocos2d::Scene*)friend_match_dialog_));

    game_mgr->friend_match_dialog_ = friend_match_dialog_;
}

void lobby_scene::hide_friend_match_pop_up()
{
    this->removeChild(game_mgr->friend_match_dialog_);
}

void lobby_scene::onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event)
{
}

void lobby_scene::chat_event_listener(Ref* sender, cocos2d::ui::ListView::EventType type)
{
}