#include "pre_header.h"
#include "logic_session.h"
#include "game_manager.h"
#include "logic_server.pb.h"
#include "network_manager.h"
#include "random_generator.h"
#include "logger.h"

bool logic_session::create()
{
    thread_sync sync;

    is_connected_ = false;
        
    socket_ = new tcp::socket(io_service_);

    work_thread_ = nullptr;

    return true;
}

bool logic_session::destroy()
{
    thread_sync sync;

    if (!is_connected_)
        return false;

    disconnect();

    is_connected_ = false;

    work_thread_->join();

    if (socket_ != nullptr)
    {
        delete socket_;
        socket_ = nullptr;
    }

    return true;
}

void logic_session::handle_send(logic_server::message_type msg_type, const protobuf::Message& message)
{
    thread_sync sync;

    MESSAGE_HEADER header;
    header.size = message.ByteSize();
    header.type = msg_type;

    CopyMemory(send_buf_.begin(), (void*)&header, message_header_size);

    message.SerializeToArray(send_buf_.begin() + message_header_size, header.size);

    boost::system::error_code error;
    socket_->write_some(boost::asio::buffer(send_buf_, message_header_size + header.size), error);

    if (error)
    {
        logger::print(error.message().c_str());
        return;
    }
}

void logic_session::handle_read()
{
    while (true)
    {
        if (!is_socket_open())
            break;

        boost::system::error_code error;
        static boost::array<BYTE, BUFSIZE> recv_buf_ori;

        auto size = socket_->read_some(boost::asio::buffer(recv_buf_ori), error);
       
        thread_sync sync;         

        if (error)
        {
            logger::print(error.message().c_str());
            return;
        }

        int remain_size = size;
        int process_size = 0;

        do 
        {
            MESSAGE_HEADER message_header;
            CopyMemory(&message_header, recv_buf_ori.begin() + process_size, message_header_size);
            CopyMemory(recv_buf_.begin(), recv_buf_ori.begin() + process_size, message_header.size + message_header_size);

            process_size += message_header_size + message_header.size;
            remain_size -= process_size;

            switch (message_header.type)
            {
            case logic_server::ENTER_ANS:
            {
                logic_server::packet_enter_ans message;

                if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                    break;

                process_packet_enter_ans(message);
            }
            break;

            case logic_server::PROCESS_TURN_REQ:
            {
                logic_server::packet_process_turn_req message;

                if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                    break;

                process_packet_process_turn_req(message);
            }
            break;

            case logic_server::PROCESS_TURN_NTF:
            {
                logic_server::packet_process_turn_ntf message;

                if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                    break;

                process_packet_process_turn_ntf(message);

                break;
            }

            case logic_server::GAME_STATE_NTF:
            {
                logic_server::packet_game_state_ntf message;

                if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                    break;

                process_packet_game_state_ntf(message);
            }
            break;

            case logic_server::PROCESS_CHECK_CARD_NTF:
            {
                logic_server::packet_process_check_card_ntf message;

                if (false == message.ParseFromArray(recv_buf_.begin() + message_header_size, message_header.size))
                    break;

                process_packet_process_check_card_ntf(message);
            }
            break;
            }

        } while (remain_size > 0);
    }
}


void logic_session::process_packet_enter_ans(logic_server::packet_enter_ans packet)
{
    thread_sync sync;

    logic_server::packet_enter_ans recevie_packet;

    logger::print("logic_session : process_packet_enter_ans");

    network_lobby->send_packet_matching_confirm();
    network_logic->send_packet_game_state_ntf();
}

void logic_session::process_packet_process_turn_req(logic_server::packet_process_turn_req packet)
{
    thread_sync sync;

    logger::print("logic_session : process_packet_process_turn_req");

    int bet = 0;

    bet_money_ = packet.my_money();
    opponent_money_ = packet.opponent_money();

    if (remain_money_ >= opponent_money_)
    {
        int min_bet = opponent_money_ - bet_money_;

        if (remain_money_ - bet_money_ - min_bet > 0)
            bet = min_bet + random_generator::get_random_int(0, remain_money_ - bet_money_ - min_bet);
        else
            bet = min_bet;
    }
    else
        bet = remain_money_ - bet_money_;

    char temp[128] = "";
    sprintf(temp,
        "betting money(%d)",
        bet
    );
    logger::print(temp);

    Sleep(3000);
    this->send_packet_process_turn_ans(bet);
}

void logic_session::process_packet_process_turn_ntf(logic_server::packet_process_turn_ntf packet)
{
    thread_sync sync;

    logger::print("logic_session : process_packet_process_turn_ntf");

    char temp[128] = "";
    
    sprintf(temp,
        "new turn : public_card(%d, %d), opponent_card(%d), remain_money(%d), my_money(%d), opponent_money(%d)",
        packet.public_card_number_1(),
        packet.public_card_number_2(),
        packet.opponent_card_number(),
        packet.remain_money(),
        packet.my_money(),
        packet.opponent_money()
    );

    remain_money_ = packet.remain_money();
    bet_money_ = packet.my_money();
    opponent_money_ = packet.opponent_money();

    logger::print(temp);
}

void logic_session::process_packet_process_check_card_ntf(logic_server::packet_process_check_card_ntf packet)
{
    thread_sync sync;
    
    logger::print("logic_session : process_packet_process_check_card_ntf");
}

void logic_session::process_packet_game_state_ntf(logic_server::packet_game_state_ntf packet)
{
    thread_sync sync;

    if (packet.state() == 2)
    {
        logger::print("process_packet_game_state_ntf");
        logger::print("end game");

        network_lobby->create();
        network_lobby->connect(CHANNEL_SERVER_IP, CHANNEL_SEFVER_PORT);
        network_lobby->send_packet_join_req(network_mgr->get_player_key(), network_mgr->get_player_id());

        game_mgr->end_game_ = true;
    }
}

void logic_session::send_packet_enter_req(std::string room_key, std::string player_key)
{
    thread_sync sync;

    logger::print("logic_session : send_packet_enter_req");

    logic_server::packet_enter_req enter_req_packet;
    enter_req_packet.set_room_key(room_key);
    enter_req_packet.set_player_key(player_key);

    this->handle_send(logic_server::ENTER_REQ, enter_req_packet);
}

void logic_session::send_packet_process_turn_ans(int money)
{
    thread_sync sync;

    logger::print("logic_session : send_packet_process_turn_ans");

    logic_server::packet_process_turn_ans process_turn_packet;
    process_turn_packet.set_money(money);

    this->handle_send(logic_server::PROCESS_TURN_ANS, process_turn_packet);
}

void logic_session::send_packet_disconnect_room_ntf()
{
    thread_sync sync;

    logger::print("logic_session : send_packet_disconnect_room_ntf");

    logic_server::packet_disconnect_room_ntf disconnect_room_packet;
    disconnect_room_packet.set_room_key(network_mgr->get_room_key());

    this->handle_send(logic_server::DISCONNECT_ROOM, disconnect_room_packet);
}

void logic_session::send_packet_game_state_ntf()
{
    thread_sync sync;

    logger::print("logic_session : send_packet_game_state_ntf");

    logic_server::packet_game_state_ntf packet;

    packet.set_state(0);
    packet.set_win_player_key("");

    this->handle_send(logic_server::GAME_STATE_NTF, packet);
}