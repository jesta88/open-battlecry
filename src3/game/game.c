#include <engine/defines.h>
#include <engine/game.h>

wk_game_desc wk_game_main(int argc, char* argv[])
{
    return (wk_game_desc) {
        .title = "Warhead",
        .target_tickrate = WK_TICKRATE_240
    };
}