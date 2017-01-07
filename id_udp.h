#ifndef __ID_UDP__
#define __ID_UDP__

#include <stdint.h>
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

            void serialize(unsigned int &x)
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

        inline void serialize(Stream &stream, unsigned int &x)
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

        class Player
        {
        public:
            typedef std::vector<Player> Vec;

            int peeruid;
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

        class World
        {
        public:
            Player::Vec players;

            World()
            {
            }

            void serialize(Stream &stream)
            {
                stream & players;
            }
        };

        inline void serialize(Stream &stream, World &x)
        {
            x.serialize(stream);
        }

        class DataLayer
        {
        public:
            int sendingPeerUid;
            World world;

            DataLayer() : sendingPeerUid(0)
            {
            }

            void serialize(Stream &stream)
            {
                stream & sendingPeerUid;
                stream & world;
            }
        };

        inline void serialize(Stream &stream, DataLayer &x)
        {
            x.serialize(stream);
        }
    }
}

#endif
