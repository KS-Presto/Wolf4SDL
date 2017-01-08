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
    void serverTxPoll(void);

    void serverRxPoll(void);

    void serverFinishRxWaitPoll(void);

    void clientRxPoll(void);

    namespace PollState
    {
        enum e
        {
            serverTx,
            serverRx,
            serverFinishRxWait,
            clientRx,
        };

        typedef void (*Callback)(void);

        Callback fns[] =
        {
            serverTxPoll,
            serverRxPoll,
            serverFinishRxWaitPoll,
            clientRxPoll,
        };

        enum e cur = clientRx;
        int32_t tics = 0;

        void set(enum e x, int32_t duration)
        {
            cur = x;
            tics = duration;
        }

        void poll(void)
        {
            fns[cur]();
        }

        bool is(enum e x) { return cur == x; }
    }

    class Peer
    {
    public:
        typedef std::vector<Peer> Vec;

        int uid;
        IPaddress address;
        Protocol::DataLayer protState;
        bool expectingResp;

        Peer(int uid_, IPaddress address_) : uid(uid_),
            address(address_), expectingResp(false)
        {
        }
    };

    class PeerHasUid
    {
        int uid;

    public:
        explicit PeerHasUid(int uid_) : uid(uid_)
        {
        }

        bool operator()(Peer &peer) const
        {
            return peer.uid == uid;
        }
    };

    class SetPeerExpectingResp
    {
        bool expectingResp;

    public:
        explicit SetPeerExpectingResp(bool expectingResp_) :
            expectingResp(expectingResp_)
        {
        }

        void operator()(Peer &peer) const
        {
            peer.expectingResp = expectingResp;
        }
    };

    class PeerIsExpectingResp
    {
    public:
        bool operator()(Peer &peer) const
        {
            return peer.expectingResp;
        }
    };

    inline bool hasPeerWithUid(Peer::Vec &peers, int peeruid)
    {
        return std::find_if(peers.begin(), peers.end(),
            PeerHasUid(peeruid)) != peers.end();
    }

    Peer &peerWithUid(Peer::Vec &peers, int peeruid)
    {
        Peer::Vec::iterator it = std::find_if(peers.begin(),
            peers.end(), PeerHasUid(peeruid));
        if (it == peers.end())
        {
            throw "no peer found with specified uid";
        }
        return *it;
    }

    const int channel = 0;
    const int maxpacketsize = 1024;
    const int32_t maxWaitResponseTics = 30;

    Uint16 port = 0;
    int peeruid = 0;
    UDPsocket udpsock;
    UDPpacket *packet = NULL;
    Protocol::DataLayer protState;
    Peer::Vec peers;
    bool foundPeerNoResp = false;
    unsigned int packetSeqNum = 0;
    PollState::e pollState = PollState::serverTx;
    bool server = false;

    void fillPacket(Protocol::DataLayer &protState);
    bool addPlayerTo(int peeruid, Protocol::DataLayer &protState);
    void syncPlayerStateTo(Protocol::DataLayer &protState);
    void prepareStateForSending(Protocol::DataLayer &protState);
    void parsePacket(Protocol::DataLayer &protState);
    void handleStateReceived(Protocol::DataLayer &rxProtState);
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

    // allocate tx packet for this client
    packet = SDLNet_AllocPacket(maxpacketsize);
    if (!packet)
    {
        Quit("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
    }

    packet->channel = channel;

    PollState::set(server ? PollState::serverTx : PollState::clientRx, 0);
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

bool Comms::addPlayerTo(int peeruid, Protocol::DataLayer &protState)
{
    using namespace Protocol;

    if (peeruid != 0 && !protState.hasPlayerForPeerUid(peeruid))
    {
        protState.addPlayer(Player(peeruid));
        return true;
    }

    return false;
}

void Comms::syncPlayerStateTo(Protocol::DataLayer &protState)
{
    using namespace Protocol;

    if (protState.hasPlayerForPeerUid(peeruid))
    {
        Player &protPlayer = protState.playerForPeerUid(peeruid);
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

void Comms::prepareStateForSending(Protocol::DataLayer &protState)
{
    addPlayerTo(peeruid, protState);
    syncPlayerStateTo(protState);
    protState.sendingPeerUid = peeruid;
    protState.packetSeqNum = packetSeqNum;
}

void Comms::fillPacket(Protocol::DataLayer &protState)
{
    using namespace Protocol;

    Stream stream(packet->data, maxpacketsize,
        StreamDirection::out);
    stream & protState;

    packet->len = maxpacketsize - stream.sizeleft;
}

void Comms::parsePacket(Protocol::DataLayer &rxProtState)
{
    using namespace Protocol;

    Stream stream(packet->data, maxpacketsize,
        StreamDirection::in);
    stream & rxProtState;
}

void Comms::handleStateReceived(Protocol::DataLayer &rxProtState)
{
    using namespace Protocol;

    const int uid = rxProtState.sendingPeerUid;

    if (!hasPeerWithUid(peers, uid))
    {
        // unknown peer so ignore packet
        return;
    }

    Peer &peer = peerWithUid(peers, uid);
    if (rxProtState.packetSeqNum != packetSeqNum)
    {
        // ignore packet with wrong sequence number
        return;
    }

    peer.expectingResp = false;

    peer.protState = rxProtState;

#if 0
    if (addPlayerTo(uid, protState))
    {
        // this is a new player in our state
        Player &protPlayer = protState.playerForPeerUid(uid);

        // copy the rx player over it
        protPlayer = rxProtState.playerForPeerUid(uid);
    }

    if (protState.hasPlayerForPeerUid(uid))
    {
        Player &protPlayer = protState.playerForPeerUid(uid);
        Player &rxProtPlayer = rxProtState.playerForPeerUid(uid);

        // merge the rx player with our one
    }
#endif
}

void Comms::serverTxPoll(void)
{
    if (!foundPeerNoResp)
    {
        prepareStateForSending(protState);
    }
    fillPacket(protState);

    int numsent = SDLNet_UDP_Send(udpsock,
        packet->channel, packet);
    if (!numsent)
    {
        printf("SDLNet_UDP_Send: %s\n", SDLNet_GetError());
    }

    PollState::set(PollState::serverRx, maxWaitResponseTics);
}

void Comms::serverRxPoll(void)
{
getmore:
    int numrecv = SDLNet_UDP_Recv(udpsock, packet);
    if (numrecv == -1)
    {
        printf("SDLNet_UDP_Recv: %s\n", SDLNet_GetError());
    }
    else if (numrecv)
    {
        Protocol::DataLayer rxProtState;
        parsePacket(rxProtState);
        handleStateReceived(rxProtState);

        goto getmore;
    }

    PollState::tics -= tics;
    if (PollState::tics < 0)
    {
        if (std::find_if(peers.begin(), peers.end(),
            PeerIsExpectingResp()) != peers.end())
        {
            foundPeerNoResp = true;

            PollState::set(PollState::serverTx, 0);
        }
        else
        {
            foundPeerNoResp = false;
            packetSeqNum++;

            PollState::set(PollState::serverTx, 0);

            std::for_each(peers.begin(), peers.end(),
                SetPeerExpectingResp(true));
        }
    }
    else
    {
        if (std::find_if(peers.begin(), peers.end(),
            PeerIsExpectingResp()) == peers.end())
        {
            foundPeerNoResp = false;
            packetSeqNum++;

            PollState::set(PollState::serverFinishRxWait, PollState::tics);

            std::for_each(peers.begin(), peers.end(),
                SetPeerExpectingResp(true));
        }
    }
}

void Comms::serverFinishRxWaitPoll(void)
{
    PollState::tics -= tics;
    if (PollState::tics < 0)
    {
        PollState::set(PollState::serverTx, 0);
    }
}

void Comms::clientRxPoll(void)
{
}

void Comms::poll(void)
{
    PollState::poll();
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
            printf("The addpeer option is missing the uid argument!\n");
            hasError = true;
            return true;
        }

        const int uid = atoi(argv[i]);

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

        peers.push_back(Peer(uid, address));

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
    else IFARG("--server")
    {
        Comms::server = true;
        return true;
    }
    else
    {
        return false;
    }
}

const char *Comms::parameterHelp(void)
{
    return " --port <port>                   UDP server port\n"
           " --addpeer <uid> <host> <port>   Binds peer address to UDP socket\n"
           " --peeruid <uid>                 Peer unique id\n"
           " --server                        Sets this peer as server\n"
           ;
}

