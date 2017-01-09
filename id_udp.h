#ifndef __ID_UDP__
#define __ID_UDP__

#include <stdint.h>
#include <algorithm>
#include <vector>

/*
=====================================================================

                            COMMS CLIENT API

=====================================================================
*/

namespace Comms
{
    void startup(void);

    void shutdown(void);

    void poll(void);
}


/*
=====================================================================

                        COMMS PARAMETER HANDLING

=====================================================================
*/

namespace Comms
{
    class Parameter
    {
    public:
        char **argv;
        const int argc;
        int &i;
        bool &hasError;
        bool &showHelp;

        Parameter(
            char **argv,
            const int argc,
            int &i,
            bool &hasError,
            bool &showHelp
            );

        bool check(const char *arg);
    };

    const char *parameterHelp(void);
}


/*
=====================================================================

                           COMMS PROTOCOL

=====================================================================
*/

namespace Comms
{
    namespace Protocol
    {
        namespace StreamDirection
        {
            enum e
            {
                in,
                out
            };
        }

        class Stream
        {
        public:
            typedef StreamDirection::e Direction;

            uint8_t *data;
            size_t sizeleft;
            Direction dir;

            Stream(uint8_t *data_, size_t maxsize_, Direction dir_) :
                data(data_), sizeleft(maxsize_), dir(dir_)
            {
            }

            void serializeRaw(void *p, size_t sz)
            {
                if (sz > sizeleft)
                {
                    throw "ran out of space in stream";
                }

                if (dir == StreamDirection::in)
                {
                    memcpy(p, data, sz);
                }
                else if (dir == StreamDirection::out)
                {
                    memcpy(data, p, sz);
                }

                data += sz;
                sizeleft -= sz;
            }

            void serialize(int &x)
            {
                serializeRaw(&x, sizeof(x));
            }

            void serialize(short &x)
            {
                serializeRaw(&x, sizeof(x));
            }

            void serialize(unsigned char &x)
            {
                serializeRaw(&x, sizeof(x));
            }

            void serialize(unsigned int &x)
            {
                serializeRaw(&x, sizeof(x));
            }

            void serialize(unsigned long &x)
            {
                serializeRaw(&x, sizeof(x));
            }
        };

        inline void serialize(Stream &stream, int &x)
        {
            stream.serialize(x);
        }

        inline void serialize(Stream &stream, short &x)
        {
            stream.serialize(x);
        }

        inline void serialize(Stream &stream, unsigned char &x)
        {
            stream.serialize(x);
        }

        inline void serialize(Stream &stream, unsigned int &x)
        {
            stream.serialize(x);
        }

        inline void serialize(Stream &stream, unsigned long &x)
        {
            stream.serialize(x);
        }

        template <typename T>
        void serializeEnum(Stream &stream, T &x)
        {
            int y = (int)x;
            serialize(stream, y);
            x = (T)y;
        }

        template <typename T>
        void serialize(Stream &stream, std::vector<T> &x)
        {
            typedef typename std::vector<T>::size_type size_type;

            size_type count = x.size();
            stream & count;

            if (stream.dir == StreamDirection::in)
            {
                x.clear();
                for (size_type i = 0; i < count; i++)
                {
                    T y;
                    stream & y;
                    x.push_back(y);
                }
            }
            else if (stream.dir == StreamDirection::out)
            {
                for (size_type i = 0; i < count; i++)
                {
                    stream & x[i];
                }
            }
        }

        template <typename T>
        Stream &operator&(Stream &stream, T &x)
        {
            serialize(stream, x);
            return stream;
        }

        namespace Weapon
        {
            enum e
            {
                knife,
                pistol,
                machineGun,
                chainGun
            };

            inline void serialize(Stream &stream, enum e &x)
            {
                serializeEnum(stream, x);
            }
        }

        class Pickup
        {
        public:
            uint8_t x, y;

            Pickup() : x(0), y(0)
            {
            }

            Pickup(uint8_t x_, uint8_t y_) : x(x_), y(y_)
            {
            }

            void serialize(Stream &stream)
            {
                stream & x;
                stream & y;
            }
        };

        inline void serialize(Stream &stream, Pickup &x)
        {
            x.serialize(stream);
        }

        namespace PlayerEventOp
        {
            enum e
            {
                move,
                weaponSwitch,
                pickup,
                attack,
            };

            inline void serialize(Stream &stream, enum e &x)
            {
                serializeEnum(stream, x);
            }
        }

        class PlayerEvent
        {
        public:
            typedef std::vector<PlayerEvent> Vec;

            PlayerEventOp::e op;
            int32_t x,y;
            short angle;
            Weapon::e weapon;
            Pickup pickup;
            uint8_t numAttacks;
            int attackPeerId;

            PlayerEvent() : op(PlayerEventOp::move), x(0), y(0),
                angle(0), weapon(Weapon::knife), numAttacks(0), attackPeerId(0)
            {
            }

            void serialize(Stream &stream)
            {
                stream & op;
                switch (op)
                {
                case PlayerEventOp::move:
                    stream & x;
                    stream & y;
                    stream & angle;
                    break;
                case PlayerEventOp::weaponSwitch:
                    stream & weapon;
                    break;
                case PlayerEventOp::pickup:
                    stream & pickup;
                    break;
                case PlayerEventOp::attack:
                    stream & numAttacks;
                    stream & attackPeerId;
                    break;
                }
            }
        };

        inline void serialize(Stream &stream, PlayerEvent &x)
        {
            x.serialize(stream);
        }

        class Player
        {
        public:
            typedef std::vector<Player> Vec;

            int peeruid;
            PlayerEvent::Vec events;
            short health;
            short ammo;
            Weapon::e weapon;
            int32_t x,y;
            short angle;

            explicit Player() : peeruid(0), health(0), ammo(0),
                weapon(Weapon::knife), x(0), y(0), angle(0)
            {
            }

            explicit Player(int peeruid_) : peeruid(peeruid_),
                health(0), ammo(0), weapon(Weapon::knife),
                x(0), y(0), angle(0)
            {
            }

            void serialize(Stream &stream)
            {
                stream & peeruid;
                stream & events;
                stream & health;
                stream & ammo;
                stream & weapon;
                stream & x;
                stream & y;
                stream & angle;
            }
        };

        inline void serialize(Stream &stream, Player &x)
        {
            x.serialize(stream);
        }

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

        inline bool findPlayer(Player::Vec &players, PlayerHasPeerUid pred)
        {
            return std::find_if(players.begin(), players.end(), pred) !=
                players.end();
        }

        inline Player &getPlayer(Player::Vec &players, PlayerHasPeerUid pred)
        {
            Player::Vec::iterator it = std::find_if(players.begin(),
                players.end(), pred);
            if (it == players.end())
            {
                throw "no player found";
            }
            return *it;
        }

        class DataLayer
        {
        public:
            int sendingPeerUid;
            unsigned int packetSeqNum;
            Player::Vec players;

            DataLayer() : sendingPeerUid(0), packetSeqNum(0)
            {
            }

            void serialize(Stream &stream)
            {
                stream & sendingPeerUid;
                stream & packetSeqNum;
                stream & players;
            }
        };

        inline void serialize(Stream &stream, DataLayer &x)
        {
            x.serialize(stream);
        }
    }
}

#endif
