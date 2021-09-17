#include "net.h"

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/socket.h>
#endif