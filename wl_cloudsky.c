// WL_CLOUDSKY.C

#include "version.h"

#ifdef USE_CLOUDSKY

#include "wl_def.h"
#include "wl_cloudsky.h"


/*
=============================================================================

                            GLOBAL VARIABLES

=============================================================================
*/


/*
==================================
=
= Each colormap defines a number of colors which should be mapped from
= the skytable. The according colormapentry_t array defines how these colors should
= be mapped to the game palette. The first number in each entry defines
= how many colors are grouped to this entry and the absolute value of the
= second number sets the starting palette index for this pair. If this value is
= negative the index will be decremented for every color, if it's positive
= it will be incremented.
=
= Example colormap:
=
=   colormapentry_t colmapents_1[] = { { 6, -10 }, { 2, 40 } };
=
=   colormap_t colorMaps[] = { { 8, colmapents_1 } };
=
=   The colormap 0 consists of 8 colors. The first color group consists of 6
=   colors and starts descending at palette index 10: 10, 9, 8, 7, 6, 5
=   The second color group consists of 2 colors and starts ascending at
=   index 40: 40, 41
=   There's no other color group because all colors of this colormap are
=   already used (6 + 2 = 8)
=
= REMEMBER: Always make sure that the sum of the amount of the colors in all
= --------  color groups is the number of colors used for your colormap!
=
====================================================================================
*/

colormapentry_t colmapents_1[] = { { 16, -31 }, { 16, 136 }, };
colormapentry_t colmapents_2[] = { { 16, -31 }, };

colormap_t colorMaps[] =
{
    { 32, colmapents_1 },
    { 16, colmapents_2 },
};

const int numColorMaps = lengthof(colorMaps);

//
// The sky definitions which can be selected as defined by GetCloudSkyDefID() in wl_def.h
// You can use <TAB>+Z in debug mode to find out suitable values for seed and colorMapIndex
// Each entry consists of seed, speed, angle and colorMapIndex
//
cloudsky_t cloudSkys[] =
{
    { 626,   800,  20,  0 },
    { 1234,  650,  60,  1 },
    { 0,     700,  120, 0 },
    { 0,     0,    0,   0 },
    { 11243, 750,  310, 0 },
    { 32141, 750,  87,  0 },
    { 12124, 750,  64,  0 },
    { 55543, 500,  240, 0 },
    { 65535, 200,  54,  1 },
    { 4,     1200, 290, 0 },
};

byte       skyc[65536L];

fixed      cloudx,cloudy;
cloudsky_t *curSky;

#ifdef USE_FEATUREFLAGS

// The lower left tile of every map determines the used cloud sky definition from cloudSkys.
int GetCloudSkyDefID (void)
{
    int skyID = ffDataBottomLeft;

    assert(skyID >= 0 && skyID < lengthof(cloudSkys));

    return skyID;
}

#else

int GetCloudSkyDefID (void)
{
    int skyID;

    switch(gamestate.episode * 10 + gamestate.mapon)
    {
        case  0: skyID =  0; break;
        case  1: skyID =  1; break;
        case  2: skyID =  2; break;
        case  3: skyID =  3; break;
        case  4: skyID =  4; break;
        case  5: skyID =  5; break;
        case  6: skyID =  6; break;
        case  7: skyID =  7; break;
        case  8: skyID =  8; break;
        case  9: skyID =  9; break;
        default: skyID =  9; break;
    }

    assert(skyID >= 0 && skyID < lengthof(cloudSkys));

    return skyID;
}

#endif

void SplitS(unsigned size,unsigned x1,unsigned y1,unsigned x2,unsigned y2)
{
    //
    // I don't even want to touch this abomination... O_o
    //
   if(size==1) return;
   if(!skyc[((x1+size/2)*256+y1)])
   {
      skyc[((x1+size/2)*256+y1)]=(byte)(((int)skyc[(x1*256+y1)]
            +(int)skyc[((x2&0xff)*256+y1)])/2)+rand()%(size*2)-size;
      if(!skyc[((x1+size/2)*256+y1)]) skyc[((x1+size/2)*256+y1)]=1;
   }
   if(!skyc[((x1+size/2)*256+(y2&0xff))])
   {
      skyc[((x1+size/2)*256+(y2&0xff))]=(byte)(((int)skyc[(x1*256+(y2&0xff))]
            +(int)skyc[((x2&0xff)*256+(y2&0xff))])/2)+rand()%(size*2)-size;
      if(!skyc[((x1+size/2)*256+(y2&0xff))])
         skyc[((x1+size/2)*256+(y2&0xff))]=1;
   }
   if(!skyc[(x1*256+y1+size/2)])
   {
      skyc[(x1*256+y1+size/2)]=(byte)(((int)skyc[(x1*256+y1)]
            +(int)skyc[(x1*256+(y2&0xff))])/2)+rand()%(size*2)-size;
      if(!skyc[(x1*256+y1+size/2)]) skyc[(x1*256+y1+size/2)]=1;
   }
   if(!skyc[((x2&0xff)*256+y1+size/2)])
   {
      skyc[((x2&0xff)*256+y1+size/2)]=(byte)(((int)skyc[((x2&0xff)*256+y1)]
            +(int)skyc[((x2&0xff)*256+(y2&0xff))])/2)+rand()%(size*2)-size;
      if(!skyc[((x2&0xff)*256+y1+size/2)]) skyc[((x2&0xff)*256+y1+size/2)]=1;
   }

   skyc[((x1+size/2)*256+y1+size/2)]=(byte)(((int)skyc[(x1*256+y1)]
         +(int)skyc[((x2&0xff)*256+y1)]+(int)skyc[(x1*256+(y2&0xff))]
         +(int)skyc[((x2&0xff)*256+(y2&0xff))])/4)+rand()%(size*2)-size;

   SplitS(size/2,x1,y1+size/2,x1+size/2,y2);
   SplitS(size/2,x1+size/2,y1,x2,y1+size/2);
   SplitS(size/2,x1+size/2,y1+size/2,x2,y2);
   SplitS(size/2,x1,y1,x1+size/2,y1+size/2);
}


