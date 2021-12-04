#include <engine/application.h>

void init(void)
{

}

void quit(void)
{

}

void reload(void)
{

}

void unload(void)
{

}

void update(float delta_time)
{

}

void draw(void)
{

}

struct ws_app_desc ws_main(int argc, char* argv[])
{
    return (struct ws_app_desc){
        .init = init,
        .quit = quit,
        .reload = reload,
        .unload = unload,
        .update = update,
        .draw = draw,
        .app_name = "Open Battlecry",
        .org_name = "Jeremie St-Amand"
    };
}