// WL_PLANE.C

#include "version.h"

#ifdef USE_FLOORCEILINGTEX

#include "wl_def.h"
#include "wl_shade.h"

byte    *ceilingsource,*floorsource;

#ifndef USE_MULTIFLATS
void GetFlatTextures (void)
{
#ifdef USE_FEATUREFLAGS
    ceilingsource = PM_GetPage(ffDataBottomRight >> 8);
    floorsource = PM_GetPage(ffDataBottomRight & 0xff);
#else
    ceilingsource = PM_GetPage(0);
    floorsource = PM_GetPage(1);
#endif
}
#endif

/*
===================
=
= DrawSpan
=
= Height ranges from 0 (infinity) to [centery] (nearest)
=
= With multi-textured floors and ceilings stored in lower and upper bytes of
= according tilex/tiley in the third mapplane, respectively
=
===================
*/
#ifdef USE_MULTIFLATS
void DrawSpan (int16_t x1, int16_t x2, int16_t height)
{
    byte      tilex,tiley,lasttilex,lasttiley;
    byte      *dest;
    byte      *shade;
    word      texture,spot;
    uint32_t  rowofs;
    int16_t   ceilingpage,floorpage,lastceilingpage,lastfloorpage;
    int16_t   count,prestep;
    fixed     basedist,stepscale;
    fixed     xfrac,yfrac;
    fixed     xstep,ystep;

    count = x2 - x1;

    if (!count)
        return;                                                 // nothing to draw

#ifdef USE_SHADING
    shade = shadetable[GetShade(height << 3)];
#endif
    dest = vbuf + ylookup[centery - 1 - height] + x1;
    rowofs = ylookup[(height << 1) + 1];                        // toprow to bottomrow delta

    prestep = centerx - x1 + 1;
    basedist = FixedDiv(scale,height + 1) >> 1;                 // distance to row projection
    stepscale = basedist / scale;

    xstep = FixedMul(stepscale,viewsin);
    ystep = -FixedMul(stepscale,viewcos);

    xfrac = (viewx + FixedMul(basedist,viewcos)) - (xstep * prestep);
    yfrac = -(viewy - FixedMul(basedist,viewsin)) - (ystep * prestep);

//
// draw two spans simultaneously
//
    lastceilingpage = lastfloorpage = -1;
    lasttilex = lasttiley = 0;

    //
    // Beware - This loop is SLOW, and WILL cause framedrops on slower machines and/or on higher resolutions.
    // I can't stress it enough: this is one of the worst loops in history! No amount of optimisation will
    // change that! This loop SUCKS!
    //
    // Someday flats rendering will be re-written, but until then, YOU HAVE BEEN WARNED.
    //
    while (count--)
    {
        //
        // get tile coords of texture
        //
        tilex = (xfrac >> TILESHIFT) & (mapwidth - 1);
        tiley = ~(yfrac >> TILESHIFT) & (mapheight - 1);

        //
        // get floor & ceiling textures if it's a new tile
        //
        if (lasttilex != tilex || lasttiley != tiley)
        {
            lasttilex = tilex;
            lasttiley = tiley;
            spot = MAPSPOT(tilex,tiley,2);

            if (spot)
            {
                ceilingpage = spot >> 8;
                floorpage = spot & 0xff;
            }
        }

        if (spot)
        {
            texture = ((xfrac >> FIXED2TEXSHIFT) & TEXTUREMASK) + (~(yfrac >> (FIXED2TEXSHIFT + TEXTURESHIFT)) & (TEXTURESIZE - 1));

            //
            // write ceiling pixel
            //
            if (ceilingpage)
            {
                if (lastceilingpage != ceilingpage)
                {
                    lastceilingpage = ceilingpage;
                    ceilingsource = PM_GetPage(ceilingpage);
                }
#ifdef USE_SHADING
                *dest = shade[ceilingsource[texture]];
#else
                *dest = ceilingsource[texture];
#endif
            }

            //
            // write floor pixel
            //
            if (floorpage)
            {
                if (lastfloorpage != floorpage)
                {
                    lastfloorpage = floorpage;
                    floorsource = PM_GetPage(floorpage);
                }
#ifdef USE_SHADING
                dest[rowofs] = shade[floorsource[texture]];
#else
                dest[rowofs] = floorsource[texture];
#endif
            }
        }

        dest++;

        xfrac += xstep;
        yfrac += ystep;
    }
}

#else

/*
===================
=
= DrawSpan
=
= Height ranges from 0 (infinity) to [centery] (nearest)
=
===================
*/

void DrawSpan (int16_t x1, int16_t x2, int16_t height)
{
    byte     *dest;
    byte     *shade;
    word     texture;
    uint32_t rowofs;                                    
    int16_t  count,prestep;
    fixed    basedist,stepscale;
    fixed    xfrac,yfrac;
    fixed    xstep,ystep;

    count = x2 - x1;

    if (!count)
        return;                                         // nothing to draw

#ifdef USE_SHADING
    shade = shadetable[GetShade(height << 3)];
#endif
    dest = vbuf + ylookup[centery - 1 - height] + x1;
    rowofs = ylookup[(height << 1) + 1];                // toprow to bottomrow delta

    prestep = centerx - x1 + 1;
    basedist = FixedDiv(scale,height + 1) >> 1;         // distance to row projection
    stepscale = basedist / scale;

    xstep = FixedMul(stepscale,viewsin);
    ystep = -FixedMul(stepscale,viewcos);

    xfrac = (viewx + FixedMul(basedist,viewcos)) - (xstep * prestep);
    yfrac = -(viewy - FixedMul(basedist,viewsin)) - (ystep * prestep);

//
// draw two spans simultaneously
//
	while (count--)
	{
		texture = ((xfrac >> FIXED2TEXSHIFT) & TEXTUREMASK) + (~(yfrac >> (FIXED2TEXSHIFT + TEXTURESHIFT)) & (TEXTURESIZE - 1));

#ifdef USE_SHADING
        *dest = shade[ceilingsource[texture]];
        dest[rowofs] = shade[floorsource[texture]];
#else
        *dest = ceilingsource[texture];
        dest[rowofs] = floorsource[texture];
#endif
		dest++;
		xfrac += xstep;
		yfrac += ystep;
	}
}
#endif

/*
===================
=
= DrawPlanes
=
===================
*/

void DrawPlanes (void)
{
    int     x,y;
    int16_t	height;

//
// loop over all columns
//
    y = centery;

    for (x = 0; x < viewwidth; x++)
    {
        height = wallheight[x] >> 3;

        if (height < y)
        {
            //
            // more starts
            //
            while (y > height)
                spanstart[--y] = x;
        }
        else if (height > y)
        {
            //
            // draw clipped spans
            //
            if (height > centery)
                height = centery;

            while (y < height)
            {
                if (y > 0)
                    DrawSpan (spanstart[y],x,y);

                y++;
            }
        }
    }

    //
    // draw spans
    //
    height = centery;

    while (y < height)
    {
        if (y > 0)
            DrawSpan (spanstart[y],viewwidth,y);

        y++;
    }
}

#endif
