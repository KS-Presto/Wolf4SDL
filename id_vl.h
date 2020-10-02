// ID_VL.H

#ifndef __ID_VL_H_
#define __ID_VL_H_

// wolf compatability

void Quit (const char *error,...);

//===========================================================================

#define CHARWIDTH		2
#define TILEWIDTH		4

//===========================================================================

extern SDL_Surface *screen, *screenBuffer, *curSurface;

extern  boolean  fullscreen, usedoublebuffering;
extern  unsigned screenWidth, screenHeight, screenBits, screenPitch, bufferPitch, curPitch;
extern  int      scaleFactor;

extern	boolean  screenfaded;
extern	unsigned bordercolor;

extern SDL_Color gamepal[256];

//===========================================================================

//
// VGA hardware routines
//

#define VL_WaitVBL(a)        SDL_Delay((a)*8)
#define VL_ClearScreen(c)    SDL_FillRect(curSurface,NULL,(c))

void VL_SetVGAPlaneMode (void);
void VL_SetTextMode (void);
void VL_Shutdown (void);

void VL_ConvertPalette(byte *srcpal, SDL_Color *destpal, int numColors);
void VL_FillPalette (int red, int green, int blue);
void VL_SetColor    (int color, int red, int green, int blue);
void VL_GetColor    (int color, int *red, int *green, int *blue);
void VL_SetPalette  (SDL_Color *palette, bool forceupdate);
void VL_GetPalette  (SDL_Color *palette);
void VL_FadeOut     (int start, int end, int red, int green, int blue, int steps);
void VL_FadeIn      (int start, int end, SDL_Color *palette, int steps);

byte *VL_LockSurface(SDL_Surface *surface);
void VL_UnlockSurface(SDL_Surface *surface);

byte VL_GetPixel        (int x, int y);
void VL_Plot            (int x, int y, int color);
void VL_Hlin            (unsigned x, unsigned y, unsigned width, int color);
void VL_Vlin            (int x, int y, int height, int color);
void VL_BarScaledCoord  (int scx, int scy, int scwidth, int scheight, int color);
void VL_Bar             (int x, int y, int width, int height, int color);

void VL_MungePic                (byte *source, unsigned width, unsigned height);
void VL_DrawPicBare             (int x, int y, byte *pic, int width, int height);
void VL_MemToLatch              (byte *source, int width, int height,
                                    SDL_Surface *destSurface, int x, int y);
void VL_ScreenToScreen          (SDL_Surface *source, SDL_Surface *dest);
void VL_MemToScreenScaledCoord  (byte *source, int width, int height, int scx, int scy);
void VL_MemToScreenScaledCoord2  (byte *source, int origwidth, int origheight, int srcx, int srcy,
                                    int destx, int desty, int width, int height);

void VL_MemToScreen (byte *source, int width, int height, int x, int y);

void VL_MaskedToScreen (byte *source, int width, int height, int x, int y);

void VL_LatchToScreenScaledCoord (SDL_Surface *source, int xsrc, int ysrc, int width, int height, int scxdest, int scydest);

void VL_LatchToScreen (SDL_Surface *source, int xsrc, int ysrc, int width, int height, int xdest, int ydest);

#endif
