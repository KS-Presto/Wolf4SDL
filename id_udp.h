#ifndef __ID_UDP__
#define __ID_UDP__

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

    void startup(void);

    void shutdown(void);
}

#endif
