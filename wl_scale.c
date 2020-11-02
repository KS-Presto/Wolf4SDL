// WL_SCALE.C

#include "wl_def.h"

#ifdef USE_SHADING
#include "wl_shade.h"
#endif


/*
=============================================================================

                              SPRITE DRAWING ROUTINES

=============================================================================
*/


/*
===================
=
= ScaleLine
=
= Reconstruct a sprite and draw it
=
= each vertical line of the shape has a pointer to segment data:
= 	end of segment pixel*2 (0 terminates line)
= 	top of virtual line with segment in proper place
=	start of segment pixel*2
=	<repeat>
=
===================
*/

void ScaleLine (int16_t x, int16_t toppix, fixed fracstep, byte *linesrc, byte *linecmds, byte *curshades)
{
    byte    *src,*dest;
    byte    col;
    int16_t start,end,top;
    int16_t startpix,endpix;
    fixed   frac;

    for (end = READWORD(linecmds) >> 1; end; end = READWORD(linecmds) >> 1)
    {
        top = READWORD(linecmds + 2);
        start = READWORD(linecmds + 4) >> 1;

        frac = start * fracstep;

        endpix = (frac >> FRACBITS) + toppix;

        for (src = &linesrc[top + start]; start != end; start++, src++)
        {
            startpix = endpix;

            if (startpix >= viewheight)
                break;                          // off the bottom of the view area

            frac += fracstep;
            endpix = (frac >> FRACBITS) + toppix;

            if (endpix < 0)
                continue;                       // not into the view area

            if (startpix < 0)
                startpix = 0;                   // clip upper boundary

            if (endpix > viewheight)
                endpix = viewheight;            // clip lower boundary

#ifdef USE_SHADING
            if (curshades)
                col = curshades[*src];
            else
#endif
                col = *src;

            dest = vbuf + ylookup[startpix] + x;

            while (startpix < endpix)
            {
                *dest = col;
                dest += bufferPitch;
                startpix++;
            }
        }

        linecmds += 6;                          // next segment list
    }
}


/*
===================
=
= ScaleShape
=
= Draws a compiled shape at [scale] pixels high
=
===================
*/

void ScaleShape (int xcenter, int shapenum, int height, uint32_t flags)
{
    int         i;
    compshape_t *shape;
    byte        *linesrc,*linecmds;
    byte        *curshades;
    int16_t     scale,toppix;
    int16_t     x1,x2,actx;
    fixed       frac,fracstep;

    scale = height >> 3;        // low three bits are fractional

    if (!scale)
        return;                 // too close or far away

    linesrc = PM_GetSpritePage(shapenum);
    shape = (compshape_t *)linesrc;

#ifdef USE_SHADING
    if (flags & FL_FULLBRIGHT)
        curshades = shadetable[0];
    else
        curshades = shadetable[GetShade(height)];
#endif

    fracstep = FixedDiv(scale,TEXTURESIZE/2);
    frac = shape->leftpix * fracstep;

    actx = xcenter - scale;
    toppix = centery - scale;

    x2 = (frac >> FRACBITS) + actx;

    for (i = shape->leftpix; i <= shape->rightpix; i++)
    {
        //
        // calculate edges of the shape
        //
        x1 = x2;

        if (x1 >= viewwidth)
            break;                // off the right side of the view area

        frac += fracstep;
        x2 = (frac >> FRACBITS) + actx;

        if (x2 < 0)
            continue;             // not into the view area

        if (x1 < 0)
            x1 = 0;               // clip left boundary

        if (x2 > viewwidth)
            x2 = viewwidth;       // clip right boundary

        while (x1 < x2)
        {
            if (wallheight[x1] < height)
            {
                linecmds = &linesrc[shape->dataofs[i - shape->leftpix]];

                ScaleLine (x1,toppix,fracstep,linesrc,linecmds,curshades);
            }

            x1++;
        }
    }
}


/*
===================
=
= SimpleScaleShape
=
= NO CLIPPING, height in pixels
=
= Draws a compiled shape at [scale] pixels high
=
===================
*/

