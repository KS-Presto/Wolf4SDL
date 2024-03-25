#include "wl_def.h"


pictabletype	*pictable;

int	    px,py;
byte	fontcolor,backcolor;
int	    fontnumber;

//==========================================================================

void VWB_DrawPropString(const char* string)
{
	fontstruct  *font;
	int		    width, step, height;
	byte	    *source, *dest;
	byte	    ch;
	int i;
	int sx, sy;

	dest = VL_LockSurface(screenBuffer);
	if(dest == NULL) return;

	font = (fontstruct *) grsegs[STARTFONT+fontnumber];
	height = font->height;
	dest += scaleFactor * (ylookup[py] + px);

	while ((ch = (byte)*string++)!=0)
	{
		width = step = font->width[ch];
		source = ((byte *)font)+font->location[ch];
		while (width--)
		{
			for(i=0; i<height; i++)
			{
				if(source[i*step])
				{
					for(sy=0; sy<scaleFactor; sy++)
						for(sx=0; sx<scaleFactor; sx++)
							dest[ylookup[scaleFactor*i+sy]+sx]=fontcolor;
				}
			}

			source++;
			px++;
			dest+=scaleFactor;
		}
	}

	VL_UnlockSurface(screenBuffer);
}


void VWL_MeasureString (const char *string, word *width, word *height, fontstruct *font)
{
	*height = font->height;
	for (*width = 0;*string;string++)
		*width += font->width[*((byte *)string)];	// proportional width
}

void VW_MeasurePropString (const char *string, word *width, word *height)
{
	VWL_MeasureString(string,width,height,(fontstruct *)grsegs[STARTFONT+fontnumber]);
}

/*
=============================================================================

				Double buffer management routines

=============================================================================
*/

