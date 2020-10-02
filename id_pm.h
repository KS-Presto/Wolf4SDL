#ifndef __ID_PM_H_
#define __ID_PM_H_

#ifdef USE_HIRES
#define PMPageSize 16384
#else
#define PMPageSize 4096
#endif

extern int ChunksInFile;
extern int PMSpriteStart;
extern int PMSoundStart;

extern bool PMSoundInfoPagePadded;

// ChunksInFile+1 pointers to page starts.
// The last pointer points one byte after the last page.
extern uint8_t **PMPages;

void PM_Startup();
void PM_Shutdown();

uint32_t PM_GetPageSize(int page);

uint8_t *PM_GetPage(int page);

uint8_t *PM_GetEnd();

byte *PM_GetTexture(int wallpic);

uint16_t *PM_GetSprite(int shapenum);

byte *PM_GetSound(int soundpagenum);

#endif
