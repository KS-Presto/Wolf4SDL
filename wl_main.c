// WL_MAIN.C

#ifdef _WIN32
    #include <io.h>
#else
    #include <unistd.h>
#endif

#include "wl_def.h"
#include "wl_atmos.h"
#include <SDL_syswm.h>


/*
=============================================================================

                             WOLFENSTEIN 3-D

                        An Id Software production

                             by John Carmack

=============================================================================
*/

extern byte signon[];

/*
=============================================================================

                             LOCAL CONSTANTS

=============================================================================
*/


#define FOCALLENGTH     (0x5700l)               // in global coordinates
#define VIEWGLOBAL      0x10000                 // globals visable flush to wall

#define VIEWWIDTH       256                     // size of view window
#define VIEWHEIGHT      144

/*
=============================================================================

                            GLOBAL VARIABLES

=============================================================================
*/

char    str[80];
int     dirangle[9] = {0,ANGLES/8,2*ANGLES/8,3*ANGLES/8,4*ANGLES/8,
                       5*ANGLES/8,6*ANGLES/8,7*ANGLES/8,ANGLES};

//
// proejection variables
//
fixed    focallength;
unsigned screenofs;
int      viewscreenx, viewscreeny;
int      viewwidth;
int      viewheight;
short    centerx,centery;
int      shootdelta;           // pixels away from centerx a target can be
fixed    scale;
int32_t  heightnumerator;


boolean startgame;
boolean loadedgame;
int     mouseadjustment;

char    configdir[256] = "";
char    configname[13] = "config.";

//
// Command line parameter variables
//
boolean param_debugmode = false;
boolean param_nowait = false;
int     param_difficulty = 1;           // default is "normal"
int     param_tedlevel = -1;            // default is not to start a level
int     param_joystickindex = 0;
int     param_audiobuffer = DEFAULT_AUDIO_BUFFER_SIZE;

#if defined(_arch_dreamcast)
int     param_joystickhat = 0;
int     param_samplerate = 11025;       // higher samplerates result in "out of memory"
#elif defined(GP2X_940)
int     param_joystickhat = -1;
int     param_samplerate = 11025;       // higher samplerates result in "out of memory"
#else
int     param_joystickhat = -1;
int     param_samplerate = 44100;
#endif

int     param_mission = 0;
boolean param_goodtimes = false;
boolean param_ignorenumchunks = false;

/*
=============================================================================

                            LOCAL VARIABLES

=============================================================================
*/


/*
====================
=
= ReadConfig
=
====================
*/

void ReadConfig(void)
{
    byte  sd;
    byte  sm;
    byte sds;
    FILE *file;

    char configpath[300];

#ifdef _arch_dreamcast
    DC_LoadFromVMU(configname);
#endif

    if (configdir[0])
        snprintf (configpath, sizeof(configpath), "%s/%s", configdir, configname);
    else
        snprintf (configpath,sizeof(configpath),"%s",configname);

    file = fopen(configpath,"rb");

    if (file)
    {
        //
        // valid config file
        //
        word tmp;
        fread (&tmp,sizeof(tmp),1,file);

        if (tmp!=0xfefa)
        {
            fclose (file);
            goto noconfig;
        }

        fread (Scores,sizeof(Scores),1,file);

        fread (&sd,sizeof(sd),1,file);
        fread (&sm,sizeof(sm),1,file);
        fread (&sds,sizeof(sds),1,file);

        fread (&mouseenabled,sizeof(mouseenabled),1,file);
        fread (&joystickenabled,sizeof(joystickenabled),1,file);
        boolean dummyJoypadEnabled;
        fread (&dummyJoypadEnabled,sizeof(dummyJoypadEnabled),1,file);
        boolean dummyJoystickProgressive;
        fread (&dummyJoystickProgressive,sizeof(dummyJoystickProgressive),1,file);
        int dummyJoystickPort = 0;
        fread (&dummyJoystickPort,sizeof(dummyJoystickPort),1,file);

        fread (dirscan,sizeof(dirscan),1,file);
        fread (buttonscan,sizeof(buttonscan),1,file);
        fread (buttonmouse,sizeof(buttonmouse),1,file);
        fread (buttonjoy,sizeof(buttonjoy),1,file);

        fread (&viewsize,sizeof(viewsize),1,file);
        fread (&mouseadjustment,sizeof(mouseadjustment),1,file);

        fclose (file);

        if ((sd == sdm_AdLib || sm == smm_AdLib) && !AdLibPresent
                && !SoundBlasterPresent)
        {
            sd = sdm_PC;
            sm = smm_Off;
        }

        if ((sds == sds_SoundBlaster && !SoundBlasterPresent))
            sds = sds_Off;

        // make sure values are correct

        if(mouseenabled) mouseenabled=true;
        if(joystickenabled) joystickenabled=true;

        if (!MousePresent)
            mouseenabled = false;
        if (!IN_JoyPresent())
            joystickenabled = false;

        if(mouseadjustment<0) mouseadjustment=0;
        else if(mouseadjustment>9) mouseadjustment=9;

        if(viewsize<4) viewsize=4;
        else if(viewsize>21) viewsize=21;

        MainMenu[6].active=1;
        MainItems.curpos=0;
    }
    else
    {
        //
        // no config file, so select by hardware
        //
noconfig:
        if (SoundBlasterPresent || AdLibPresent)
        {
            sd = sdm_AdLib;
            sm = smm_AdLib;
        }
        else
        {
            sd = sdm_PC;
            sm = smm_Off;
        }

        if (SoundBlasterPresent)
            sds = sds_SoundBlaster;
        else
            sds = sds_Off;

        if (MousePresent)
            mouseenabled = true;

        if (IN_JoyPresent())
            joystickenabled = true;

        viewsize = 19;                          // start with a good size
        mouseadjustment=5;
    }

    SD_SetMusicMode (sm);
    SD_SetSoundMode (sd);
    SD_SetDigiDevice (sds);
}

/*
====================
=
= WriteConfig
=
====================
*/

