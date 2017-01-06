// ID_UDP.C

#include <sys/types.h>
#if defined _WIN32
    #include <io.h>
#elif defined _arch_dreamcast
    #include <unistd.h>
#else
    #include <sys/uio.h>
    #include <unistd.h>
#endif
#include "SDL_net.h"

#include "wl_def.h"
#pragma hdrstop

using namespace Comms;


/*
=====================================================================

                            COMMS CLIENT API

=====================================================================
*/

namespace Comms
{
    Uint16 port = 0;
    UDPsocket udpsock;
    IPaddress bindToIpaddr;
    int channel = -1;
    UDPpacket *packet = NULL;

    bool buildPacket(void);
}

void Comms::startup(void)
{
    SDL_version compile_version;
    const SDL_version *link_version = SDLNet_Linked_Version();
    SDL_NET_VERSION(&compile_version);
    printf("compiled with SDL_net version: %d.%d.%d\n", 
            compile_version.major,
            compile_version.minor,
            compile_version.patch);
    printf("running with SDL_net version: %d.%d.%d\n", 
            link_version->major,
            link_version->minor,
            link_version->patch);

    //if(SDL_Init(0)==-1)
    //{
    //    Quit("SDL_Init: %s\n", SDL_GetError());
    //    exit(1);
    //}
    if(SDLNet_Init()==-1)
    {
        Quit("SDLNet_Init: %s\n", SDLNet_GetError());
    }

    // create a UDPsocket on port
    udpsock = SDLNet_UDP_Open(port);
    if(!udpsock)
    {
        Quit("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
    }

    // Bind address to the first free channel
    channel = SDLNet_UDP_Bind(udpsock, -1, &bindToIpaddr);
    if (channel == -1)
    {
        Quit("SDLNet_UDP_Bind: %s\n", SDLNet_GetError());
    }
}

void Comms::shutdown(void)
{
    if (packet)
    {
        SDLNet_FreePacket(packet);
        packet = NULL;
    }

    SDLNet_UDP_Unbind(udpsock, channel);

    SDLNet_UDP_Close(udpsock);
    udpsock = NULL;

    SDLNet_Quit();
}

bool Comms::buildPacket(void)
{
    if (!packet)
    {
        packet = SDLNet_AllocPacket(1024);
        if(!packet)
        {
            printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
            return false;
            // perhaps do something else since you can't make this packet
        }
    }

    return true;
}

void Comms::poll(void)
{
    if (buildPacket())
    {
        int numsent = SDLNet_UDP_Send(udpsock,
            packet->channel, packet);
        if (!numsent)
        {
            printf("SDLNet_UDP_Send: %s\n", SDLNet_GetError());
        }
    }
}


/*
=====================================================================

                        COMMS PARAMETER HANDLING

=====================================================================
*/

Parameter::Parameter(
    char **argv_,
    const int argc_,
    int &i_,
    bool &hasError_,
    bool &showHelp_
    ) :
    argv(argv_),
    argc(argc_),
    i(i_),
    hasError(hasError_),
    showHelp(showHelp_)
{
}

#define IFARG(str) if(!strcmp(arg, (str)))

bool Parameter::check(const char *arg)
{
    IFARG("--port")
    {
        if(++i >= argc)
        {
            printf("The port option is missing the udp server port "
                "argument!\n");
            hasError = true;
        }
        else
        {
            Comms::port = atoi(argv[i]);
        }

        return true;
    }
    else IFARG("--bindto")
    {
        if(++i >= argc)
        {
            printf("The bindto option is missing the host argument!\n");
            hasError = true;
            return true;
        }

        const char *host = argv[i];

        if(++i >= argc)
        {
            printf("The bindto option is missing the port argument!\n");
            hasError = true;
            return true;
        }

        const int port = atoi(argv[i]);

        if (SDLNet_ResolveHost(&Comms::bindToIpaddr,
            host, port) == -1)
        {
            printf("IP address could not be resolved "
                "from %s:%d!\n", host, port);
            hasError = true;
            return true;
        }

        return true;
    }
    else
    {
        return false;
    }
}

const char *Comms::parameterHelp(void)
{
    return " --port                 UDP server port\n"
           " --bindto <host> <port> Binds UDP socket to an address\n";
}

