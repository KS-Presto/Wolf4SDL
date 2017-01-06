// ID_UDP.C

#include <algorithm>
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
    namespace TransmitState
    {
        enum e
        {
            tx,
            rx,
        };
    }

    class Peer
    {
    public:
        typedef std::vector<Peer> Vec;

        int id;
        IPaddress address;
        Protocol::World world;

        Peer(int id_, IPaddress address_) : id(id_), address(address_)
        {
        }
    };

    const int channel = 0;
    const int maxpacketsize = 1024;
    const int32_t transmitrate = 70;

    Uint16 port = 0;
    int peeruid = 0;
    UDPsocket udpsock;
    UDPpacket *packet = NULL;
    int32_t transmitTics = 0;
    Protocol::World world;
    Peer::Vec peers;
    TransmitState::e transmitState = TransmitState::tx;

    void fillPacket(void);
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
    if (SDLNet_Init() == -1)
    {
        Quit("SDLNet_Init: %s\n", SDLNet_GetError());
    }

    // create a UDPsocket on port
    udpsock = SDLNet_UDP_Open(port);
    if (!udpsock)
    {
        Quit("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
    }

    // Bind addresses to channel
    for (Peer::Vec::size_type i = 0; i < peers.size(); i++)
    {
        Peer &peer = peers[i];

        const int ret = SDLNet_UDP_Bind(udpsock, channel,
            &peer.address);
        if (ret != channel)
        {
            Quit("SDLNet_UDP_Bind: %s\n", SDLNet_GetError());
        }
    }

    packet = SDLNet_AllocPacket(maxpacketsize);
    if (!packet)
    {
        Quit("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
    }

    packet->channel = channel;
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

namespace Comms
{
    namespace Protocol
    {
        class PlayerHasPeerUid
        {
            int peeruid;

        public:
            PlayerHasPeerUid(int peeruid_) : peeruid(peeruid_)
            {
            }

            bool operator()(const Player &player) const
            {
                return player.peeruid == peeruid;
            }
        };
    }
}

void Comms::fillPacket(void)
{
    using namespace Protocol;

    // add this player to world if not there already
    {
        Player::Vec &players = world.players;
        Player::Vec::iterator it = std::find_if(players.begin(),
            players.end(), PlayerHasPeerUid(peeruid));
        if (peeruid != 0 && it == players.end())
        {
            players.push_back(Player(peeruid));
        }
    }

    // update our player
    {
        Player::Vec &players = world.players;
        Player::Vec::iterator it = std::find_if(players.begin(),
            players.end(), PlayerHasPeerUid(peeruid));
        if (it != players.end())
        {
            Player &protPlayer = *it;
            protPlayer.x = player->x;
            protPlayer.y = player->y;
            protPlayer.angle = player->angle;
            protPlayer.health = gamestate.health;
            protPlayer.ammo = gamestate.ammo;
            switch (gamestate.weapon)
            {
                case wp_knife:
                    protPlayer.weapon = Weapon::knife;
                    break;
                case wp_pistol:
                    protPlayer.weapon = Weapon::pistol;
                    break;
                case wp_machinegun:
                    protPlayer.weapon = Weapon::machineGun;
                    break;
                case wp_chaingun:
                    protPlayer.weapon = Weapon::chainGun;
                    break;
            }
        }
    }

    Stream stream(packet->data, maxpacketsize,
        StreamDirection::out);
    stream & world;

    packet->len = maxpacketsize - stream.sizeleft;
}

void Comms::poll(void)
{
    transmitTics += tics;
    if (transmitTics >= transmitrate)
    {
        transmitTics = 0;

        if (transmitState == TransmitState::tx)
        {
            fillPacket();

            int numsent = SDLNet_UDP_Send(udpsock,
                packet->channel, packet);
            if (!numsent)
            {
                printf("SDLNet_UDP_Send: %s\n", SDLNet_GetError());
            }

            transmitState = TransmitState::rx;
        }
        else if (transmitState == TransmitState::rx)
        {
            int numrecv = SDLNet_UDP_Recv(udpsock, packet);
            if (numrecv == -1)
            {
                printf("SDLNet_UDP_Recv: %s\n", SDLNet_GetError());
            }
            else if (numrecv)
            {
                // do something with packet
            }
            else
            {
                transmitState = TransmitState::tx;
            }
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
    else IFARG("--addpeer")
    {
        if(++i >= argc)
        {
            printf("The addpeer option is missing the id argument!\n");
            hasError = true;
            return true;
        }

        const int id = atoi(argv[i]);

        if(++i >= argc)
        {
            printf("The addpeer option is missing the host argument!\n");
            hasError = true;
            return true;
        }

        const char *host = argv[i];

        if(++i >= argc)
        {
            printf("The addpeer option is missing the port argument!\n");
            hasError = true;
            return true;
        }

        const int port = atoi(argv[i]);

        IPaddress address;
        if (SDLNet_ResolveHost(&address, host, port) == -1)
        {
            printf("IP address could not be resolved "
                "from %s:%d!\n", host, port);
            hasError = true;
            return true;
        }

        peers.push_back(Peer(id, address));

        return true;
    }
    else IFARG("--peeruid")
    {
        if(++i >= argc)
        {
            printf("The peeruid option is missing the uid "
                "argument!\n");
            hasError = true;
        }
        else
        {
            Comms::peeruid = atoi(argv[i]);
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
    return " --port                          UDP server port\n"
           " --addpeer <id> <host> <port>    Binds UDP socket to peer\n"
           " --peeruid <uid>                 Peer unique id\n"
           ;
}