void WriteConfig(void)
{
    char configpath[300];
    FILE *file;

#ifdef _arch_dreamcast
    fs_unlink(configname);
#endif

    if (configdir[0])
        snprintf (configpath, sizeof(configpath), "%s/%s", configdir, configname);
    else
        snprintf (configpath,sizeof(configpath),"%s",configname);

    file = fopen(configpath,"wb");

    if (file)
    {
        word tmp=0xfefa;
        fwrite (&tmp,sizeof(tmp),1,file);
        fwrite (Scores,sizeof(Scores),1,file);

        fwrite (&SoundMode,sizeof(SoundMode),1,file);
        fwrite (&MusicMode,sizeof(MusicMode),1,file);
        fwrite (&DigiMode,sizeof(DigiMode),1,file);

        fwrite (&mouseenabled,sizeof(mouseenabled),1,file);
        fwrite (&joystickenabled,sizeof(joystickenabled),1,file);
        boolean dummyJoypadEnabled = false;
        fwrite (&dummyJoypadEnabled,sizeof(dummyJoypadEnabled),1,file);
        boolean dummyJoystickProgressive = false;
        fwrite (&dummyJoystickProgressive,sizeof(dummyJoystickProgressive),1,file);
        int dummyJoystickPort = 0;
        fwrite (&dummyJoystickPort,sizeof(dummyJoystickPort),1,file);

        fwrite (dirscan,sizeof(dirscan),1,file);
        fwrite (buttonscan,sizeof(buttonscan),1,file);
        fwrite (buttonmouse,sizeof(buttonmouse),1,file);
        fwrite (buttonjoy,sizeof(buttonjoy),1,file);

        fwrite (&viewsize,sizeof(viewsize),1,file);
        fwrite (&mouseadjustment,sizeof(mouseadjustment),1,file);

        fclose (file);
    }
#ifdef _arch_dreamcast
    DC_SaveToVMU(configname, NULL);
#endif
}


//===========================================================================

/*
=====================
=
= NewGame
=
= Set up new game to start from the beginning
=
=====================
*/

void NewGame (int difficulty,int episode)
{
    memset (&gamestate,0,sizeof(gamestate));
    gamestate.difficulty = difficulty;
    gamestate.weapon = gamestate.bestweapon
            = gamestate.chosenweapon = wp_pistol;
    gamestate.health = 100;
    gamestate.ammo = STARTAMMO;
    gamestate.lives = 3;
    gamestate.nextextra = EXTRAPOINTS;
    gamestate.episode=episode;

    startgame = true;
}

//===========================================================================

void DiskFlopAnim(int x,int y)
{
    static int8_t which=0;
    if (!x && !y)
        return;
    VWB_DrawPic(x,y,C_DISKLOADING1PIC+which);
    VW_UpdateScreen();
    which^=1;
}


int32_t DoChecksum (void *source, unsigned size, int32_t checksum)
{
    unsigned i;
    byte     *src;

    src = source;

    for (i = 0; i < size - 1; i++)
        checksum += src[i] ^ src[i + 1];

    return checksum;
}


/*
==================
=
= SaveTheGame
=
==================
*/

extern statetype s_grdstand;
extern statetype s_player;

boolean SaveTheGame (FILE *file, int x, int y)
{
    int       i,j;
    int       checksum;
    word      actnum,laststatobjnum;
    objtype   *ob;
    objtype   nullobj;
    statobj_t *statobj;
    doorobj_t *door;

    checksum = 0;

    DiskFlopAnim (x,y);
    fwrite (&gamestate,sizeof(gamestate),1,file);
    checksum = DoChecksum(&gamestate,sizeof(gamestate),checksum);

    DiskFlopAnim (x,y);
    fwrite (LevelRatios,sizeof(LevelRatios),1,file);
    checksum = DoChecksum(LevelRatios,sizeof(LevelRatios),checksum);

    DiskFlopAnim (x,y);
    fwrite (tilemap,sizeof(tilemap),1,file);
    checksum = DoChecksum(tilemap,sizeof(tilemap),checksum);
#ifdef REVEALMAP
    DiskFlopAnim (x,y);
    fwrite (mapseen,sizeof(mapseen),1,file);
    checksum = DoChecksum(mapseen,sizeof(mapseen),checksum);
#endif
    DiskFlopAnim (x,y);

    for (i = 0; i < mapwidth; i++)
    {
        for (j = 0; j < mapheight; j++)
        {
            ob = actorat[i][j];

            if (ISPOINTER(ob))
                actnum = 0x8000 | (word)(ob - objlist);
            else
                actnum = (word)(uintptr_t)ob;

            fwrite (&actnum,sizeof(actnum),1,file);
            checksum = DoChecksum(&actnum,sizeof(actnum),checksum);
        }
    }

    fwrite (areaconnect,sizeof(areaconnect),1,file);
    fwrite (areabyplayer,sizeof(areabyplayer),1,file);

    DiskFlopAnim (x,y);
    for (ob = player; ob; ob = ob->next)
    {
        memcpy (&nullobj,ob,sizeof(nullobj));

        //
        // player object needs special treatment as it's in WL_AGENT.C and not in
        // WL_ACT2.C which could cause problems for the relative addressing
        //
        if (ob == player)
            nullobj.state = (statetype *)((uintptr_t)nullobj.state - (uintptr_t)&s_player);
        else
            nullobj.state = (statetype *)((uintptr_t)nullobj.state - (uintptr_t)&s_grdstand);

        fwrite (&nullobj,sizeof(nullobj),1,file);
    }

    nullobj.active = ac_badobject;          // end of file marker
    DiskFlopAnim (x,y);
    fwrite (&nullobj,sizeof(nullobj),1,file);

    DiskFlopAnim (x,y);
    laststatobjnum = (word)(laststatobj - statobjlist);
    fwrite (&laststatobjnum,sizeof(laststatobjnum),1,file);
    checksum = DoChecksum(&laststatobjnum,sizeof(laststatobjnum),checksum);

    DiskFlopAnim (x,y);
    for (statobj = &statobjlist[0]; statobj < laststatobj; statobj++)
    {
        fwrite (statobj,sizeof(*statobj),1,file);
        checksum = DoChecksum(statobj,sizeof(*statobj),checksum);
    }

    DiskFlopAnim (x,y);
    for (door = &doorobjlist[0]; door < lastdoorobj; door++)
    {
        fwrite (door,sizeof(*door),1,file);
        checksum = DoChecksum(door,sizeof(*door),checksum);
    }

    DiskFlopAnim (x,y);
    fwrite (&pwallstate,sizeof(pwallstate),1,file);
    checksum = DoChecksum(&pwallstate,sizeof(pwallstate),checksum);
    fwrite (&pwalltile,sizeof(pwalltile),1,file);
    checksum = DoChecksum(&pwalltile,sizeof(pwalltile),checksum);
    fwrite (&pwallx,sizeof(pwallx),1,file);
    checksum = DoChecksum(&pwallx,sizeof(pwallx),checksum);
    fwrite (&pwally,sizeof(pwally),1,file);
    checksum = DoChecksum(&pwally,sizeof(pwally),checksum);
    fwrite (&pwalldir,sizeof(pwalldir),1,file);
    checksum = DoChecksum(&pwalldir,sizeof(pwalldir),checksum);
    fwrite (&pwallpos,sizeof(pwallpos),1,file);
    checksum = DoChecksum(&pwallpos,sizeof(pwallpos),checksum);

    //
    // write out checksum
    //
    fwrite (&checksum,sizeof(checksum),1,file);

    fwrite (&lastgamemusicoffset,sizeof(lastgamemusicoffset),1,file);

    return true;
}

