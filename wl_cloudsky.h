// WL_CLOUDSKY.H

#if defined(USE_CLOUDSKY) && !defined(_WL_CLOUDSKY_H_)
#define _WL_CLOUDSKY_H_

typedef struct
{
    byte    length;
    int16_t startAndDir;
} colormapentry_t;

typedef struct
{
    byte            numColors;
    colormapentry_t *entries;
} colormap_t;


/*
=============================================================================

                          CLOUDSKY_T DATA STRUCTURE

=============================================================================
=
= The seed defines the look of the sky and every value (0-4294967295)
= describes an unique sky. You can play around with these inside the game
= when pressing <TAB>+Z in debug mode. There you'll be able to change the
= active seed to find a value which is suitable for your needs.
=
= The speed defines how fast the clouds will move (0-65535)
=
= The angle defines the move direction (0-359)
=
= The colorMapIndex selects the color map to be used for this sky definition.
= This value can also be chosen with <TAB>+Z
=
=============================================================================
*/

typedef struct
{
    uint32_t seed;
    uint16_t speed;
    uint16_t angle;
    byte     colorMapIndex;
} cloudsky_t;

extern cloudsky_t *curSky;
extern colormap_t colorMaps[];
extern const int  numColorMaps;

void InitSky (void);
void DrawCloudPlanes (void);

#ifndef USE_FEATUREFLAGS
int  GetCloudSkyDefID (void);
#endif

#endif
