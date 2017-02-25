#ifndef __COIN_H__
#define __COIN_H__

#include "cocos2d.h"

class coin {
public:
    coin(int layer_);
    ~coin();
    
    int get_layer();
    
    void move_to(cocos2d::Vec2 destination, float time);
    void move_by(cocos2d::Vec2 destination, float time);
    
    void touch();

    enum OWNER { USER, OPPONENT };
    OWNER owner_;

    void set_player(OWNER type);
    void set_z_order(int i);
    void bet(bool is_betting);
    
    cocos2d::Vec2 get_position();

    cocos2d::Sprite* sprite_;

private:
    int layer_;
    bool is_betting;

    cocos2d::Vec2 position;
};

#endif