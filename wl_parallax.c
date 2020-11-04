// WL_PARALLAX.C

#include "version.h"

#ifdef USE_PARALLAX

#include "wl_def.h"

#ifdef USE_FEATUREFLAGS

// The lower left tile of every map determines the start texture of the parallax sky.
int GetParallaxStartTexture (void)
{
    int startTex = ffDataBottomLeft;

    assert(startTex >= 0 && startTex < PMSpriteStart);

    return startTex;
}

#else

int GetParallaxStartTexture (void)
{
    int startTex;

    switch (gamestate.episode * 10 + gamestate.mapon)
    {
        case  0: startTex = 20; break;
        default: startTex =  0; break;
    }

    assert(startTex >= 0 && startTex < PMSpriteStart);

    return startTex;
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
    int     x,y;
    byte    *dest,*skysource;
    word    texture;
    int16_t angle;
    int16_t skypage,curskypage;
    int16_t lastskypage;
    int16_t xtex;
    int16_t toppix;

    skypage = GetParallaxStartTexture();
    skypage += USE_PARALLAX - 1;
    lastskypage = -1;

    for (x = 0; x < viewwidth; x++)
    {
        toppix = centery - (wallheight[x] >> 3);

        if (toppix <= 0)
            continue;                // nothing to draw

        angle = pixelangle[x] + midangle;

        if (angle < 0)
            angle += FINEANGLES;
        else if (angle >= FINEANGLES)
            angle -= FINEANGLES;

        xtex = ((angle * USE_PARALLAX) << TEXTURESHIFT) / FINEANGLES;
        curskypage = xtex >> TEXTURESHIFT;

        if (lastskypage != curskypage)
        {
            lastskypage = curskypage;
            skysource = PM_GetPage(skypage - curskypage);
        }

        texture = TEXTUREMASK - ((xtex & (TEXTURESIZE - 1)) << TEXTURESHIFT);

        for (y = 0, dest = &vbuf[x]; y < toppix; y++, dest += bufferPitch)
            *dest = skysource[texture + ((y << TEXTURESHIFT) / centery)];
    }
}

#endif
