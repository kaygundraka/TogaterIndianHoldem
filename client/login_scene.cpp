#include "login_scene.h"
#include "lobby_scene.h"
#include <ui\UIButton.h>
#include <network\HttpRequest.h>
#include <network\HttpClient.h>
#include <ui\UITextField.h>
#include "network_manager.h"
#include "channel_session.h"
#include "chat_session.h"
#include "game_manager.h"
#include <SimpleAudioEngine.h>

using namespace cocos2d;

ui::TextField* id_field;
ui::TextField* pw_field;

Scene* login_scene::createScene()
{
    auto scene = Scene::create();

    auto layer = login_scene::create();

    scene->addChild(layer);

    return scene;
}

bool login_scene::init()
{
    if (!LayerColor::initWithColor(Color4B(0, 255, 0, 255)))
        return false;

    auto winSize = CCDirector::sharedDirector()->getWinSizeInPixels();
    auto visibleSize = Director::getInstance()->getVisibleSize();

    if (game_mgr->bgm_)
        CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("main_bgm.mp3", true);

    auto background = Sprite::create("login_background.png");
    background->setAnchorPoint(Vec2(0, 0));
    background->setScale(1.28f);
    this->addChild(background, 0);

    auto login_button = ui::Button::create("button_normal.png", "button_pressed.png");

    login_button->setTitleText("LOGIN");
    login_button->setTitleFontName("fonts/D2Coding.ttf");
    login_button->setTitleFontSize(20);
    login_button->setTitleColor(Color3B::BLACK);
    login_button->setScale(1.22f, 1.22f);
    login_button->setPosition(Vec2(visibleSize.width / 2 + 29, 80));
    login_button->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type) {
        switch (type)
        {
        case ui::Widget::TouchEventType::ENDED:
            login_game();
            break;
        }
    });

    this->addChild(login_button, 1);


    id_field = ui::TextField::create("ID", "fonts/D2Coding.ttf", 20);
    id_field->setMaxLength(10);
    id_field->setMaxLengthEnabled(true);
    id_field->setColor(cocos2d::Color3B::BLACK);
    id_field->setPosition(Vec2(visibleSize.width / 2 + 27, visibleSize.height / 2 - 25));
    this->addChild(id_field, 1);
    
    pw_field = ui::TextField::create("Password", "fonts/D2Coding.ttf", 20);
    pw_field->setMaxLength(10);
    pw_field->setMaxLengthEnabled(true);
    pw_field->setPasswordEnabled(true);
    pw_field->setColor(cocos2d::Color3B::BLACK);
    pw_field->setPosition(Vec2(visibleSize.width / 2 + 27, visibleSize.height / 2 - 90));
    this->addChild(pw_field, 1);

    return true;
}

void login_scene::login_game()
{
    network_mgr->set_player_id(id_field->getString());

    cocos2d::network::HttpRequest* request = new cocos2d::network::HttpRequest();
    request->setUrl("http://192.168.1.203:3000/login");
    std::string data = "id=";
    data += id_field->getString();
    data += "&password=";
    data += pw_field->getString();

    request->setRequestType(cocos2d::network::HttpRequest::Type::POST);
    request->setRequestData(data.c_str(), data.size());
    request->setResponseCallback(this, httpresponse_selector(login_scene::http_request_complete));
    
    cocos2d::network::HttpClient::getInstance()->send(request);
    
    request->release();
}

void login_scene::http_request_complete(cocos2d::network::HttpClient *sender, cocos2d::network::HttpResponse *response)
{
    if (!response)
        return;

    std::vector<char> *buffer = response->getResponseData();
    
    std::string data;
    
    for (int i = 0; i < buffer->size(); i++)
        data.push_back((*buffer)[i]);

    data.push_back('\0');
    std::string sub2;

    for (int i = 0; i < 3; i++)
        sub2.push_back(data[i]);

    sub2.push_back('\0');

    if (strcmp(sub2.c_str(), "ok:") == 0)
    {
        std::string sub;

        for (int i = 3; i < data.size() - 1; i++)
            sub.push_back(data[i]);

        network_mgr->set_player_key(sub);

        auto scene = lobby_scene::createScene();
        Director::getInstance()->pushScene(TransitionMoveInR::create(2, scene));
    }
    else
    {
        id_field->setText("");
        pw_field->setText(""); 
    }
}