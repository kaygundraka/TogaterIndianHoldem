#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "cocos2d.h"
#include "coin.h"
#include "my_thread_sync.h"

class player {
public:
    player();
    ~player();

    void init(bool is_user);
    void set_type(bool is_user);
    void release();

    void set_lock_bet_size(int size);
    int get_lock_bet_size();

    void bet_coin();
    void restore_coin();

    void reset_coin(player* other_player);
    void set_bet_coin(int size);
        
    int get_coin_size();
    int get_bet_coin_size();

    coin* find_coin(cocos2d::Sprite* sprite);

    std::vector<coin>* coin_list_;
    std::vector<coin>* bet_coin_list_;

private:
    void add_coin(coin temp_coin);

    bool is_user_;
    int lock_bet_size_;
};

#endif