//===========================================================================

/*
==================
=
= LoadTheGame
=
==================
*/

boolean LoadTheGame (FILE *file, int x, int y)
{
    int       i,j;
    int       actnum = 0;
    word      laststatobjnum;
    word      *map,tile;
    int32_t   checksum,oldchecksum;
    objtype   *newobj = NULL;
    objtype   nullobj;
    statobj_t *statobj;
    doorobj_t *door;

    checksum = 0;

    DiskFlopAnim (x,y);
    fread (&gamestate,sizeof(gamestate),1,file);
    checksum = DoChecksum(&gamestate,sizeof(gamestate),checksum);

    DiskFlopAnim (x,y);
    fread (LevelRatios,sizeof(LevelRatios),1,file);
    checksum = DoChecksum(LevelRatios,sizeof(LevelRatios),checksum);

    DiskFlopAnim (x,y);
    SetupGameLevel ();

    DiskFlopAnim (x,y);
    fread (tilemap,sizeof(tilemap),1,file);
    checksum = DoChecksum(tilemap,sizeof(tilemap),checksum);
#ifdef REVEALMAP
    DiskFlopAnim (x,y);
    fread (mapseen,sizeof(mapseen),1,file);
    checksum = DoChecksum(mapseen,sizeof(mapseen),checksum);
#endif
    DiskFlopAnim (x,y);

    for (i = 0; i < mapwidth; i++)
    {
        for (j = 0; j < mapheight; j++)
        {
            fread (&actnum,sizeof(word),1,file);
            checksum = DoChecksum(&actnum,sizeof(word),checksum);

            if (actnum & 0x8000)
                actorat[i][j] = &objlist[actnum & 0x7fff];
            else
                actorat[i][j] = (objtype *)(uintptr_t)actnum;
        }
    }

    fread (areaconnect,sizeof(areaconnect),1,file);
    fread (areabyplayer,sizeof(areabyplayer),1,file);

    DiskFlopAnim (x,y);
    InitActorList ();

    while (1)
    {
        fread (&nullobj,sizeof(nullobj),1,file);

        if (nullobj.active == ac_badobject)
            break;

        if (nullobj.obclass == playerobj)
        {
            newobj = player;
            nullobj.state = (statetype *)((uintptr_t)nullobj.state + (uintptr_t)&s_player);
        }
        else
        {
            newobj = GetNewActor();
            nullobj.state = (statetype *)((uintptr_t)nullobj.state + (uintptr_t)&s_grdstand);
        }

        //
        // skip the last 2 members so we don't copy over the links
        //
        memcpy (newobj,&nullobj,sizeof(nullobj) - (sizeof(nullobj.next) + sizeof(nullobj.prev)));
    }

    DiskFlopAnim (x,y);
    fread (&laststatobjnum,sizeof(laststatobjnum),1,file);
    laststatobj = &statobjlist[laststatobjnum];
    checksum = DoChecksum(&laststatobjnum,sizeof(laststatobjnum),checksum);

    DiskFlopAnim (x,y);
    for (statobj = &statobjlist[0]; statobj < laststatobj; statobj++)
    {
        fread (statobj,sizeof(*statobj),1,file);
        checksum = DoChecksum(statobj,sizeof(*statobj),checksum);
    }

    DiskFlopAnim (x,y);
    for (door = &doorobjlist[0]; door < lastdoorobj; door++)
    {
        fread (door,sizeof(*door),1,file);
        checksum = DoChecksum(door,sizeof(*door),checksum);
    }

    DiskFlopAnim(x,y);
    fread (&pwallstate,sizeof(pwallstate),1,file);
    checksum = DoChecksum(&pwallstate,sizeof(pwallstate),checksum);
    fread (&pwalltile,sizeof(pwalltile),1,file);
    checksum = DoChecksum(&pwalltile,sizeof(pwalltile),checksum);
    fread (&pwallx,sizeof(pwallx),1,file);
    checksum = DoChecksum(&pwallx,sizeof(pwallx),checksum);
    fread (&pwally,sizeof(pwally),1,file);
    checksum = DoChecksum(&pwally,sizeof(pwally),checksum);
    fread (&pwalldir,sizeof(pwalldir),1,file);
    checksum = DoChecksum(&pwalldir,sizeof(pwalldir),checksum);
    fread (&pwallpos,sizeof(pwallpos),1,file);
    checksum = DoChecksum(&pwallpos,sizeof(pwallpos),checksum);

    if (gamestate.secretcount)
    {
        //
        // assign valid floorcodes under moved pushwalls
        //
        map = mapsegs[0];

        for (y = 0; y < mapheight; y++)
        {
            for (x = 0; x < mapwidth; x++)
            {
                tile = *map;

                if (MAPSPOT(x,y,1) == PUSHABLETILE && !tilemap[x][y] && !VALIDAREA(tile))
                {
                    if (VALIDAREA(*(map + 1)))
                        tile = *(map + 1);
                    if (VALIDAREA(*(map - mapwidth)))
                        tile = *(map - mapwidth);
                    if (VALIDAREA(*(map + mapwidth)))
                        tile = *(map + mapwidth);
                    if (VALIDAREA(*(map - 1)))
                        tile = *(map - 1);

                    *map = tile;
                    MAPSPOT(x,y,1) = 0;
                }

                map++;
            }
        }
    }

    Thrust (0,0);    // set player->areanumber to the floortile you're standing on

    fread (&oldchecksum,sizeof(oldchecksum),1,file);

    fread (&lastgamemusicoffset,sizeof(lastgamemusicoffset),1,file);

    if (lastgamemusicoffset < 0)
        lastgamemusicoffset = 0;

    if (oldchecksum != checksum)
    {
        Message(STR_SAVECHT1"\n"
                STR_SAVECHT2"\n"
                STR_SAVECHT3"\n"
                STR_SAVECHT4);

        IN_ClearKeysDown ();
        IN_Ack ();

        gamestate.oldscore = gamestate.score = 0;
        gamestate.lives = 1;
        gamestate.weapon =
        gamestate.chosenweapon =
        gamestate.bestweapon = wp_pistol;
        gamestate.ammo = 8;
    }

    return true;
}

