// WL_ATMOS.H

#include "version.h"

#if defined(USE_STARSKY) || defined(USE_RAIN) || defined(USE_SNOW)

#include "wl_def.h"

#if defined(USE_RAIN) || defined(USE_SNOW)
uint32_t rainpos;
#endif

typedef struct
{
    fixed x,y,z;
} point3d_t;

#define MAXPOINTS 400
point3d_t points[MAXPOINTS];

byte moon[100] =
{
     0,  0, 27, 18, 15, 16, 19, 29,  0,  0,
     0, 22, 16, 15, 15, 16, 16, 18, 24,  0,
    27, 17, 15, 17, 16, 16, 17, 17, 18, 29,
    18, 15, 15, 15, 16, 16, 17, 17, 18, 20,
    16, 15, 15, 16, 16, 17, 17, 18, 19, 21,
    16, 15, 17, 20, 18, 17, 18, 18, 20, 22,
    19, 16, 18, 19, 17, 17, 18, 19, 22, 24,
    28, 19, 17, 17, 17, 18, 19, 21, 25, 31,
     0, 23, 18, 19, 18, 20, 22, 24, 28,  0,
     0,  0, 28, 21, 20, 22, 28, 30,  0,  0,
};


/*
====================
=
= Init3DPoints
=
====================
*/

void Init3DPoints (void)
{
    int       i,j;
    float     length;
    point3d_t *pt;

    for (i = 0; i < MAXPOINTS; i++)
    {
        pt = &points[i];

        pt->x = 16384 - (rand() & 32767);
        pt->z = 16384 - (rand() & 32767);

        length = sqrt((float)pt->x * pt->x + (float)pt->z * pt->z);
        j = 50;

        do
        {
            pt->y = 1024 + (rand() & 8191);
            j--;

        } while (j > 0 && ((float)pt->y * 256.F) / length >= centery);
    }
}

#endif

#ifdef USE_STARSKY

/*
====================
=
= DrawStarSky
=
====================
*/

void DrawStarSky (void)
{
    int       i,j;
    point3d_t *pt;
    byte      *dest;
    byte      shade;
    int16_t   stopx,starty,stopy;
    fixed     x,y,z;
    fixed     xx,yy;
    
    dest = vbuf;
    
    for (i = 0; i < centery; i++, dest += bufferPitch)
        memset (dest,0,viewwidth);

    for (i = 0; i < MAXPOINTS; i++)
    {
        pt = &points[i];

        x = pt->x * viewcos + pt->z * viewsin;
        y = pt->y << 16;
        z = (pt->z * viewcos - pt->x * viewsin) >> 8;

        if (z <= 0)
            continue;

        shade = (byte)(z >> 18);

        if (shade > 15)
            continue;

        xx = ((x / z) * scaleFactor) + (centerx + 1);
        yy = centery - (y / z);

        if (xx >= 0 && xx < viewwidth && yy >= 0 && yy < centery)
            vbuf[ylookup[yy] + xx] = shade + 15;
    }

    x = (16384 * viewcos) + (16384 * viewsin);
    z = ((16384 * viewcos) - (16384 * viewsin)) >> 8;

    if (z <= 0)
        return;

    xx = ((x / z) * scaleFactor) + (centerx + 1);
    yy = centery - (((centery - (centery >> 3)) << 22) / z);

    if (xx > (scaleFactor * -10) && xx < viewwidth) 
    { 
        stopx = 10 * scaleFactor;
        starty = 0;
        stopy = 10 * scaleFactor;
        i = 0; 

        if (xx < 0)
            i = -xx; 
        if (xx >= viewwidth - (10 * scaleFactor))
            stopx = viewwidth - xx; 

        if (yy < 0)
            starty = -yy;
        if (yy >= viewheight - (10 * scaleFactor))
            stopy = viewheight - yy;

        while (i < stopx)
        {
            for (j = starty; j < stopy; j++) 
                vbuf[ylookup[yy + j] + xx + i] = moon[((j / scaleFactor) * 10) + (i / scaleFactor)];

            i++;
        }
    } 
}

#endif

#ifdef USE_RAIN

/*
====================
=
= DrawRain
=
====================
*/

