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

struct app_desc app_main(int argc, char* argv[])
{
    return (struct app_desc){
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