//===========================================================================

/*
==========================
=
= ShutdownId
=
= Shuts down all ID_?? managers
=
==========================
*/

void ShutdownId (void)
{
    US_Shutdown ();         // This line is completely useless...
    SD_Shutdown ();
    PM_Shutdown ();
    IN_Shutdown ();
    VW_Shutdown ();
    CA_Shutdown ();
#if defined(GP2X_940)
    GP2X_Shutdown();
#endif
}


//===========================================================================

/*
==================
=
= BuildTables
=
= Calculates:
=
= scale                 projection constant
= sintable/costable     overlapping fractional tables
=
==================
*/

const float radtoint = (float)(FINEANGLES/2/PI);

void BuildTables (void)
{
    //
    // calculate fine tangents
    //

    int i;
    for(i=0;i<FINEANGLES/8;i++)
    {
        double tang=tan((i+0.5)/radtoint);
        finetangent[i]=(int32_t)(tang*GLOBAL1);
        finetangent[FINEANGLES/4-1-i]=(int32_t)((1/tang)*GLOBAL1);
    }

    //
    // costable overlays sintable with a quarter phase shift
    // ANGLES is assumed to be divisable by four
    //

    float angle=0;
    float anglestep=(float)(PI/2/ANGLEQUAD);
    for(i=0; i<ANGLEQUAD; i++)
    {
        fixed value=(int32_t)(GLOBAL1*sin(angle));
        sintable[i]=sintable[i+ANGLES]=sintable[ANGLES/2-i]=value;
        sintable[ANGLES-i]=sintable[ANGLES/2+i]=-value;
        angle+=anglestep;
    }
    sintable[ANGLEQUAD] = 65536;
    sintable[3*ANGLEQUAD] = -65536;

#if defined(USE_STARSKY) || defined(USE_RAIN) || defined(USE_SNOW)
    Init3DPoints();
#endif
}

//===========================================================================


/*
====================
=
= CalcProjection
=
= Uses focallength
=
====================
*/

void CalcProjection (int32_t focal)
{
    int     i;
    int    intang;
    float   angle;
    double  tang;
    int     halfview;
    double  facedist;

    focallength = focal;
    facedist = focal+MINDIST;
    halfview = viewwidth/2;                                 // half view in pixels

    //
    // calculate scale value for vertical height calculations
    // and sprite x calculations
    //
    scale = (fixed) (halfview*facedist/(VIEWGLOBAL/2));

    //
    // divide heightnumerator by a posts distance to get the posts height for
    // the heightbuffer.  The pixel height is height>>2
    //
    heightnumerator = (TILEGLOBAL*scale)>>6;

    //
    // calculate the angle offset from view angle of each pixel's ray
    //

    for (i=0;i<halfview;i++)
    {
        // start 1/2 pixel over, so viewangle bisects two middle pixels
        tang = (int32_t)i*VIEWGLOBAL/viewwidth/facedist;
        angle = (float) atan(tang);
        intang = (int) (angle*radtoint);
        pixelangle[halfview-1-i] = intang;
        pixelangle[halfview+i] = -intang;
    }
}



//===========================================================================

/*
===================
=
= SetupWalls
=
= Map tile values to scaled pics
=
===================
*/

void SetupWalls (void)
{
    int     i;

    horizwall[0]=0;
    vertwall[0]=0;

    for (i=1;i<MAXWALLTILES;i++)
    {
        horizwall[i]=(i-1)*2;
        vertwall[i]=(i-1)*2+1;
    }
}

//===========================================================================

/*
==========================
=
= SignonScreen
=
==========================
*/

void SignonScreen (void)                        // VGA version
{
    VL_SetVGAPlaneMode ();

    VL_MemToScreen (signon,320,200,0,0);
}


/*
==========================
=
= FinishSignon
=
==========================
*/

void FinishSignon (void)
{
#ifndef SPEAR
    VW_Bar (0,189,300,11,VL_GetPixel(0,0));
    WindowX = 0;
    WindowW = 320;
    PrintY = 190;

    #ifndef JAPAN
    SETFONTCOLOR(14,4);

    #ifdef SPANISH
    US_CPrint ("Oprima una tecla");
    #else
    US_CPrint ("Press a key");
    #endif

    #endif

    VW_UpdateScreen();

    if (!param_nowait)
        IN_Ack ();

    #ifndef JAPAN
    VW_Bar (0,189,300,11,VL_GetPixel(0,0));

    PrintY = 190;
    SETFONTCOLOR(10,4);

    #ifdef SPANISH
    US_CPrint ("pensando...");
    #else
    US_CPrint ("Working...");
    #endif

    VW_UpdateScreen();
    #endif

    SETFONTCOLOR(0,15);
#else
    VW_UpdateScreen();

    if (!param_nowait)
        VW_WaitVBL(3*70);
#endif
}

