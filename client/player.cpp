#include "player.h"
#include "game_manager.h"

player::player()
{
    srand((unsigned int)time(nullptr));

    lock_bet_size_ = 1;
}

player::~player()
{
}

void player::release()
{
    if (coin_list_ != nullptr)
    {
        coin_list_->clear();
        delete coin_list_;
    }

    if (bet_coin_list_ != nullptr)
    {
        bet_coin_list_->clear();
        delete bet_coin_list_;
    }
}

void player::init(bool is_user)
{
    is_user_ = is_user;

    coin_list_ = new std::vector<coin>();
    bet_coin_list_ = new std::vector<coin>();
   
    for (int i = 0; i < 20; i++)
    {
        coin temp_coin(5);

        if (is_user_)
            temp_coin.set_player(coin::OWNER::USER);
        else
            temp_coin.set_player(coin::OWNER::OPPONENT);

        add_coin(temp_coin);
    }
}

coin* player::find_coin(cocos2d::Sprite* sprite)
{
    for (auto iter = coin_list_->begin(); iter != coin_list_->end(); iter++)
        if ((*iter).sprite_ == sprite)
            return &(*iter);

    for (auto iter = bet_coin_list_->begin(); iter != bet_coin_list_->end(); iter++)
        if ((*iter).sprite_ == sprite)
            return &(*iter);

    return nullptr;
}

void player::reset_coin(player* other_player)
{
    bet_coin_list_->insert(
        bet_coin_list_->end(),
        other_player->bet_coin_list_->begin(), 
        other_player->bet_coin_list_->end()
    );

    other_player->bet_coin_list_->clear();

    set_lock_bet_size(0);

    int size = bet_coin_list_->size();

    for (int i = 0; i < size; i++)
        restore_coin();
}

void player::set_bet_coin(int size)
{
    for (int i = 0; i < size; i++)
        bet_coin();
}

void player::set_lock_bet_size(int size)
{
    lock_bet_size_ = size;
}

int player::get_lock_bet_size()
{
    return lock_bet_size_;
}

void player::bet_coin()
{
    if (coin_list_->size() <= 0)
        return;

    auto temp_coin = coin_list_->front();

    auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
    cocos2d::Vec2 origin = cocos2d::Director::getInstance()->getVisibleOrigin();

    cocos2d::Vec2 middle_pos(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);

    if (!is_user_)
        temp_coin.move_to(middle_pos + cocos2d::Vec2(150 + rand() % 50, rand() % 50 + 70), 2);
    else
        temp_coin.move_to(middle_pos + cocos2d::Vec2(150 + rand() % 50, rand() % 50 - 70), 2);

    temp_coin.bet(true);
    
    coin_list_->erase(coin_list_->begin());

    bet_coin_list_->push_back(temp_coin);

    int i = 0;

    for (auto iter = coin_list_->begin(); iter != coin_list_->end(); iter++)
    {
        (*iter).set_z_order(i + 5);

        if (!is_user_)
            (*iter).move_to(cocos2d::Vec2(220 + i * 10, visibleSize.height - 30), 1);
        else
            (*iter).move_to(cocos2d::Vec2(220 + i * 10, 30), 1);

        i++;
    }
}

void player::set_type(bool is_user)
{
    is_user = is_user_;
}

void player::restore_coin()
{
    if (bet_coin_list_->size() <= 0)
        return;

    if (lock_bet_size_ >= bet_coin_list_->size())
        return;

    auto temp_coin = bet_coin_list_->front();

    temp_coin.bet(false);

    add_coin(temp_coin);
    
    bet_coin_list_->erase(bet_coin_list_->begin());
}

void player::add_coin(coin temp_coin)
{
    auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();

    temp_coin.set_z_order(coin_list_->size() + 5);

    if (!is_user_)
        temp_coin.move_to(cocos2d::Vec2(220 + coin_list_->size() * 10, visibleSize.height - 30), 1);
    else
        temp_coin.move_to(cocos2d::Vec2(220 + coin_list_->size() * 10, 30), 1);

    coin_list_->push_back(temp_coin);
}

int player::get_coin_size()
{
    return coin_list_->size();
}

int player::get_bet_coin_size()
{
    return bet_coin_list_->size();
}