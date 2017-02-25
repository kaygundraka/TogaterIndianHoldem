#include "tcp_client.h"
#include "config.h"
#include <atlstr.h>


int main(int argc, char* argv[])
{
    boost::asio::io_service io_service;


    std::string SERVER_IP;
    int SERVER_PORT;

    config::get_value("SERVER_IP", SERVER_IP);
    config::get_value("SERVER_PORT", SERVER_PORT);


    auto endpoint = boost::asio::ip::tcp::endpoint(
        boost::asio::ip::address::from_string(SERVER_IP), SERVER_PORT);

    tcp_client chat_client(io_service);
        
    // --------------------------
    std::string id = "Admin";//argv[1];
    chat_client.set_id(id);
    chat_client.set_key(id);
    // --------------------------

    chat_client.connect(endpoint);

    boost::thread thread(boost::bind(&boost::asio::io_service::run, &io_service));

    //chat_client.post_verify_req();
    
    while (chat_client.is_login())
    {
        std::string message;
        std::getline(std::cin, message);
       
        std::wstring strUni = CA2W(message.c_str());
        std::string strUTF8 = CW2A(strUni.c_str(), CP_UTF8);

        chat_client.post_notice(strUTF8);
    }

    io_service.stop();
    thread.join();
    chat_client.close();

    std::cout << "클라이언트를 종료해 주세요" << std::endl;

    return 0;
}