#pragma once
#include "pre_headers.h"
#include "singleton.h"
#include "redis_connector.h"
#include "connected_session.h"
#include "critical_section.h"

typedef struct _PLAYER_INFO {
    bool submit_card_;
    int opponent_card_num_;
    std::string id_;

    connected_session* session_;

    int sum_money_;
    int remain_money_;
    int submit_money_;

    _PLAYER_INFO();

} PLAYER_INFO;

typedef struct _ROOM_INFO {
    PLAYER_INFO player_[2];

    PLAYER_INFO* turn_player_;
    
    int turn_count_;

    int ready_player_num_;

    enum GAME_STATE { READY, PLAYING, END };
    GAME_STATE state_;

    int public_card_[2];
    bool hide_card_;

    std::queue<int> card_list_;

    _ROOM_INFO(PLAYER_INFO player_info);

    void generate_card_queue();
    int get_card();

} ROOM_INFO;

class logic_worker : public singleton<logic_worker>, public multi_thread_sync<logic_worker> {
public:
    virtual bool init_singleton();
    virtual bool release_singleton();
    
    bool enter_room_player(connected_session* session, std::string room_key);
    bool process_turn(std::string room_key, std::string player_key, int money);
    bool process_turn_v2(std::string room_key, std::string player_key, int money);
    bool disconnect_room(std::string room_key, std::string player_key);
    bool give_up_game(std::string room_key, std::string player_key);
    bool is_create_room(std::string room_key);
    bool ready_for_game(std::string room_key);

    void process_queue();

    logic_worker();
    virtual ~logic_worker();
    
private:
    boost::unordered_map<std::string, ROOM_INFO> room_hashs_;
    boost::thread* logic_thread_;

    bool end_server_;

    enum HOLDEM_HANDS { NONE = 0, PAIR = 1, STRAIGHT = 2, TRIPLE = 3 };
    HOLDEM_HANDS check_card_mix(int i, int j, int k);
    
    bool create_room(connected_session* session, std::string room_key);
};