#include "game_manager.h"
#include "network_manager.h"
#include "main_scene.h"
#include "logic_session.h"
#include "ui\UIText.h"
#include "loading_scene.h"
#include "channel_session.h"
#include "chat_session.h"
#include "CCShake.h"
#include <SimpleAudioEngine.h>

bool game_manager::init_singleton()
{
    public_card_ = nullptr;
    opponent_card_ = nullptr;
    send_friend_match_ = false;
    bgm_ = true;
    accept_friend_match_ = false;
    rating_image = nullptr;

    return true;
}

bool game_manager::release_singleton()
{
    if (user_ != nullptr)
    {
        delete user_;
        user_ = nullptr;
    }

    if (opponent_ != nullptr)
    {
        delete opponent_;
        opponent_ = nullptr;
    }

    public_card_ = nullptr;
    opponent_card_ = nullptr;

    return true;
}

game_manager::game_manager()
{
    
}

void game_manager::set_opponent_info(std::string id, int win, int defeat, int rating)
{
    char temp[256] = "";

    sprintf(temp, "ID : %s\nWin : %d\nLose : %d\nRating : %d", id.c_str(), win, defeat, rating);

    opponent_id_ = id;
    opponent_info_ = temp;
}

void game_manager::new_turn(int public_card_1, int public_card_2, int opponent_card, int remain_money, int my_money, int opponent_money)
{
    if (game_mgr->bgm_)
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("turn_sound.mp3");
        
    scene_->runAction(CCShake::actionWithDuration(1.0f, 10.0f));

    auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
    cocos2d::Vec2 origin = cocos2d::Director::getInstance()->getVisibleOrigin();
    cocos2d::Vec2 middle_pos(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);

    if (remain_money > user_->get_coin_size() + user_->get_bet_coin_size())
    {
        opponent_->set_bet_coin(
            remain_money - opponent_->get_bet_coin_size() - (user_->get_bet_coin_size() + user_->get_coin_size())
        );
        user_->reset_coin(opponent_);
    }
    else if (my_money == 0 && opponent_money == 0)
        opponent_->reset_coin(user_);
   
    user_->set_lock_bet_size(my_money + 1);
    opponent_->set_lock_bet_size(opponent_money + 1);

    user_->set_bet_coin(1);
    opponent_->set_bet_coin(1);
   
    if (public_card_ != nullptr)
    {
        public_card_[0].release();
        public_card_[1].release();

        delete[] public_card_;
    }

    if (opponent_card_ != nullptr)
    {
        opponent_card_->release();

        delete opponent_card_;
    }

    public_card_ = new holdem_card[2] {
        holdem_card(2, public_card_1, true, "Public Card"),
        holdem_card(2, public_card_2, false, "Public Card")
    };

    public_card_[0].move_action(middle_pos + cocos2d::Vec2(-180, 20), 2);
    public_card_[1].move_action(middle_pos + cocos2d::Vec2(-80, 20), 2);

    opponent_card_ = new holdem_card(2, opponent_card, false, "Opponent Card");
    opponent_card_->move_action(cocos2d::Vec2(270, visibleSize.height - 100), 2);


    std::string opponent_bet_text = "\nBet : 1";
    
    opponent_info_text_->setString(opponent_info_ + opponent_bet_text);
    user_bet_text_->setString("Bet : 1");
}

void game_manager::set_tear(int rating)
{
    if (rating_image != nullptr)
    {
        rating_image->removeFromParent();
        rating_image = nullptr;
    }

    if (rating < 300)
        rating_image = cocos2d::Sprite::create("bronze.png");
    else if (rating >= 300 && rating < 400)
        rating_image = cocos2d::Sprite::create("silver.png");
    else if (rating >= 400 && rating < 500)
        rating_image = cocos2d::Sprite::create("Gold.png");
    else if (rating >= 500 && rating < 600)
        rating_image = cocos2d::Sprite::create("platinum.png");
    else if (rating >= 600 && rating < 700)
        rating_image = cocos2d::Sprite::create("diamond.png");
    else if (rating >= 700 && rating < 800)
        rating_image = cocos2d::Sprite::create("master.png");
    else if (rating >= 800 && rating < 900)
        rating_image = cocos2d::Sprite::create("GrandMaster.png");
    else if (rating >= 900)
        rating_image = cocos2d::Sprite::create("challenger.png");

    rating_image->setPosition(Vec2(400, 240 + 10));
    rating_image->setAnchorPoint(Vec2(0.5f, 0.5f));
    rating_image->setScale(1.5f, 1.5f);
    lobby_scene_->addChild(rating_image, 1);
}