void VH_UpdateScreen (SDL_Surface *surface)
{
	SDL_BlitSurface (surface,NULL,screen,NULL);

    SDL_UpdateTexture(texture, NULL, screen->pixels, screenPitch);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void VWB_DrawTile8 (int x, int y, int tile)
{
	VL_MemToScreen (grsegs[STARTTILE8]+tile*64,8,8,x,y);
}

void VWB_DrawPic (int x, int y, int chunknum)
{
	int	picnum = chunknum - STARTPICS;
	unsigned width,height;

	x &= ~7;

	width = pictable[picnum].width;
	height = pictable[picnum].height;

	VL_MemToScreen (grsegs[chunknum],width,height,x,y);
}

void VWB_DrawPicScaledCoord (int scx, int scy, int chunknum)
{
	int	picnum = chunknum - STARTPICS;
	unsigned width,height;

	width = pictable[picnum].width;
	height = pictable[picnum].height;

    VL_MemToScreenScaledCoord (grsegs[chunknum],width,height,scx,scy);
}


void VWB_Bar (int x, int y, int width, int height, int color)
{
	VW_Bar (x,y,width,height,color);
}

void VWB_Plot (int x, int y, int color)
{
    if(scaleFactor == 1)
        VW_Plot(x,y,color);
    else
        VW_Bar(x, y, 1, 1, color);
}

void VWB_Hlin (int x1, int x2, int y, int color)
{
    if(scaleFactor == 1)
    	VW_Hlin(x1,x2,y,color);
    else
        VW_Bar(x1, y, x2-x1+1, 1, color);
}

void VWB_Vlin (int y1, int y2, int x, int color)
{
    if(scaleFactor == 1)
		VW_Vlin(y1,y2,x,color);
    else
        VW_Bar(x, y1, 1, y2-y1+1, color);
}


/*
=============================================================================

						WOLFENSTEIN STUFF

=============================================================================
*/


/*
===================
=
= FizzleFade
=
= returns true if aborted
=
= It uses maximum-length Linear Feedback Shift Registers (LFSR) counters.
= You can find a list of them with lengths from 3 to 168 at:
= http://www.xilinx.com/support/documentation/application_notes/xapp052.pdf
= Many thanks to Xilinx for this list!!!
=
===================
*/

// XOR masks for the pseudo-random number sequence starting with n=17 bits
static const uint32_t rndmasks[] = {
                    // n    XNOR from (starting at 1, not 0 as usual)
    0x00012000,     // 17   17,14
    0x00020400,     // 18   18,11
    0x00040023,     // 19   19,6,2,1
    0x00090000,     // 20   20,17
    0x00140000,     // 21   21,19
    0x00300000,     // 22   22,21
    0x00420000,     // 23   23,18
    0x00e10000,     // 24   24,23,22,17
    0x01200000,     // 25   25,22      (this is enough for 8191x4095)
};

static unsigned int rndbits_y;
static unsigned int rndmask;

extern SDL_Color curpal[256];

// Returns the number of bits needed to represent the given value
static int log2_ceil(uint32_t x)
{
    int n = 0;
    uint32_t v = 1;
    while(v < x)
    {
        n++;
        v <<= 1;
    }
    return n;
}

void VH_Startup()
{
    int rndbits_x = log2_ceil(screenWidth);
    rndbits_y = log2_ceil(screenHeight);

    int rndbits = rndbits_x + rndbits_y;
    if(rndbits < 17)
        rndbits = 17;       // no problem, just a bit slower
    else if(rndbits > 25)
        rndbits = 25;       // fizzle fade will not fill whole screen

    rndmask = rndmasks[rndbits - 17];
}

boolean FizzleFade (SDL_Surface *source, int x1, int y1,
    unsigned width, unsigned height, unsigned frames, boolean abortable)
{
    unsigned x, y, p, frame, pixperframe;
    int32_t  rndval;

    rndval = 1;
    pixperframe = width * height / frames;

    IN_StartAck ();

    frame = GetTimeCount();
    byte *srcptr = VL_LockSurface(source);
    if(srcptr == NULL) return false;

    while (1)
    {
        IN_ProcessEvents();

        if(abortable && IN_CheckAck ())
        {
            VL_UnlockSurface(source);
            VH_UpdateScreen (source);
            return true;
        }

        byte *destptr = VL_LockSurface(screen);

        if (!destptr)
            Quit ("Unable to lock dest surface: %s\n",SDL_GetError());

        for (p = 0; p < pixperframe; p++)
        {
            //
            // seperate random value into x/y pair
            //
            x = rndval >> rndbits_y;
            y = rndval & ((1 << rndbits_y) - 1);

            //
            // advance to next random element
            //
            rndval = (rndval >> 1) ^ (rndval & 1 ? 0 : rndmask);

            if (x >= width || y >= height)
                p--;                         // not into the view area; get a new pair
            else
            {
                //
                // copy one pixel
                //
                if(screenBits == 8)
                {
                    *(destptr + (y1 + y) * screen->pitch + x1 + x)
                        = *(srcptr + (y1 + y) * source->pitch + x1 + x);
                }
                else
                {
                    byte col = *(srcptr + (y1 + y) * source->pitch + x1 + x);
                    uint32_t fullcol = SDL_MapRGBA(screen->format, curpal[col].r, curpal[col].g, curpal[col].b,SDL_ALPHA_OPAQUE);
                    memcpy(destptr + (y1 + y) * screen->pitch + (x1 + x) * screen->format->BytesPerPixel,
                        &fullcol, screen->format->BytesPerPixel);
                }
            }

            if (rndval == 1)
            {
                //
                // entire sequence has been completed
                //
                VL_UnlockSurface (screenBuffer);
                VL_UnlockSurface (screen);
                VH_UpdateScreen (screenBuffer);

                return false;
            }
        }

        VL_UnlockSurface(screen);

        SDL_UpdateTexture(texture, NULL, screen->pixels, screenPitch);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        frame++;
        Delay(frame - GetTimeCount());        // don't go too fast
    }

    return false;
}
