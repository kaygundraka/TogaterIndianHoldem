#include "pre_header.h"
#include "logger.h"
#include "game_manager.h"
#include "network_manager.h"
#include "configurator.h"

int main(int argc, char* argv[])
{
    if (strcmp(argv[2], "true") == 0)
        logger::is_debug_mode(true);
    else
        logger::is_debug_mode(false);

    network_mgr->init_singleton();
    game_mgr->init_singleton();
    
    game_mgr->play_game(argv[1], argv[1]);
    
    game_mgr->release_singleton();
    network_mgr->release_singleton();

    return 0;
}