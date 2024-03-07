// WL_PARALLAX.C

#include "version.h"

#ifdef USE_PARALLAX

#include "wl_def.h"

#ifdef USE_COMPOSITEPARALLAX
#define SKYTEXTURESHIFT 7
#else
#define SKYTEXTURESHIFT 6
#endif

#define SKYTEXTURESIZE  (1 << SKYTEXTURESHIFT)
#define SKYTEXTUREMASK  (SKYTEXTURESIZE * (TEXTURESIZE - 1))

int  startskytex;

#ifdef USE_COMPOSITEPARALLAX
/*
====================
=
= Build a table of composite sky textures
=
====================
*/

byte skytextures[USE_PARALLAX][SKYTEXTURESIZE * TEXTURESIZE];

void MakeSky (void)
{
    int  i;
    int  x,y;
    int  skypage;
    byte *srctop,*srcbot,*dest;

    skypage = startskytex + (USE_PARALLAX - 1);

    for (i = 0; i < USE_PARALLAX; i++)
    {
        srctop = PM_GetPage(skypage - i);
        srcbot = PM_GetPage(skypage + USE_PARALLAX - i);

        dest = skytextures[i];

        for (y = 0; y < TEXTURESIZE; y++)
        {
            x = 0;

            while (x < TEXTURESIZE)
            {
                *dest++ = *srctop++;
                x++;
            }

            while (x < SKYTEXTURESIZE)
            {
                *dest++ = *srcbot++;
                x++;
            }
        }
    }
}
#endif

#ifdef USE_FEATUREFLAGS

// The lower left tile of every map determines the start texture of the parallax sky.
void SetParallaxStartTexture (void)
{
    startskytex = ffDataBottomLeft;

    assert(startskytex >= 0 && startskytex < PMSpriteStart - (USE_PARALLAX - 1));

#ifdef USE_COMPOSITEPARALLAX
    MakeSky ();
#endif
}

#else

void SetParallaxStartTexture (void)
{
    switch (gamestate.episode * 10 + gamestate.mapon)
    {
        case  0: startskytex = 20; break;
        default: startskytex =  0; break;
    }

    assert(startskytex >= 0 && startskytex < PMSpriteStart - (USE_PARALLAX - 1));

#ifdef USE_COMPOSITEPARALLAX
    MakeSky ();
#endif
}

#endif

/*
====================
=
= DrawParallax
=
====================
*/

void DrawParallax (void)
{
    int   x,y;
    byte  *dest,*skysource;
    int   texture,texoffs;
    int   angle;
    int   skypage;
    fixed frac,fracstep;

    skypage = startskytex + (USE_PARALLAX - 1);

    for (x = 0; x < viewwidth; x++)
    {
        y = centery - (wallheight[x] >> 3);

        if (y <= 0)
            continue;                // nothing to draw

        angle = pixelangle[x] + midangle;

        if (angle < 0)
            angle += FINEANGLES;
        else if (angle >= FINEANGLES)
            angle -= FINEANGLES;

        texture = ((angle * USE_PARALLAX) << TEXTURESHIFT) / FINEANGLES;

        texoffs = ((texture << SKYTEXTURESHIFT) & SKYTEXTUREMASK) ^ SKYTEXTUREMASK;
#ifdef USE_COMPOSITEPARALLAX
        skysource = &skytextures[texture >> TEXTURESHIFT][texoffs];
#else
        skysource = PM_GetPage(skypage - (texture >> TEXTURESHIFT)) + texoffs;
#endif
        dest = vbuf + x;

        fracstep = 0xffffffff / (viewheight << (FRACBITS - SKYTEXTURESHIFT - 1));
        frac = 0;

        while (y--)
        {
            *dest = skysource[(frac >> FRACBITS) & (SKYTEXTURESIZE - 1)];
            dest += bufferPitch;
            frac += fracstep;
        }
    }
}

#endif
