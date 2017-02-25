#ifndef __LOADING_SCENE_H__
#define __LOADING_SCENE_H__

#include "cocos2d.h"
#include <ui/UIListView.h>

class loading_scene : public cocos2d::LayerColor
{
private:
    float timer;

public:
    static cocos2d::Scene* createScene();

    virtual bool init();

    void update(float delta);
   
    CREATE_FUNC(loading_scene);
};

#endif