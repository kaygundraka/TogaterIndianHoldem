#ifndef __GAME_MANAGER_H__
#define __GAME_MANAGER_H__

#include "cocos2d.h"
#include "player.h"
#include "main_scene.h"
#include "lobby_scene.h"
#include "friend_match_dialog.h"
#include "singleton.h"
#include "holdem_card.h"
#include "ui\UIButton.h"
#include "ui\UITextField.h"
#include "ui\UIListView.h"
#include "ui\UIText.h"

class game_manager : public singleton<game_manager>
{
public:
    virtual bool init_singleton();
    virtual bool release_singleton();

    game_manager();

    enum SCENE_TYPE { LOGIN = 0, LOBBY, LOADING, ROOM };
    enum CHAT_TYPE { NORMAL, WHISPER, NOTICE };


    // friend ui
    cocos2d::ui::ListView* friend_list_;
    cocos2d::ui::TextField* friend_text_field;

    void set_friend_text_field(std::string text);
    void add_friend_in_list(std::string id);
    void del_friend_in_list(std::string id);


    // game play
    int total_money_;
    int batting_money_;
    bool wait_turn;

    player* user_;
    player* opponent_;
    
    holdem_card* public_card_;
    holdem_card* opponent_card_;

    cocos2d::Label* opponent_info_text_;
    cocos2d::Label* user_bet_text_;
    
    std::string opponent_info_;
    std::string opponent_id_;

    cocos2d::Scheduler* scheduler_[4];
    
    cocos2d::ui::Button* bet_button_;
    cocos2d::ui::TextField* text_field_;

    cocos2d::ui::ListView* lobby_chat_list_;
    cocos2d::ui::ListView* room_chat_list_;
        
    main_scene* scene_;
    lobby_scene* lobby_scene_;
    
    friend_match_dialog* friend_match_dialog_;

    cocos2d::ui::Text* history_;

    bool send_friend_match_;
    bool accept_friend_match_;
    std::string friend_match_id_;
    bool hide_card_;

    SCENE_TYPE scene_type_;

    cocos2d::Sprite* rating_image;

    bool bgm_;

    // function

    void set_scene_status(SCENE_TYPE status);
    cocos2d::Scheduler* get_scheduler();

    void set_opponent_info(std::string id, int win, int defeat, int rating);
    void update_chat(std::string id, std::string str, CHAT_TYPE type);

    void new_turn(int public_card_1, int public_card_2, int opponent_card, int remain_money, int my_money, int opponent_money);
    void opponent_turn_end(int my_money, int opponent_money);
    void betting();

    void check_public_card();

    void start_game();
    
    void set_history(int win, int lose, int rating);

    void set_tear(int rating);
};


#define game_mgr game_manager::get_instance()
#endif // __GAME_MANAGER_H__