void SimpleScaleShape (int xcenter, int shapenum, int height)
{
    int         i;
    compshape_t *shape;
    byte        *linesrc,*linecmds;
    int16_t     scale,toppix;
    int16_t     x1,x2,actx;
    fixed       frac,fracstep;

    scale = height >> 1;

    linesrc = PM_GetSpritePage(shapenum);
    shape = (compshape_t *)linesrc;

    fracstep = FixedDiv(scale,TEXTURESIZE/2);
    frac = shape->leftpix * fracstep;

    actx = xcenter - scale;
    toppix = centery - scale;

    x2 = (frac >> FRACBITS) + actx;

    for (i = shape->leftpix; i <= shape->rightpix; i++)
    {
        //
        // calculate edges of the shape
        //
        x1 = x2;

        frac += fracstep;
        x2 = (frac >> FRACBITS) + actx;

        while (x1 < x2)
        {
            linecmds = &linesrc[shape->dataofs[i - shape->leftpix]];

            ScaleLine (x1,toppix,fracstep,linesrc,linecmds,NULL);

            x1++;
        }
    }
}

#ifdef USE_DIR3DSPR

/*
===================
=
= Scale3DShape
=
= Draws a compiled 3D shape at [scale1] pixels high
=
= Aryan: No amount of optimisation in the world will make this code look good... Reeee-write!
=
===================
*/

void Scale3DShape (int x1, int x2, int shapenum, uint32_t flags, fixed ny1, fixed ny2, fixed nx1, fixed nx2)
{
    int         i;
    compshape_t *shape;
    byte        *linesrc,*linecmds;
    byte        *curshades;
    int16_t     scale1,toppix;
    int16_t     dx,len,slinex;
    int16_t     xpos[TEXTURESIZE + 1];
    fixed       height,dheight,height1,height2;
    fixed       fracstep;
    fixed       dxx,dzz;
    fixed       dxa,dza;

    linesrc = PM_GetSpritePage(shapenum);
    shape = (compshape_t *)linesrc;

    len = shape->rightpix - shape->leftpix + 1;

    if (!len)
        return;

    dxx = (ny2 - ny1) << 8;
    dzz = (nx2 - nx1) << 8;

    ny1 += dxx >> 9;
    nx1 += dzz >> 9;

    dxa = -(dxx >> 1);
    dza = -(dzz >> 1);
    dxx >>= TEXTURESHIFT;
    dzz >>= TEXTURESHIFT;
    dxa += shape->leftpix * dxx;
    dza += shape->leftpix * dzz;

    xpos[0] = (ny1 + (dxa >> 8)) * scale / (nx1 + (dza >> 8)) + centerx;
    height1 = heightnumerator / ((nx1 + (dza >> 8)) >> 8);
    height = (((fixed)height1) << 12) + 2048;

    for (i = 1; i <= len; i++)
    {
        dxa += dxx;
        dza += dzz;
        xpos[i] = (ny1 + (dxa >> 8)) * scale / (nx1 + (dza >> 8)) + centerx;

        if (xpos[i - 1] > viewwidth)
            break;
    }

    len = i - 1;
    dx = xpos[len] - xpos[0];

    if (!dx)
        return;

    height2 = heightnumerator / ((nx1 + (dza >> 8)) >> 8);
    dheight = (((fixed)height2 - (fixed)height1) << 12) / (fixed)dx;

    if (x2 > viewwidth)
        x2 = viewwidth;

    for (i = 0; i < len; i++)
    {
        for (slinex = xpos[i]; slinex < xpos[i + 1] && slinex < x2; slinex++)
        {
            height += dheight;

            if (slinex < 0)
                continue;

            scale1 = height >> 15;

            if (!scale1)
                continue;

            if (wallheight[slinex] < (height >> 12))
            {
#ifdef USE_SHADING
                if (flags & FL_FULLBRIGHT)
                    curshades = shadetable[0];
                else
                    curshades = shadetable[GetShade(scale1 << 3)];
#endif
                fracstep = FixedDiv(scale1,TEXTURESIZE/2);
                toppix = centery - scale1;

                linecmds = &linesrc[shape->dataofs[i]];

                ScaleLine (slinex,toppix,fracstep,linesrc,linecmds,curshades);
            }
        }
    }
}


