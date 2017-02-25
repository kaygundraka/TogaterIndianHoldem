//#include "network_manager.h"
//#include "game_manager.h"
//
//void network_manager::process_packet_enter_ans(logic_server::packet_enter_ans packet)
//{
//    thread_sync sync;
//
//    logic_server::packet_enter_ans recevie_packet;
//}
//
//void network_manager::process_packet_process_turn_req(logic_server::packet_process_turn_req packet)
//{
//    thread_sync sync;
//
//    game_mgr->scheduler_->performFunctionInCocosThread(
//        CC_CALLBACK_0(
//            game_manager::opponent_turn_end, game_mgr,
//            packet.my_money(),
//            packet.opponent_money()
//        )
//    );
//}
//
//void network_manager::process_packet_process_turn_ntf(logic_server::packet_process_turn_ntf packet)
//{
//    thread_sync sync;
//
//    game_mgr->scheduler_->performFunctionInCocosThread(
//        CC_CALLBACK_0(
//            game_manager::new_turn, game_mgr,
//            packet.public_card_number_1(),
//            packet.public_card_number_2(),
//            packet.opponent_card_number(),
//            packet.remain_money(),
//            packet.my_money(),
//            packet.opponent_money()
//        )
//    );
//}
//
//void network_manager::process_packet_process_check_card_ntf(logic_server::packet_process_check_card_ntf packet)
//{
//    thread_sync sync;
//
//    game_mgr->scheduler_->performFunctionInCocosThread(
//        CC_CALLBACK_0(
//            game_manager::check_public_card, game_mgr,
//        )
//    );
//}
//
//void network_manager::process_packet_game_state_ntf(logic_server::packet_game_state_ntf packet)
//{
//    thread_sync sync;
//
//    if (packet.state() == 1)
//    {
//        game_mgr->scheduler_->performFunctionInCocosThread(
//            CC_CALLBACK_0(
//                game_manager::start_game, game_mgr,
//                )
//        );
//    }
//    else if (packet.state() == 2)
//    {
//        this->disconnect();
//        
//        game_mgr->scheduler_->performFunctionInCocosThread(
//            CC_CALLBACK_0(
//                main_scene::end, game_mgr->scene_,
//                )
//        );
//    }
//}
//
//void network_manager::send_packet_enter_req(std::string room_key, std::string player_key)
//{
//    thread_sync sync;
//
//    logic_server::packet_enter_req enter_req_packet;
//    enter_req_packet.set_room_key(room_key);
//    enter_req_packet.set_player_key(player_key);
//
//    room_key_ = room_key;
//
//    this->handle_send(logic_server::ENTER_REQ, enter_req_packet);
//}
//
//void network_manager::send_packet_process_turn_ans(int money)
//{
//    thread_sync sync;
//
//    logic_server::packet_process_turn_ans process_turn_packet;
//    process_turn_packet.set_money(money);
//    
//    this->handle_send(logic_server::PROCESS_TURN_ANS, process_turn_packet);
//}
//
//void network_manager::send_packet_disconnect_room_ntf()
//{
//    thread_sync sync;
//
//    logic_server::packet_disconnect_room_ntf disconnect_room_packet;
//    disconnect_room_packet.set_room_key(room_key_);
//
//    this->handle_send(logic_server::DISCONNECT_ROOM, disconnect_room_packet);
//}