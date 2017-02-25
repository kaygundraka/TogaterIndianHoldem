#ifndef __LOGIN_SCENE_H__
#define __LOGIN_SCENE_H__

#include "cocos2d.h"
#include <network\HttpClient.h>

class login_scene : public cocos2d::LayerColor
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();

    void login_game();

    void http_request_complete(cocos2d::network::HttpClient *sender, cocos2d::network::HttpResponse *response);

    CREATE_FUNC(login_scene);
};

#endif