/*
========================
=
= Transform3DShape
=
========================
*/

void Transform3DShape (statobj_t *statptr)
{
    #define SIZEADD 1024

    fixed nx1,nx2,ny1,ny2;
    int   viewx1,viewx2;
    fixed diradd;
    fixed gy1,gy2,gx,gyt1,gyt2,gxt;
    fixed gx1,gx2,gy,gxt1,gxt2,gyt;

    //
    // the following values for "diradd" aren't optimized yet
    // if you have problems with sprites being visible through wall edges
    // where they shouldn't, you can try to adjust these values and SIZEADD
    //
    switch (statptr->flags & FL_DIR_POS_MASK)
    {
        case FL_DIR_POS_FW: diradd = 0x7ff0 + 0x8000; break;
        case FL_DIR_POS_BW: diradd = -0x7ff0 + 0x8000; break;
        case FL_DIR_POS_MID: diradd = 0x8000; break;

        default:
            Quit ("Unknown directional 3d sprite position (shapenum = %i)",statptr->shapenum);
    }

    if (statptr->flags & FL_DIR_VERT_FLAG)
    {
        //
        // translate point to view centered coordinates
        //
        gy1 = (((fixed)statptr->tiley) << TILESHIFT) + 0x8000 - viewy - 0x8000L - SIZEADD;
        gy2 = gy1 + 0x10000L + (2 * SIZEADD);
        gx = (((fixed)statptr->tilex) << TILESHIFT) + diradd - viewx;

        //
        // calculate nx
        //
        gxt = FixedMul(gx,viewcos);
        gyt1 = FixedMul(gy1,viewsin);
        gyt2 = FixedMul(gy2,viewsin);
        nx1 = gxt - gyt1;
        nx2 = gxt - gyt2;

        //
        // calculate ny
        //
        gxt = FixedMul(gx,viewsin);
        gyt1 = FixedMul(gy1,viewcos);
        gyt2 = FixedMul(gy2,viewcos);
        ny1 = gyt1 + gxt;
        ny2 = gyt2 + gxt;
    }
    else
    {
        
        //
        // translate point to view centered coordinates
        //
        gx1 = (((fixed)statptr->tilex) << TILESHIFT) + 0x8000 - viewx - 0x8000L - SIZEADD;
        gx2 = gx1 + 0x10000L + (2 * SIZEADD);
        gy = (((fixed)statptr->tiley) << TILESHIFT) + diradd - viewy;

        //
        // calculate nx
        //
        gxt1 = FixedMul(gx1,viewcos);
        gxt2 = FixedMul(gx2,viewcos);
        gyt = FixedMul(gy,viewsin);
        nx1 = gxt1 - gyt;
        nx2 = gxt2 - gyt;

        //
        // calculate ny
        //
        gxt1 = FixedMul(gx1,viewsin);
        gxt2 = FixedMul(gx2,viewsin);
        gyt = FixedMul(gy,viewcos);
        ny1 = gyt + gxt1;
        ny2 = gyt + gxt2;
    }

    if (nx1 < 0 || nx2 < 0)
        return;              // TODO: Clip on viewplane

    //
    // calculate perspective ratio
    //
    if (nx1 >= 0 && nx1 <= 1792) nx1 = 1792;
    if (nx1 < 0 && nx1 >= -1792) nx1 = -1792;
    if (nx2 >= 0 && nx2 <= 1792) nx2 = 1792;
    if (nx2 < 0 && nx2 >= -1792) nx2 = -1792;

    viewx1 = (int)(centerx + ny1 * scale / nx1);
    viewx2 = (int)(centerx + ny2 * scale / nx2);

    if (viewx2 < viewx1)
        Scale3DShape (viewx2,viewx1,statptr->shapenum,statptr->flags,ny2,ny1,nx2,nx1);
    else
        Scale3DShape (viewx1,viewx2,statptr->shapenum,statptr->flags,ny1,ny2,nx1,nx2);
}

#endif
