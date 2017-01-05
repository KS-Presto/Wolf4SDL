// ID_UDP.C

/*
=============================================================================

Refer to LW_UDPCOMMS for a general description of what this is

=============================================================================
*/

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
#include "id_udp.h"

#include "wl_def.h"
#pragma hdrstop

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
}

void Comms::shutdown(void)
{
    SDLNet_Quit();
}
