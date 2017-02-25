#include "pre_headers.h"
#include "connected_session.h"
#include "logic_worker.h"

void connected_session::process_packet_enter_req(logic_server::packet_enter_req packet)
{
    logic_server::packet_enter_ans recevie_packet;

    room_key_ = packet.room_key();
    player_key_ = packet.player_key();
    
    if (logic_worker::get_instance()->enter_room_player(this, packet.room_key()))
        recevie_packet.set_result(1);
    else
        recevie_packet.set_result(0);

    handle_send(logic_server::ENTER_ANS, recevie_packet);
}

void connected_session::process_packet_process_turn_ans(logic_server::packet_process_turn_ans packet)
{
    //if (!logic_worker::get_instance()->process_turn_v2(room_key_, player_key_, packet.money()))
        //return;
        
    if (!logic_worker::get_instance()->process_turn(room_key_, player_key_, packet.money()))
        return;
}

void connected_session::process_packet_disconnect_room_ntf(logic_server::packet_disconnect_room_ntf packet)
{
    if (logic_worker::get_instance()->give_up_game(room_key_, this->get_player_key()))
        return;
}

void connected_session::process_packet_echo_ntf(logic_server::packet_echo_ntf packet)
{
    this->handle_send(logic_server::ECHO_NTF, packet);
}

void connected_session::porcess_packet_game_state_ntf(logic_server::packet_game_state_ntf packet)
{
    logic_worker::get_instance()->ready_for_game(room_key_);
}