void DrawRain (void)
{
#if defined(USE_FLOORCEILINGTEX) && defined(FIXRAINSNOWLEAKS)
    byte      tilex,tiley;
    int16_t   prestep;
    fixed     basedist,stepscale;
    fixed     xfrac,yfrac;
    fixed     xstep,ystep;
#endif

    int       i;
    point3d_t *pt;
    byte      shade;
    int32_t   ax,az,x,y,z,xx,yy,height,actheight;
    fixed     px,pz;

    px = (player->y + FixedMul(0x7900, viewsin)) >> 6;
    pz = (player->x - FixedMul(0x7900, viewcos)) >> 6;

    rainpos -= tics * 900;

    for (i = 0; i < MAXPOINTS; i++)
    {
        pt = &points[i];

        ax = pt->x + px;
        ax = 0x1fff - (ax & 0x3fff);
        az = pt->z + pz;
        az = 0x1fff - (az & 0x3fff);
        x = (ax * viewcos) + (az * viewsin);
        y = -(heightnumerator << 7) + ((((pt->y << 6) + rainpos) & 0x0ffff) << 11);
        z = ((az * viewcos) - (ax * viewsin)) >> 8;

        if (z <= 0)
            continue;

        shade = (byte)(z >> 17);

        if (shade > 13)
            continue;

        xx = (x / z) + (centerx + 1);

        if (xx < 0 || xx >= viewwidth)
            continue;

        actheight = y / z;
        yy = centery - actheight;
        height = (heightnumerator << 10) / z;

        if (actheight < 0)
            actheight = -actheight;
        if (actheight < (wallheight[xx] >> 3) && height < wallheight[xx])
            continue;

        if (xx >= 0 && xx < viewwidth && yy > 0 && yy < viewheight)
        {
#if defined(USE_FLOORCEILINGTEX) && defined(FIXRAINSNOWLEAKS)
            //
            // Find the rain's tile coordinate
            // NOTE: This sometimes goes over the map edges
            //
            prestep = centerx - xx + 1;
            basedist = FixedDiv(scale,(height >> 3) + 1) >> 1;
            stepscale = basedist / scale;

            xstep = FixedMul(stepscale,viewsin);
            ystep = -FixedMul(stepscale,viewcos);

            xfrac = (viewx + FixedMul(basedist,viewcos)) - (xstep * prestep);
            yfrac = -(viewy - FixedMul(basedist,viewsin)) - (ystep * prestep);

            tilex = (xfrac >> TILESHIFT) & (mapwidth - 1);
            tiley = ~(yfrac >> TILESHIFT) & (mapheight - 1);

            //
            // is there a ceiling tile?
            //
            if (MAPSPOT(tilex,tiley, 2) >> 8)
                continue;
#endif

            vbuf[ylookup[yy] + xx] = shade + 15;
            vbuf[ylookup[yy - 1] + xx] = shade + 16;

            if (yy > 2)
                vbuf[ylookup[yy - 2] + xx] = shade + 17;
        }
    }
}

#endif

#ifdef USE_SNOW

/*
====================
=
= DrawSnow
=
====================
*/

void DrawSnow (void)
{
#if defined(USE_FLOORCEILINGTEX) && defined(FIXRAINSNOWLEAKS)
    byte      tilex,tiley;
    int16_t   prestep;
    fixed     basedist,stepscale;
    fixed     xfrac,yfrac;
    fixed     xstep,ystep;
#endif

    int       i;
    point3d_t *pt;
    byte      shade;
    int32_t   ax,az,x,y,z,xx,yy,height,actheight;
    fixed     px,pz;
    
    px = (player->y + FixedMul(0x7900, viewsin)) >> 6;
    pz = (player->x - FixedMul(0x7900, viewcos)) >> 6;
    
    rainpos -= tics * 256;

    for (i = 0; i < MAXPOINTS; i++)
    {
        pt = &points[i];

        ax = pt->x + px;
        ax = 0x1fff - (ax & 0x3fff);
        az = pt->z + pz;
        az = 0x1fff - (az & 0x3fff);
        x = (ax * viewcos) + (az * viewsin);
        y = -(heightnumerator << 7) + ((((pt->y << 6) + rainpos) & 0x0ffff) << 11);
        z = ((az * viewcos) - (ax * viewsin)) >> 8;

        if (z <= 0)
            continue;

        shade = (byte)(z >> 17);

        if (shade > 13)
            continue;

        xx = (x / z) + (centerx + 1);

        if (xx < 0 || xx >= viewwidth)
            continue;

        actheight = y / z;
        yy = centery - actheight;
        height = (heightnumerator << 10) / z;

        if (actheight < 0)
            actheight = -actheight;
        if (actheight < (wallheight[xx] >> 3) && height < wallheight[xx])
            continue;

        if (xx > 0 && xx < viewwidth && yy > 0 && yy < viewheight)
        {
#if defined(USE_FLOORCEILINGTEX) && defined(FIXRAINSNOWLEAKS)
            //
            // Find the snow's tile coordinate
            // NOTE: This sometimes goes over the map edges
            //
            prestep = centerx - xx + 1;
            basedist = FixedDiv(scale,(height >> 3) + 1) >> 1;
            stepscale = basedist / scale;

            xstep = FixedMul(stepscale,viewsin);
            ystep = -FixedMul(stepscale,viewcos);

            xfrac = (viewx + FixedMul(basedist,viewcos)) - (xstep * prestep);
            yfrac = -(viewy - FixedMul(basedist,viewsin)) - (ystep * prestep);

            tilex = (xfrac >> TILESHIFT) & (mapwidth - 1);
            tiley = ~(yfrac >> TILESHIFT) & (mapheight - 1);

            //
            // is there a ceiling tile?
            //
            if (MAPSPOT(tilex,tiley,2) >> 8)
                continue;
#endif

            if (shade < 10)
            {
                vbuf[ylookup[yy] + xx] = shade + 17;
                vbuf[ylookup[yy] + xx - 1] = shade + 16;
                vbuf[ylookup[yy - 1] + xx] = shade + 16;
                vbuf[ylookup[yy - 1] + xx - 1] = shade + 15;
            }
            else
                vbuf[ylookup[yy] + xx] = shade + 15;
        }
    }
}

#endif
