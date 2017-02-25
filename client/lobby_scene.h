#ifndef __LOBBY_SCENE_H__
#define __LOBBY_SCENE_H__

#include "cocos2d.h"
#include <ui/UIListView.h>

class lobby_scene : public cocos2d::LayerColor
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();

    void show_friend_match_pop_up();
    void hide_friend_match_pop_up();

    void chat_event_listener(Ref* sender, cocos2d::ui::ListView::EventType type);

    void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) override;
    void onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) override;

    void match_game();

    CREATE_FUNC(lobby_scene);
};

#endif