/*
====================
=
= InitSky
=
= Called from PlayLoop
=
====================
*/

void InitSky (void)
{
    colormapentry_t *curEntry;
    colormap_t      *curMap;
    byte            colormap[256];
    int             i,j,k,m,n,calcedCols;
    int             cloudskyid = GetCloudSkyDefID();
    int16_t         index;
    int32_t         value;

    if(cloudskyid >= lengthof(cloudSkys))
        Quit("Illegal cloud sky id: %u", cloudskyid);

    curSky = &cloudSkys[cloudskyid];

    cloudx = cloudy = 0;                             // reset cloud position
    memset (skyc,0,sizeof(skyc));
    // funny water texture if used instead of memset ;D
    // for (i = 0; i < 65536; i++)
    //     skyc[i] = rand() % 32 * 8;

    srand (curSky->seed);
    skyc[0] = rand() % 256;
    SplitS (256,0,0,256,256);

    //
    // smooth the clouds a bit
    //
    for (k = 0; k < 2; k++)
    {
        for (i = 0; i < 256; i++)
        {
            for (j = 0; j < 256; j++)
            {
                value = -skyc[(j << 8) + i];

                for (m = 0; m < 3; m++)
                {
                    for (n = 0; n < 3; n++)
                        value += skyc[(((j + n - 1) & 0xff) << 8) + ((i + m - 1) & 0xff)];
                }

                skyc[(j << 8) + i] = (byte)(value >> 3);
            }
        }
    }

    //
    // the following commented lines could be useful if you're trying to
    // create a new color map. This will display your current color map
    // in one repeating strip of the sky
    //
    // for (i = 0; i < 256; i++)
    //     skyc[i] = skyc[i + 256] = skyc[i + 512] = i;

    if (curSky->colorMapIndex >= numColorMaps)
        Quit ("Illegal colorMapIndex for cloud sky def %u: %u",cloudskyid,curSky->colorMapIndex);

    curMap = &colorMaps[curSky->colorMapIndex];
    curEntry = curMap->entries;

    for (calcedCols = 0; calcedCols < curMap->numColors; curEntry++)
    {
        if (curEntry->startAndDir < 0)
        {
            for (i = 0, index = -curEntry->startAndDir; i < curEntry->length; i++, index--)
            {
                if (index < 0)
                    index = 0;                       // don't go below the first color

                colormap[calcedCols++] = index;
            }
        }
        else
        {
            for (i = 0, index = curEntry->startAndDir; i < curEntry->length; i++, index++)
            {
                if (index > 255)
                    index = 255;                     // don't go above the last color

                colormap[calcedCols++] = index;
            }
        }
    }

    for (j = 0; j < 256; j++)
    {
        for (i = 0; i < 256; i++)
            skyc[(j << 8) + i] = colormap[(skyc[(j << 8) + i] * curMap->numColors) >> 8];
    }
}


/*
===================
=
= DrawCloudSpan
=
= Height ranges from 0 (infinity) to [centery] (nearest)
=
===================
*/

void DrawCloudSpan (int16_t x1, int16_t x2, int16_t height)
{
    byte     *dest;
    word     texture;
    int16_t  count,prestep;
    fixed    basedist;
    fixed    stepscale;
    fixed    xfrac,yfrac;
    fixed    xstep,ystep;

    count = x2 - x1;

    if (!count)
        return;                                             // nothing to draw

    dest = vbuf + ylookup[centery - 1 - height] + x1;

    prestep = centerx - x1 + 1;
    basedist = FixedDiv(scale,height) << 2;                 // distance to row projection
    stepscale = basedist / scale;

    xstep = FixedMul(stepscale,viewsin);
    ystep = -FixedMul(stepscale,viewcos);

    xfrac = (viewx + FixedMul(basedist,viewcos) + cloudx) - (xstep * prestep);
    yfrac = -(viewy - FixedMul(basedist,viewsin) - cloudy) - (ystep * prestep);

    while (count--)
    {
        texture = ((xfrac >> 5) & 0xff00) + (~(yfrac >> 13) & 0xff);

        *dest++ = skyc[texture];

        xfrac += xstep;
        yfrac += ystep;
    }
}


/*
===================
=
= DrawCloudPlanes
=
===================
*/

void DrawCloudPlanes (void)
{
    int     x,y;
    int16_t	height;
    int32_t speed;

    speed = curSky->speed * tics;

    cloudx += FixedMul(speed,sintable[curSky->angle]);
    cloudy -= FixedMul(speed,costable[curSky->angle]);

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
                    DrawCloudSpan (spanstart[y],x,y);

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
            DrawCloudSpan (spanstart[y],viewwidth,y);

        y++;
    }
}

#endif
