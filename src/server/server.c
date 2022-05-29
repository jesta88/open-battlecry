#define SERVER
#include "../../engine/include/carbon/log.h"

int main(int argc, char* argv[])
{
    log_info("Server started");
#ifdef SERVER
    log_info("Server");
#endif
    return 0;
}