void game_manager::betting()
{ 
    if (user_->get_bet_coin_size() >= opponent_->get_bet_coin_size()
        || user_->get_bet_coin_size() - user_->get_lock_bet_size() == 0 || user_->get_coin_size() == 0)
    {
        network_logic->send_packet_process_turn_ans(user_->get_bet_coin_size() - user_->get_lock_bet_size());

        user_->set_lock_bet_size(user_->get_bet_coin_size());

        bet_button_->setEnabled(false);
    }
}

void game_manager::opponent_turn_end(int my_money, int opponent_money)
{
    int size = opponent_->get_bet_coin_size();
    
    for (int i = 0; i < opponent_money - size; i++)
       opponent_->bet_coin();

    user_->set_lock_bet_size(my_money);

    bet_button_->setEnabled(true);

    char temp[20] = "";
    sprintf(temp, "\nBet : %d", opponent_->get_bet_coin_size());
    
    std::string opponent_bet_text = temp;
    
    opponent_info_text_->setString(opponent_info_ + opponent_bet_text);
}

void game_manager::check_public_card()
{
    public_card_[0].show();
}

void game_manager::start_game()
{
    auto scene = main_scene::createScene();
    
    cocos2d::Director::getInstance()->pushScene(cocos2d::TransitionFade::create(1, scene));
}

cocos2d::Scheduler* game_manager::get_scheduler()
{
    return scheduler_[(int)scene_type_];
}

void game_manager::set_scene_status(SCENE_TYPE status)
{
    scene_type_ = status;
}

void game_manager::update_chat(std::string id, std::string str, CHAT_TYPE type)
{
    std::string message = id;

    bool is_system = false;

    if (id == "SYSTEM")
        is_system = true;

    if (scene_type_ == LOBBY)
    {
        message += " : ";
        message += str;
    }
    else if (scene_type_ == ROOM)
    {
        if (id.find("[w]") != std::string::npos)
            message += " : " + str;
        else
            message = str;
    }

    auto label = cocos2d::ui::Text::create(message, "fonts/D2Coding.ttf", 15);

    if (is_system)
        label->setTextColor(cocos2d::Color4B::ORANGE);
    else if (id == network_mgr->get_player_id())
        label->setTextColor(cocos2d::Color4B::BLUE);
    else if (type == CHAT_TYPE::NORMAL)
        label->setTextColor(cocos2d::Color4B::BLACK);
    else if (type == CHAT_TYPE::WHISPER)
        label->setTextColor(cocos2d::Color4B::MAGENTA);
    else if (type == CHAT_TYPE::NOTICE)
        label->setTextColor(cocos2d::Color4B::RED);

    label->setTouchEnabled(true);

    if (scene_type_ == LOBBY)
    {
        if (this->lobby_chat_list_->getChildrenCount() > 100)
            this->lobby_chat_list_->removeAllItems();

        this->lobby_chat_list_->pushBackCustomItem(label);
        this->lobby_chat_list_->scrollToBottom(1, true);
    }
    else if (scene_type_ == ROOM)
    {
        if (this->room_chat_list_->getChildrenCount() > 100)
            this->room_chat_list_->removeAllItems();

        this->room_chat_list_->pushBackCustomItem(label);
        this->room_chat_list_->scrollToBottom(1, true);
    }
}

void game_manager::set_friend_text_field(std::string text)
{
    friend_text_field->setString(text);
}

void game_manager::add_friend_in_list(std::string id)
{
    auto label = cocos2d::ui::Text::create(id, "fonts/D2Coding.ttf", 15);
    label->setTextColor(cocos2d::Color4B::BLACK);
    label->setTouchEnabled(true);
    friend_list_->pushBackCustomItem(label);
}

void game_manager::del_friend_in_list(std::string id)
{
    auto items = friend_list_->getItems();

    int index = -1;

    for (int i = 0; i < items.size(); i++)
        if (((cocos2d::ui::Text*)items.at(i))->getString() == id)
            index = i;

    if (index != -1)
        friend_list_->removeItem(index);
}

void game_manager::set_history(int win, int lose, int rating)
{
    char temp[255] = "";
    sprintf(temp, "ID : %s\nWin : %d\nLose : %d\nTotal Game : %d\nRating : %d", network_mgr->get_player_id().c_str(),
        win, lose, win + lose, rating);

    std::string str = temp;

    history_->setText(str);
}