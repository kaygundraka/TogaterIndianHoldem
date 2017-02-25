#ifndef __MAIN_SCENE_H__
#define __MAIN_SCENE_H__

#include "cocos2d.h"
#include "player.h"

class main_scene : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();

    bool is_end;
    float end_timer;

    void end();
    void show_result(std::string msg);

    void menu_close_callback(cocos2d::Ref* sender);

    bool on_touch_begin(cocos2d::Touch *touch, cocos2d::Event *unused_event);
    void on_touch_moved(cocos2d::Touch *touch, cocos2d::Event *unused_event);
    void on_touch_cancelled(cocos2d::Touch *touch, cocos2d::Event *unused_event);
    void on_touch_ended(cocos2d::Touch *touch, cocos2d::Event *unused_event);

    void on_key_pressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);
    void update(float delta);

    CREATE_FUNC(main_scene);
};

#endif // __MAIN_SCENE_H__