//===========================================================================

/*
=====================
=
= InitDigiMap
=
=====================
*/

// channel mapping:
//  -1: any non reserved channel
//   0: player weapons
//   1: boss weapons

static int wolfdigimap[] =
    {
        // These first sounds are in the upload version
#ifndef SPEAR
        HALTSND,                0,  -1,
        DOGBARKSND,             1,  -1,
        CLOSEDOORSND,           2,  -1,
        OPENDOORSND,            3,  -1,
        ATKMACHINEGUNSND,       4,   0,
        ATKPISTOLSND,           5,   0,
        ATKGATLINGSND,          6,   0,
        SCHUTZADSND,            7,  -1,
        GUTENTAGSND,            8,  -1,
        MUTTISND,               9,  -1,
        BOSSFIRESND,            10,  1,
        SSFIRESND,              11, -1,
        DEATHSCREAM1SND,        12, -1,
        DEATHSCREAM2SND,        13, -1,
        DEATHSCREAM3SND,        13, -1,
        TAKEDAMAGESND,          14, -1,
        PUSHWALLSND,            15, -1,

        LEBENSND,               20, -1,
        NAZIFIRESND,            21, -1,
        SLURPIESND,             22, -1,

        YEAHSND,                32, -1,

#ifndef UPLOAD
        // These are in all other episodes
        DOGDEATHSND,            16, -1,
        AHHHGSND,               17, -1,
        DIESND,                 18, -1,
        EVASND,                 19, -1,

        TOT_HUNDSND,            23, -1,
        MEINGOTTSND,            24, -1,
        SCHABBSHASND,           25, -1,
        HITLERHASND,            26, -1,
        SPIONSND,               27, -1,
        NEINSOVASSND,           28, -1,
        DOGATTACKSND,           29, -1,
        LEVELDONESND,           30, -1,
        MECHSTEPSND,            31, -1,

        SCHEISTSND,             33, -1,
        DEATHSCREAM4SND,        34, -1,         // AIIEEE
        DEATHSCREAM5SND,        35, -1,         // DEE-DEE
        DONNERSND,              36, -1,         // EPISODE 4 BOSS DIE
        EINESND,                37, -1,         // EPISODE 4 BOSS SIGHTING
        ERLAUBENSND,            38, -1,         // EPISODE 6 BOSS SIGHTING
        DEATHSCREAM6SND,        39, -1,         // FART
        DEATHSCREAM7SND,        40, -1,         // GASP
        DEATHSCREAM8SND,        41, -1,         // GUH-BOY!
        DEATHSCREAM9SND,        42, -1,         // AH GEEZ!
        KEINSND,                43, -1,         // EPISODE 5 BOSS SIGHTING
        MEINSND,                44, -1,         // EPISODE 6 BOSS DIE
        ROSESND,                45, -1,         // EPISODE 5 BOSS DIE

#endif
#else
//
// SPEAR OF DESTINY DIGISOUNDS
//
        HALTSND,                0,  -1,
        CLOSEDOORSND,           2,  -1,
        OPENDOORSND,            3,  -1,
        ATKMACHINEGUNSND,       4,   0,
        ATKPISTOLSND,           5,   0,
        ATKGATLINGSND,          6,   0,
        SCHUTZADSND,            7,  -1,
        BOSSFIRESND,            8,   1,
        SSFIRESND,              9,  -1,
        DEATHSCREAM1SND,        10, -1,
        DEATHSCREAM2SND,        11, -1,
        TAKEDAMAGESND,          12, -1,
        PUSHWALLSND,            13, -1,
        AHHHGSND,               15, -1,
        LEBENSND,               16, -1,
        NAZIFIRESND,            17, -1,
        SLURPIESND,             18, -1,
        LEVELDONESND,           22, -1,
        DEATHSCREAM4SND,        23, -1,         // AIIEEE
        DEATHSCREAM3SND,        23, -1,         // DOUBLY-MAPPED!!!
        DEATHSCREAM5SND,        24, -1,         // DEE-DEE
        DEATHSCREAM6SND,        25, -1,         // FART
        DEATHSCREAM7SND,        26, -1,         // GASP
        DEATHSCREAM8SND,        27, -1,         // GUH-BOY!
        DEATHSCREAM9SND,        28, -1,         // AH GEEZ!
        GETGATLINGSND,          38, -1,         // Got Gat replacement

#ifndef SPEARDEMO
        DOGBARKSND,             1,  -1,
        DOGDEATHSND,            14, -1,
        SPIONSND,               19, -1,
        NEINSOVASSND,           20, -1,
        DOGATTACKSND,           21, -1,
        TRANSSIGHTSND,          29, -1,         // Trans Sight
        TRANSDEATHSND,          30, -1,         // Trans Death
        WILHELMSIGHTSND,        31, -1,         // Wilhelm Sight
        WILHELMDEATHSND,        32, -1,         // Wilhelm Death
        UBERDEATHSND,           33, -1,         // Uber Death
        KNIGHTSIGHTSND,         34, -1,         // Death Knight Sight
        KNIGHTDEATHSND,         35, -1,         // Death Knight Death
        ANGELSIGHTSND,          36, -1,         // Angel Sight
        ANGELDEATHSND,          37, -1,         // Angel Death
        GETSPEARSND,            39, -1,         // Got Spear replacement
#endif
#endif
        LASTSOUND
    };


void InitDigiMap (void)
{
    int *map;

    for (map = wolfdigimap; *map != LASTSOUND; map += 3)
    {
        DigiMap[map[0]] = map[1];
        DigiChannel[map[1]] = map[2];
        SD_PrepareSound(map[1]);
    }
}

