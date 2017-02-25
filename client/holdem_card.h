#ifndef __HOLDEM_CARD_H__
#define __HOLDEM_CARD_H__

#include "cocos2d.h"
#include "my_thread_sync.h"

class holdem_card {
public :
    holdem_card(int layer, int num, bool is_rotated, std::string info);

    void move_action(cocos2d::Vec2 destination, float time);

    void show();

    void release();

private:
    bool is_rotated_;
    int num_;

    cocos2d::Vec2 pivot_;

    cocos2d::Sprite* sprite_;
    cocos2d::Label* label_num_;
    
    bool show_label_;
};

#endif