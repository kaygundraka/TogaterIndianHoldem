#ifndef __FRIEND_MATCH_DIALOG_H__
#define __FRIEND_MATCH_DIALOG_H__

#include "cocos2d.h"
#include <ui/UIListView.h>

class friend_match_dialog : public cocos2d::LayerColor
{
private:
    float timer;

public:
    static cocos2d::Scene* createScene();

    virtual bool init();

    void update(float delta);

    CREATE_FUNC(friend_match_dialog);
};

#endif