#ifndef SPEAR
CP_iteminfo MusicItems={CTL_X,CTL_Y,6,0,32};
CP_itemtype MusicMenu[]=
    {
        {1,"Get Them!",0},
        {1,"Searching",0},
        {1,"P.O.W.",0},
        {1,"Suspense",0},
        {1,"War March",0},
        {1,"Around The Corner!",0},

        {1,"Nazi Anthem",0},
        {1,"Lurking...",0},
        {1,"Going After Hitler",0},
        {1,"Pounding Headache",0},
        {1,"Into the Dungeons",0},
        {1,"Ultimate Conquest",0},

        {1,"Kill the S.O.B.",0},
        {1,"The Nazi Rap",0},
        {1,"Twelfth Hour",0},
        {1,"Zero Hour",0},
        {1,"Ultimate Conquest",0},
        {1,"Wolfpack",0}
    };
#else
CP_iteminfo MusicItems={CTL_X,CTL_Y-20,9,0,32};
CP_itemtype MusicMenu[]=
    {
        {1,"Funky Colonel Bill",0},
        {1,"Death To The Nazis",0},
        {1,"Tiptoeing Around",0},
        {1,"Is This THE END?",0},
        {1,"Evil Incarnate",0},
        {1,"Jazzin' Them Nazis",0},
        {1,"Puttin' It To The Enemy",0},
        {1,"The SS Gonna Get You",0},
        {1,"Towering Above",0}
    };
#endif

#ifndef SPEARDEMO
void DoJukebox(void)
{
    int which,lastsong=-1;
    unsigned start;
    unsigned songs[]=
        {
#ifndef SPEAR
            GETTHEM_MUS,
            SEARCHN_MUS,
            POW_MUS,
            SUSPENSE_MUS,
            WARMARCH_MUS,
            CORNER_MUS,

            NAZI_OMI_MUS,
            PREGNANT_MUS,
            GOINGAFT_MUS,
            HEADACHE_MUS,
            DUNGEON_MUS,
            ULTIMATE_MUS,

            INTROCW3_MUS,
            NAZI_RAP_MUS,
            TWELFTH_MUS,
            ZEROHOUR_MUS,
            ULTIMATE_MUS,
            PACMAN_MUS
#else
            XFUNKIE_MUS,             // 0
            XDEATH_MUS,              // 2
            XTIPTOE_MUS,             // 4
            XTHEEND_MUS,             // 7
            XEVIL_MUS,               // 17
            XJAZNAZI_MUS,            // 18
            XPUTIT_MUS,              // 21
            XGETYOU_MUS,             // 22
            XTOWER2_MUS              // 23
#endif
        };

    IN_ClearKeysDown();
    if (!AdLibPresent && !SoundBlasterPresent)
        return;

    MenuFadeOut();

#ifndef SPEAR
#ifndef UPLOAD
    start = ((SDL_GetTicks()/10)%3)*6;
#else
    start = 0;
#endif
#else
    start = 0;
#endif

    CA_LoadAllSounds ();

    fontnumber=1;
    ClearMScreen ();
    VWB_DrawPic(112,184,C_MOUSELBACKPIC);
    DrawStripes (10);
    SETFONTCOLOR (TEXTCOLOR,BKGDCOLOR);

#ifndef SPEAR
    DrawWindow (CTL_X-2,CTL_Y-6,280,13*7,BKGDCOLOR);
#else
    DrawWindow (CTL_X-2,CTL_Y-26,280,13*10,BKGDCOLOR);
#endif

    DrawMenu (&MusicItems,&MusicMenu[start]);

    SETFONTCOLOR (READHCOLOR,BKGDCOLOR);
    PrintY=15;
    WindowX = 0;
    WindowY = 320;
    US_CPrint ("Robert's Jukebox");

    SETFONTCOLOR (TEXTCOLOR,BKGDCOLOR);
    VW_UpdateScreen();
    MenuFadeIn();

    do
    {
        which = HandleMenu(&MusicItems,&MusicMenu[start],NULL);
        if (which>=0)
        {
            if (lastsong >= 0)
                MusicMenu[start+lastsong].active = 1;

            StartCPMusic(songs[start + which]);
            MusicMenu[start+which].active = 2;
            DrawMenu (&MusicItems,&MusicMenu[start]);
            VW_UpdateScreen();
            lastsong = which;
        }
    } while(which>=0);

    MenuFadeOut();
    IN_ClearKeysDown();
}
#endif

/*
==========================
=
= InitGame
=
= Load a few things right away
=
==========================
*/

static void InitGame()
{
#ifndef SPEARDEMO
    boolean didjukebox=false;
#endif

    // initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0)
    {
        printf("Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);

    int numJoysticks = SDL_NumJoysticks();
    if(param_joystickindex && (param_joystickindex < -1 || param_joystickindex >= numJoysticks))
    {
        if(!numJoysticks)
            printf("No joysticks are available to SDL!\n");
        else
            printf("The joystick index must be between -1 and %i!\n", numJoysticks - 1);
        exit(1);
    }

#if defined(GP2X_940)
    GP2X_MemoryInit();
#endif

    SignonScreen ();

	VW_UpdateScreen();

    VH_Startup ();
    IN_Startup ();
    PM_Startup ();
    SD_Startup ();
    CA_Startup ();
    US_Startup ();

    // TODO: Will any memory checking be needed someday??
#ifdef NOTYET
#ifndef SPEAR
    if (mminfo.mainmem < 235000L)
#else
    if (mminfo.mainmem < 257000L && !MS_CheckParm("debugmode"))
#endif
    {
        byte *screen;

        screen = grsegs[ERRORSCREEN];
        ShutdownId();
/*        memcpy((byte *)0xb8000,screen+7+7*160,17*160);
        gotoxy (1,23);*/
        exit(1);
    }
#endif


//
// build some tables
//
    InitDigiMap ();

    ReadConfig ();

    SetupSaveGames();

//
// HOLDING DOWN 'M' KEY?
//
	IN_ProcessEvents();

#ifndef SPEARDEMO
    if (Keyboard[sc_M])
    {
        DoJukebox();
        didjukebox=true;
    }
    else
#endif

//
// draw intro screen stuff
//
    IntroScreen ();

#ifdef _arch_dreamcast
    //TODO: VMU Selection Screen
#endif

//
// load in and lock down some basic chunks
//
    BuildTables ();          // trig tables
    SetupWalls ();

    NewViewSize (viewsize);

//
// initialize variables
//
    InitRedShifts ();
#ifndef SPEARDEMO
    if(!didjukebox)
#endif
        FinishSignon();

#ifdef NOTYET
    vdisp = (byte *) (0xa0000+PAGE1START);
    vbuf = (byte *) (0xa0000+PAGE2START);
#endif
}

//===========================================================================

/*
==========================
=
= SetViewSize
=
==========================
*/

boolean SetViewSize (unsigned width, unsigned height)
{
    viewwidth = width&~15;                  // must be divisable by 16
    viewheight = height&~1;                 // must be even
    centerx = viewwidth/2-1;
    centery = viewheight / 2;
    shootdelta = viewwidth/10;
    if (viewheight == screenHeight)
        viewscreenx = viewscreeny = screenofs = 0;
    else
    {
        viewscreenx = (screenWidth-viewwidth) / 2;
        viewscreeny = (screenHeight-scaleFactor*STATUSLINES-viewheight)/2;
        screenofs = viewscreeny*screenWidth+viewscreenx;
    }

//
// calculate trace angles and projection constants
//
    CalcProjection (FOCALLENGTH);

    return true;
}


void ShowViewSize (int width)
{
    int oldwidth,oldheight;

    oldwidth = viewwidth;
    oldheight = viewheight;

    if(width == 21)
    {
        viewwidth = screenWidth;
        viewheight = screenHeight;
        VWB_BarScaledCoord (0, 0, screenWidth, screenHeight, 0);
    }
    else if(width == 20)
    {
        viewwidth = screenWidth;
        viewheight = screenHeight - scaleFactor*STATUSLINES;
        DrawPlayBorder ();
    }
    else
    {
        viewwidth = width*16*screenWidth/320;
        viewheight = (int) (width*16*HEIGHTRATIO*screenHeight/200);
        DrawPlayBorder ();
    }

    viewwidth = oldwidth;
    viewheight = oldheight;
}


void NewViewSize (int width)
{
    viewsize = width;
    if(viewsize == 21)
        SetViewSize(screenWidth, screenHeight);
    else if(viewsize == 20)
        SetViewSize(screenWidth, screenHeight - scaleFactor * STATUSLINES);
    else
        SetViewSize(width*16*screenWidth/320, (unsigned) (width*16*HEIGHTRATIO*screenHeight/200));
}



//===========================================================================

/*
==========================
=
= Quit
=
==========================
*/

void Quit (const char *errorStr, ...)
{
    int  ret;
    char error[256];

    if (errorStr != NULL)
    {
        va_list vlist;
        va_start(vlist, errorStr);
        vsnprintf(error,sizeof(error), errorStr, vlist);
        va_end(vlist);
    }
    else
        error[0] = '\0';

    ret = *error != '\0';

    if (!ret)
        WriteConfig ();

    ShutdownId ();

    if (ret)
        Error (error);

    exit(ret);
}

//===========================================================================



/*
=====================
=
= DemoLoop
=
=====================
*/


static void DemoLoop()
{
    int LastDemo = 0;

//
// check for launch from ted
//
    if (param_tedlevel != -1)
    {
        param_nowait = true;
        EnableEndGameMenuItem();
        NewGame(param_difficulty,0);

#ifndef SPEAR
        gamestate.episode = param_tedlevel/10;
        gamestate.mapon = param_tedlevel%10;
#else
        gamestate.episode = 0;
        gamestate.mapon = param_tedlevel;
#endif
        GameLoop();
        Quit (NULL);
    }


//
// main game cycle
//

#ifndef DEMOTEST

    #ifndef UPLOAD

        #ifndef GOODTIMES
        #ifndef SPEAR
        #ifndef JAPAN
        if (!param_nowait)
            NonShareware();
        #endif
        #else
            #ifndef GOODTIMES
            #ifndef SPEARDEMO
            extern void CopyProtection(void);
            if(!param_goodtimes)
                CopyProtection();
            #endif
            #endif
        #endif
        #endif
    #endif

    StartCPMusic(INTROSONG);

#ifndef JAPAN
    if (!param_nowait)
        PG13 ();
#endif

#endif

    while (1)
    {
        while (!param_nowait)
        {
//
// title page
//
#ifndef DEMOTEST

#ifdef SPEAR
            SDL_Color pal[256];
            VL_ConvertPalette(grsegs[TITLEPALETTE], pal, 256);

            VWB_DrawPic (0,0,TITLE1PIC);
            VWB_DrawPic (0,80,TITLE2PIC);

            VW_UpdateScreen ();
            VL_FadeIn(0,255,pal,30);
#else
            VWB_DrawPic (0,0,TITLEPIC);
            VW_UpdateScreen ();
            VW_FadeIn();
#endif
            if (IN_UserInput(TickBase*15))
                break;
            VW_FadeOut();
//
// credits page
//
            VWB_DrawPic (0,0,CREDITSPIC);
            VW_UpdateScreen();
            VW_FadeIn ();
            if (IN_UserInput(TickBase*10))
                break;
            VW_FadeOut ();
//
// high scores
//
            DrawHighScores ();
            VW_UpdateScreen ();
            VW_FadeIn ();

            if (IN_UserInput(TickBase*10))
                break;
#endif
//
// demo
//

            #ifndef SPEARDEMO
            PlayDemo (LastDemo++%4);
            #else
            PlayDemo (0);
            #endif

            if (playstate == ex_abort)
                break;
            VW_FadeOut();
            if(screenHeight % 200 != 0)
                VL_ClearScreen(0);
            StartCPMusic(INTROSONG);
        }

        VW_FadeOut ();

#ifdef DEBUGKEYS
        if (Keyboard[sc_Tab] && param_debugmode)
            RecordDemo ();
        else
            US_ControlPanel (0);
#else
        US_ControlPanel (0);
#endif

        if (startgame || loadedgame)
        {
            GameLoop ();
            if(!param_nowait)
            {
                VW_FadeOut();
                StartCPMusic(INTROSONG);
            }
        }
    }
}


//===========================================================================

#define IFARG(str) if(!strcmp(arg, (str)))

void CheckParameters(int argc, char *argv[])
{
    const char *header =
    {
        "Wolf4SDL v2.0\n"
        "Ported by Chaos-Software, additions by the community\n"
        "Original Wolfenstein 3D by id Software\n"
        "Usage: Wolf4SDL [options]\n"
        "See Options.txt for help\n\n"
    };

    int    i;
    size_t len;
    char   error[256],*helpstr;

    error[0] = '\0';

    for(i = 1; i < argc; i++)
    {
        char *arg = argv[i];
#ifndef SPEAR
        IFARG("--goobers")
#else
        IFARG("--debugmode")
#endif
            param_debugmode = true;
        else IFARG("--baby")
            param_difficulty = 0;
        else IFARG("--easy")
            param_difficulty = 1;
        else IFARG("--normal")
            param_difficulty = 2;
        else IFARG("--hard")
            param_difficulty = 3;
        else IFARG("--nowait")
            param_nowait = true;
        else IFARG("--tedlevel")
        {
            if (++i >= argc)
                snprintf (error,sizeof(error),"The tedlevel option is missing the level argument!");
            else
                param_tedlevel = atoi(argv[i]);
        }
        else IFARG("--windowed")
            fullscreen = false;
        else IFARG("--windowed-mouse")
        {
            fullscreen = false;
            forcegrabmouse = true;
        }
        else IFARG("--res")
        {
            if (i + 2 >= argc)
                snprintf (error,sizeof(error),"The res option needs the width and/or the height argument!");
            else
            {
                screenWidth = atoi(argv[++i]);
                screenHeight = atoi(argv[++i]);
                int factor = screenWidth / 320;
                if ((screenWidth % 320) || (screenHeight != 200 * factor && screenHeight != 240 * factor))
                    snprintf (error,sizeof(error),"Screen size must be a multiple of 320x200 or 320x240!");
            }
        }
        else IFARG("--resf")
        {
            if (i + 2 >= argc)
                snprintf (error,sizeof(error),"The resf option needs the width and/or the height argument!");
            else
            {
                screenWidth = atoi(argv[++i]);
                screenHeight = atoi(argv[++i]);
                if (screenWidth < 320)
                    snprintf (error,sizeof(error),"Screen width must be at least 320!");
                if (screenHeight < 200)
                    snprintf (error,sizeof(error),"Screen height must be at least 200!");
            }
        }
        else IFARG("--bits")
        {
            if (++i >= argc)
                snprintf (error,sizeof(error),"The bits option is missing the color depth argument!");
            else
            {
                screenBits = atoi(argv[i]);

                if (screenBits > 32 || (screenBits & 7))
                    snprintf (error,sizeof(error),"Screen color depth must be 8, 16, 24, or 32!");
            }
        }
        else IFARG("--extravbls")
        {
            if (++i >= argc)
                snprintf (error,sizeof(error),"The extravbls option is missing the vbls argument!");
            else
            {
                extravbls = atoi(argv[i]);
                if(extravbls < 0)
                    snprintf (error,sizeof(error),"Extravbls must be positive!");
            }
        }
        else IFARG("--joystick")
        {
            if (++i >= argc)
                snprintf (error,sizeof(error),"The joystick option is missing the index argument!");
            else
                param_joystickindex = atoi(argv[i]);   // index is checked in InitGame
        }
        else IFARG("--joystickhat")
        {
            if (++i >= argc)
                snprintf (error,sizeof(error),"The joystickhat option is missing the index argument!");
            else
                param_joystickhat = atoi(argv[i]);
        }
        else IFARG("--samplerate")
        {
            if (++i >= argc)
                snprintf (error,sizeof(error),"The samplerate option is missing the rate argument!");
            else
            {
                param_samplerate = atoi(argv[i]);

                if (param_samplerate < 7042 || param_samplerate > 44100)
                    snprintf (error,sizeof(error),"The samplerate must be between 7042 and 44100!");
            }
        }
        else IFARG("--audiobuffer")
        {
            if (++i >= argc)
                snprintf (error,sizeof(error),"The audiobuffer option is missing the size argument!");
            else
                param_audiobuffer = atoi(argv[i]);
        }
        else IFARG("--mission")
        {
            if (++i >= argc)
                snprintf (error,sizeof(error),"The mission option is missing the mission argument!");
            else
            {
                param_mission = atoi(argv[i]);

                if (param_mission < 0 || param_mission > 3)
                    snprintf (error,sizeof(error),"The mission option must be between 0 and 3!");
            }
        }
        else IFARG("--configdir")
        {
            if (++i >= argc)
                snprintf (error,sizeof(error),"The configdir option is missing the dir argument!");
            else
            {
                len = strlen(argv[i]);

                if (len + 2 > sizeof(configdir))
                    snprintf (error,sizeof(error),"The config directory is too long!");
                else
                {
                    if (argv[i][len] != '/' && argv[i][len] != '\\')
                        snprintf (configdir,sizeof(configdir),"%s/",argv[i]);
                    else
                        snprintf (configdir,sizeof(configdir),"%s",argv[i]);
                }
            }
        }
        else IFARG("--goodtimes")
            param_goodtimes = true;
        else IFARG("--ignorenumchunks")
            param_ignorenumchunks = true;
        else
            snprintf (error,sizeof(error),"Invalid parameter(s)!");
    }

    if (*error)
    {
        len = strlen(header) + strlen(error) + 1;
        helpstr = SafeMalloc(len);

        snprintf (helpstr,len,"%s%s",header,error);

        Error (helpstr);

        free (helpstr);

        exit(1);
    }
}

/*
==========================
=
= main
=
==========================
*/

int main (int argc, char *argv[])
{
#if defined(_arch_dreamcast)
    DC_Init();
#else
    CheckParameters(argc, argv);
#endif

    CheckForEpisodes();

    InitGame();

    DemoLoop();

    Quit("Demo loop exited???");
    return 1;
}
