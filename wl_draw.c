// WL_DRAW.C

#include "wl_def.h"
#include "wl_cloudsky.h"
#include "wl_atmos.h"
#include "wl_shade.h"

/*
=============================================================================

                               LOCAL CONSTANTS

=============================================================================
*/

#define ACTORSIZE       0x4000

/*
=============================================================================

                              GLOBAL VARIABLES

=============================================================================
*/

byte     *vbuf;

visobj_t vislist[MAXVISABLE];

int32_t    lasttimecount;
int32_t    frameon;
boolean fpscounter;

int fps_frames=0, fps_time=0, fps=0;

#if defined(USE_FLOORCEILINGTEX) || defined(USE_CLOUDSKY)
int16_t *spanstart;
#endif

int16_t *wallheight;

//
// math tables
//
short *pixelangle;
int32_t finetangent[FINEANGLES/4];
fixed sintable[ANGLES+ANGLES/4];
fixed *costable = sintable+(ANGLES/4);

//
// refresh variables
//
fixed   viewx,viewy;                    // the focal point
short   viewangle;
fixed   viewsin,viewcos;

void    TransformActor (objtype *ob);
void    BuildTables (void);
void    ClearScreen (void);
int     CalcRotate (objtype *ob);
void    DrawScaleds (void);

#ifdef USE_SKYWALLPARALLAX
void    ScaleSkyPost();
#endif

int     postx;
byte    *postsource;
#ifdef USE_SKYWALLPARALLAX
byte    *postsourcesky;
#endif

//
// ray tracing variables
//
short    focaltx,focalty;
longword xpartialup,xpartialdown,ypartialup,ypartialdown;

short   midangle;

word    tilehit;
int     pixx;

short   xtile,ytile;
short   xtilestep,ytilestep;
fixed   xintercept,yintercept;
fixed   xinttile,yinttile;
word    texdelta;

word    horizwall[MAXWALLTILES],vertwall[MAXWALLTILES];


/*
============================================================================

                           3 - D  DEFINITIONS

============================================================================
*/

/*
========================
=
= TransformActor
=
= Takes paramaters:
=   gx,gy               : globalx/globaly of point
=
= globals:
=   viewx,viewy         : point of view
=   viewcos,viewsin     : sin/cos of viewangle
=   scale               : conversion from global value to screen value
=
= sets:
=   screenx,transx,transy,screenheight: projected edge location and size
=
========================
*/


//
// transform actor
//
void TransformActor (objtype *ob)
{
    fixed gx,gy,gxt,gyt,nx,ny;

//
// translate point to view centered coordinates
//
    gx = ob->x-viewx;
    gy = ob->y-viewy;

//
// calculate newx
//
    gxt = FixedMul(gx,viewcos);
    gyt = FixedMul(gy,viewsin);
    nx = gxt-gyt-ACTORSIZE;         // fudge the shape forward a bit, because
                                    // the midpoint could put parts of the shape
                                    // into an adjacent wall

//
// calculate newy
//
    gxt = FixedMul(gx,viewsin);
    gyt = FixedMul(gy,viewcos);
    ny = gyt+gxt;

//
// calculate perspective ratio
//
    ob->transx = nx;

    if (nx<MINDIST)                 // too close, don't overflow the divide
    {
        ob->viewheight = 0;
        return;
    }

    ob->viewx = (word)(centerx + ny*scale/nx);

//
// calculate height (heightnumerator/(nx>>8))
//
    ob->viewheight = (word)(heightnumerator/(nx>>8));
}

//==========================================================================

/*
========================
=
= TransformTile
=
= Takes paramaters:
=   tx,ty               : tile the object is centered in
=
= globals:
=   viewx,viewy         : point of view
=   viewcos,viewsin     : sin/cos of viewangle
=   scale               : conversion from global value to screen value
=
= sets:
=   screenx,transx,transy,screenheight: projected edge location and size
=
= Returns true if the tile is withing getting distance
=
========================
*/

boolean TransformTile (int tx, int ty, short *dispx, short *dispheight)
{
    fixed gx,gy,gxt,gyt,nx,ny;

//
// translate point to view centered coordinates
//
    gx = ((int32_t)tx<<TILESHIFT)+0x8000-viewx;
    gy = ((int32_t)ty<<TILESHIFT)+0x8000-viewy;

//
// calculate newx
//
    gxt = FixedMul(gx,viewcos);
    gyt = FixedMul(gy,viewsin);
    nx = gxt-gyt-0x2000;            // 0x2000 is size of object

//
// calculate newy
//
    gxt = FixedMul(gx,viewsin);
    gyt = FixedMul(gy,viewcos);
    ny = gyt+gxt;


//
// calculate height / perspective ratio
//
    if (nx<MINDIST)                 // too close, don't overflow the divide
        *dispheight = 0;
    else
    {
        *dispx = (short)(centerx + ny*scale/nx);
        *dispheight = (short)(heightnumerator/(nx>>8));
    }

//
// see if it should be grabbed
//
    if (nx<TILEGLOBAL && ny>-TILEGLOBAL/2 && ny<TILEGLOBAL/2)
        return true;
    else
        return false;
}

//==========================================================================

/*
====================
=
= CalcHeight
=
= Calculates the height of xintercept,yintercept from viewx,viewy
=
====================
*/

int16_t CalcHeight (void)
{
    int16_t height;
    fixed   gx,gy,gxt,gyt,nx;

//
// translate point to view centered coordinates
//
    gx = xintercept - viewx;
    gy = yintercept - viewy;

//
// calculate nx
//
    gxt = FixedMul(gx,viewcos);
    gyt = FixedMul(gy,viewsin);
    nx = gxt - gyt;

//
// calculate perspective ratio
//
#ifdef BANDEDHOLOWALLS
    if (nx < MINDIST)
        nx = MINDIST - ((nx / 36) & 3);

    height = (int16_t)(heightnumerator / (nx >> 8));

    if (nx < MINDIST)
        height = 0;
#else
    if (nx < MINDIST)
        nx = MINDIST;             // don't let divide overflow

    height = (int16_t)(heightnumerator / (nx >> 8));
#endif

    return height;
}

//==========================================================================

/*
===================
=
= ScalePost
=
===================
*/

void ScalePost (void)
{
    int ywcount, yoffs, yw, yd, yendoffs;
    byte col;

#ifdef USE_SKYWALLPARALLAX
    if (tilehit == 16)
    {
        ScaleSkyPost();
        return;
    }
#endif

#ifdef USE_SHADING
    byte *curshades = shadetable[GetShade(wallheight[postx])];
#endif

    ywcount = yd = wallheight[postx] >> 3;
    if(yd <= 0) yd = 100;

    yoffs = (centery - ywcount) * bufferPitch;
    if(yoffs < 0) yoffs = 0;
    yoffs += postx;

    yendoffs = centery + ywcount - 1;
    yw=TEXTURESIZE-1;

    while(yendoffs >= viewheight)
    {
        ywcount -= TEXTURESIZE/2;
        while(ywcount <= 0)
        {
            ywcount += yd;
            yw--;
        }
        yendoffs--;
    }
    if(yw < 0) return;

#ifdef USE_SHADING
    col = curshades[postsource[yw]];
#else
    col = postsource[yw];
#endif
    yendoffs = yendoffs * bufferPitch + postx;
    while(yoffs <= yendoffs)
    {
        vbuf[yendoffs] = col;
        ywcount -= TEXTURESIZE/2;
        if(ywcount <= 0)
        {
            do
            {
                ywcount += yd;
                yw--;
            }
            while(ywcount <= 0);
            if(yw < 0) break;
#ifdef USE_SHADING
            col = curshades[postsource[yw]];
#else
            col = postsource[yw];
#endif
        }
        yendoffs -= bufferPitch;
    }
}

#ifdef USE_SKYWALLPARALLAX
void ScaleSkyPost (void)
{
    int ywcount, yoffs, yendoffs, texoffs;
    byte col;
    int midy, y, skyheight;

    skyheight = viewheight;
    ywcount = wallheight[postx] >> 3;

    midy = (viewheight / 2) - 1;

    yoffs = midy * bufferPitch;
    if(yoffs < 0) yoffs = 0;
    yoffs += postx;

    yendoffs = midy + (ywcount * 2) - 1;

    if (yendoffs >= viewheight)
        yendoffs = viewheight - 1;

    int curang = pixelangle[postx] + midangle;
    if(curang < 0)
        curang += FINEANGLES;
    else if(curang >= FINEANGLES)
        curang -= FINEANGLES;
    int xtex = curang * USE_SKYWALLPARALLAX * TEXTURESIZE / FINEANGLES;
    texoffs = TEXTUREMASK - ((xtex & (TEXTURESIZE - 1)) << TEXTURESHIFT);

    y = yendoffs;
    yendoffs = yendoffs * bufferPitch + postx;
    while(yoffs <= yendoffs)
    {
        vbuf[yendoffs] = postsourcesky[texoffs + (y * TEXTURESIZE) / skyheight];
        yendoffs -= bufferPitch;
        y--;
    }
}
#endif

/*
====================
=
= HitVertWall
=
= tilehit bit 7 is 0, because it's not a door tile
= if bit 6 is 1 and the adjacent tile is a door tile, use door side pic
=
====================
*/

void HitVertWall (void)
{
    int wallpic;
    int texture;

    texture = ((yintercept - texdelta) >> FIXED2TEXSHIFT) & TEXTUREMASK;

    if (xtilestep == -1)
    {
        texture = TEXTUREMASK - texture;
        xintercept += TILEGLOBAL;
    }

    wallheight[pixx] = CalcHeight();
    postx = pixx;

    if (tilehit & BIT_WALL)
    {
        //
        // check for adjacent doors
        //
        if (tilemap[xtile - xtilestep][yinttile] & BIT_DOOR)
            wallpic = DOORWALL+3;
        else
            wallpic = vertwall[tilehit & ~BIT_WALL];
    }
    else
        wallpic = vertwall[tilehit];

    postsource = PM_GetPage(wallpic) + texture;
#ifdef USE_SKYWALLPARALLAX
    postsourcesky = postsource - texture;
#endif
    ScalePost ();
}


/*
====================
=
= HitHorizWall
=
= tilehit bit 7 is 0, because it's not a door tile
= if bit 6 is 1 and the adjacent tile is a door tile, use door side pic
=
====================
*/

void HitHorizWall (void)
{
    int wallpic;
    int texture;

    texture = ((xintercept - texdelta) >> FIXED2TEXSHIFT) & TEXTUREMASK;

    if (ytilestep == -1)
        yintercept += TILEGLOBAL;
    else
        texture = TEXTUREMASK - texture;

    wallheight[pixx] = CalcHeight();
    postx = pixx;

    if (tilehit & BIT_WALL)
    {
        //
        // check for adjacent doors
        //
        if (tilemap[xinttile][ytile - ytilestep] & BIT_DOOR)
            wallpic = DOORWALL + 2;
        else
            wallpic = horizwall[tilehit & ~BIT_WALL];
    }
    else
        wallpic = horizwall[tilehit];

    postsource = PM_GetPage(wallpic) + texture;
#ifdef USE_SKYWALLPARALLAX
    postsourcesky = postsource - texture;
#endif
    ScalePost ();
}

//==========================================================================

/*
====================
=
= HitHorizDoor
=
====================
*/

void HitHorizDoor (void)
{
    int doorpage;
    int doornum;
    int texture;

    doornum = tilehit & ~BIT_DOOR;
    texture = ((xintercept - doorobjlist[doornum].position) >> FIXED2TEXSHIFT) & TEXTUREMASK;

    wallheight[pixx] = CalcHeight();
    postx = pixx;

    switch (doorobjlist[doornum].lock)
    {
        case dr_normal:
            doorpage = DOORWALL;
            break;

        case dr_lock1:
        case dr_lock2:
        case dr_lock3:
        case dr_lock4:
            doorpage = DOORWALL + 6;
            break;

        case dr_elevator:
            doorpage = DOORWALL + 4;
            break;
    }

    postsource = PM_GetPage(doorpage) + texture;

    ScalePost ();
}

//==========================================================================

/*
====================
=
= HitVertDoor
=
====================
*/

void HitVertDoor (void)
{
    int doorpage;
    int doornum;
    int texture;

    doornum = tilehit & ~BIT_DOOR;
    texture = ((yintercept - doorobjlist[doornum].position) >> FIXED2TEXSHIFT) & TEXTUREMASK;

    wallheight[pixx] = CalcHeight();
    postx = pixx;

    switch (doorobjlist[doornum].lock)
    {
        case dr_normal:
            doorpage = DOORWALL + 1;
            break;

        case dr_lock1:
        case dr_lock2:
        case dr_lock3:
        case dr_lock4:
            doorpage = DOORWALL + 7;
            break;

        case dr_elevator:
            doorpage = DOORWALL + 5;
            break;
    }

    postsource = PM_GetPage(doorpage) + texture;

    ScalePost ();
}

//==========================================================================

byte vgaCeiling[]=
{
#ifndef SPEAR
 0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0xbf,
 0x4e,0x4e,0x4e,0x1d,0x8d,0x4e,0x1d,0x2d,0x1d,0x8d,
 0x1d,0x1d,0x1d,0x1d,0x1d,0x2d,0xdd,0x1d,0x1d,0x98,

 0x1d,0x9d,0x2d,0xdd,0xdd,0x9d,0x2d,0x4d,0x1d,0xdd,
 0x7d,0x1d,0x2d,0x2d,0xdd,0xd7,0x1d,0x1d,0x1d,0x2d,
 0x1d,0x1d,0x1d,0x1d,0xdd,0xdd,0x7d,0xdd,0xdd,0xdd
#else
 0x6f,0x4f,0x1d,0xde,0xdf,0x2e,0x7f,0x9e,0xae,0x7f,
 0x1d,0xde,0xdf,0xde,0xdf,0xde,0xe1,0xdc,0x2e,0x1d,0xdc
#endif
};

/*
=====================
=
= VGAClearScreen
=
=====================
*/

void VGAClearScreen (void)
{
    byte ceiling=vgaCeiling[gamestate.episode*10+gamestate.mapon];

    int y;
    byte *dest = vbuf;
#ifdef USE_SHADING
    for(y = 0; y < viewheight / 2; y++, dest += bufferPitch)
        memset(dest, shadetable[GetShade((viewheight / 2 - y) << 3)][ceiling], viewwidth);
    for(; y < viewheight; y++, dest += bufferPitch)
        memset(dest, shadetable[GetShade((y - viewheight / 2) << 3)][0x19], viewwidth);
#else
    for(y = 0; y < viewheight / 2; y++, dest += bufferPitch)
        memset(dest, ceiling, viewwidth);
    for(; y < viewheight; y++, dest += bufferPitch)
        memset(dest, 0x19, viewwidth);
#endif
}

//==========================================================================

/*
=====================
=
= CalcRotate
=
=====================
*/

int CalcRotate (objtype *ob)
{
    int angle, viewangle;

    // this isn't exactly correct, as it should vary by a trig value,
    // but it is close enough with only eight rotations

#ifdef FIXCALCROTATE
    viewangle = (int)( player->angle + (centerx - ob->viewx) / (8 * viewwidth / 320.0) );
#else
    viewangle = player->angle + (centerx - ob->viewx)/8;
#endif


    if (ob->obclass == rocketobj || ob->obclass == hrocketobj)
        angle = (viewangle-180) - ob->angle;
    else
        angle = (viewangle-180) - dirangle[ob->dir];

    angle+=ANGLES/16;
    while (angle>=ANGLES)
        angle-=ANGLES;
    while (angle<0)
        angle+=ANGLES;

    if (ob->state->rotate == 2)             // 2 rotation pain frame
        return 0;               // pain with shooting frame bugfix

    return angle/(ANGLES/8);
}


/*
=====================
=
= DrawScaleds
=
= Draws all objects that are visable
=
=====================
*/

void DrawScaleds (void)
{
    int       i,least,numvisable,height;
    byte      *visspot;
    statobj_t *statptr;
    objtype   *obj;
    visobj_t  *visptr,*visstep,*farthest;

    visptr = &vislist[0];

//
// place static objects
//
    for (statptr = &statobjlist[0] ; statptr !=laststatobj ; statptr++)
    {
        if ((visptr->shapenum = statptr->shapenum) == -1)
            continue;                                               // object has been deleted

        if (!spotvis[statptr->tilex][statptr->tiley])
            continue;                                               // not visable

        if (TransformTile (statptr->tilex,statptr->tiley,
            &visptr->viewx,&visptr->viewheight) && statptr->flags & FL_BONUS)
        {
            GetBonus (statptr);
            if(statptr->shapenum == -1)
                continue;                                           // object has been taken
        }

        if (!visptr->viewheight)
            continue;                                               // to close to the object

        if (visptr < &vislist[MAXVISABLE-1])    // don't let it overflow
        {
            visptr->tilex = statptr->tilex;
            visptr->tiley = statptr->tiley;
            visptr->flags = statptr->flags;
            visptr++;
        }
    }

//
// place active objects
//
    for (obj = player->next;obj;obj=obj->next)
    {
        if ((visptr->shapenum = obj->state->shapenum)==0)
            continue;                                               // no shape

        visspot = (byte *)&spotvis[obj->tilex][obj->tiley];

        //
        // could be in any of the nine surrounding tiles
        //
        if (*visspot
            || ( *(visspot-1) )
            || ( *(visspot+1) )
            || ( *(visspot-(MAPSIZE+1)) )
            || ( *(visspot-(MAPSIZE)) )
            || ( *(visspot-(MAPSIZE-1)) )
            || ( *(visspot+(MAPSIZE+1)) )
            || ( *(visspot+(MAPSIZE)) )
            || ( *(visspot+(MAPSIZE-1)) ) )
        {
            obj->active = ac_yes;
            TransformActor (obj);
            if (!obj->viewheight)
                continue;                                               // too close or far away

            visptr->viewx = obj->viewx;
            visptr->viewheight = obj->viewheight;
            if (visptr->shapenum == -1)
                visptr->shapenum = obj->temp1;  // special shape

            if (obj->state->rotate)
                visptr->shapenum += CalcRotate (obj);

            if (visptr < &vislist[MAXVISABLE-1])    // don't let it overflow
            {
                visptr->tilex = obj->tilex;
                visptr->tiley = obj->tiley;
                visptr->flags = obj->flags;
                visptr++;
            }
            obj->flags |= FL_VISABLE;
        }
        else
            obj->flags &= ~FL_VISABLE;
    }

//
// draw from back to front
//
    numvisable = (int) (visptr-&vislist[0]);

    if (!numvisable)
        return;                                                                 // no visable objects

    for (i = 0; i<numvisable; i++)
    {
        least = 32000;
        for (visstep=&vislist[0] ; visstep<visptr ; visstep++)
        {
            height = visstep->viewheight;
            if (height < least)
            {
                least = height;
                farthest = visstep;
            }
        }
        //
        // draw farthest
        //
#ifdef USE_DIR3DSPR
        if (farthest->flags & FL_DIR_MASK)
            Transform3DShape (farthest);
        else
#endif
            ScaleShape (farthest->viewx,farthest->shapenum,farthest->viewheight,farthest->flags);

        farthest->viewheight = 32000;
    }
}

//==========================================================================

/*
==============
=
= DrawPlayerWeapon
=
= Draw the player's hands
=
==============
*/

int weaponscale[NUMWEAPONS] = {SPR_KNIFEREADY, SPR_PISTOLREADY,
    SPR_MACHINEGUNREADY, SPR_CHAINREADY};

void DrawPlayerWeapon (void)
{
    int shapenum;

#ifndef SPEAR
    if (gamestate.victoryflag)
    {
#ifndef APOGEE_1_0
        if (player->state == &s_deathcam && (GetTimeCount()&32) )
            SimpleScaleShape(viewwidth/2,SPR_DEATHCAM,viewheight+1);
#endif
        return;
    }
#endif

    if (gamestate.weapon != -1)
    {
        shapenum = weaponscale[gamestate.weapon]+gamestate.weaponframe;
        SimpleScaleShape(viewwidth/2,shapenum,viewheight+1);
    }

    if (demorecord || demoplayback)
        SimpleScaleShape(viewwidth/2,SPR_DEMO,viewheight+1);
}


//==========================================================================


/*
=====================
=
= CalcTics
=
=====================
*/

void CalcTics (void)
{
//
// calculate tics since last refresh for adaptive timing
//
    if (lasttimecount > (int32_t) GetTimeCount())
        lasttimecount = GetTimeCount();    // if the game was paused a LONG time

    uint32_t curtime = SDL_GetTicks();
    tics = (curtime * 7) / 100 - lasttimecount;
    if(!tics)
    {
        // wait until end of current tic
        SDL_Delay(((lasttimecount + 1) * 100) / 7 - curtime);
        tics = 1;
    }

    lasttimecount += tics;

    if (tics>MAXTICS)
        tics = MAXTICS;
}


//==========================================================================


/*
=====================
=
= WallRefresh
=
= For each column of pixels on screen, cast a ray from the player
= to the nearest wall
=
= There is no check to stop the ray from passing outside of the map
= boundaries, so it is the map designer's responsibility to make sure there
= are solid walls covering the entire outer edge of each map
=
=====================
*/

void WallRefresh (void)
{
    int16_t   angle;
    int32_t   xstep,ystep;
    fixed     xinttemp,yinttemp;                            // holds temporary intercept position
    longword  xpartial,ypartial;
    doorobj_t *door;
    int       pwallposnorm,pwallposinv,pwallposi;           // holds modified pwallpos

    for (pixx = 0; pixx < viewwidth; pixx++)
    {
        //
        // setup to trace a ray through pixx view pixel
        //
        angle = midangle + pixelangle[pixx];                // delta for this pixel

        if (angle < 0)                                      // -90 - -1 degree arc
            angle += ANG360;                                // -90 is the same as 270
        if (angle >= ANG360)                                // 360-449 degree arc
            angle -= ANG360;                                // -449 is the same as 89

        //
        // setup xstep/ystep based on angle
        //
        if (angle < ANG90)                                  // 0-89 degree arc
        {
            xtilestep = 1;
            ytilestep = -1;
            xstep = finetangent[ANG90 - 1 - angle];
            ystep = -finetangent[angle];
            xpartial = xpartialup;
            ypartial = ypartialdown;
        }
        else if (angle < ANG180)                            // 90-179 degree arc
        {
            xtilestep = -1;
            ytilestep = -1;
            xstep = -finetangent[angle - ANG90];
            ystep = -finetangent[ANG180 - 1 - angle];
            xpartial = xpartialdown;
            ypartial = ypartialdown;
        }
        else if (angle < ANG270)                            // 180-269 degree arc
        {
            xtilestep = -1;
            ytilestep = 1;
            xstep = -finetangent[ANG270 - 1 - angle];
            ystep = finetangent[angle - ANG180];
            xpartial = xpartialdown;
            ypartial = ypartialup;
        }
        else if (angle < ANG360)                            // 270-359 degree arc
        {
            xtilestep = 1;
            ytilestep = 1;
            xstep = finetangent[angle - ANG270];
            ystep = finetangent[ANG360 - 1 - angle];
            xpartial = xpartialup;
            ypartial = ypartialup;
        }

        //
        // initialise variables for intersection testing
        //
        yintercept = FixedMul(ystep,xpartial) + viewy;
        yinttile = yintercept >> TILESHIFT;
        xtile = focaltx + xtilestep;

        xintercept = FixedMul(xstep,ypartial) + viewx;
        xinttile = xintercept >> TILESHIFT;
        ytile = focalty + ytilestep;

        texdelta = 0;

        //
        // special treatment when player is in back tile of pushwall
        //
        if (tilemap[focaltx][focalty] == BIT_WALL)
        {
            if ((pwalldir == di_east && xtilestep == 1) || (pwalldir == di_west && xtilestep == -1))
            {
                yinttemp = yintercept - ((ystep * (64 - pwallpos)) >> 6);

                //
                //  trace hit vertical pushwall back?
                //
                if (yinttemp >> TILESHIFT == focalty)
                {
                    if (pwalldir == di_east)
                        xintercept = (focaltx << TILESHIFT) + (pwallpos << 10);
                    else
                        xintercept = ((focaltx << TILESHIFT) - TILEGLOBAL) + ((64 - pwallpos) << 10);

                    yintercept = yinttemp;
                    yinttile = yintercept >> TILESHIFT;
                    tilehit = pwalltile;
                    HitVertWall();
                    continue;
                }
            }
            else if ((pwalldir == di_south && ytilestep == 1) || (pwalldir == di_north && ytilestep == -1))
            {
                xinttemp = xintercept - ((xstep * (64 - pwallpos)) >> 6);

                //
                // trace hit horizontal pushwall back?
                //
                if (xinttemp >> TILESHIFT == focaltx)
                {
                    if (pwalldir == di_south)
                        yintercept = (focalty << TILESHIFT) + (pwallpos << 10);
                    else
                        yintercept = ((focalty << TILESHIFT) - TILEGLOBAL) + ((64 - pwallpos) << 10);

                    xintercept = xinttemp;
                    xinttile = xintercept >> TILESHIFT;
                    tilehit = pwalltile;
                    HitHorizWall();
                    continue;
                }
            }
        }

//
// trace along this angle until we hit a wall
//
// CORE LOOP!
//
        while (1)
        {
            //
            // check intersections with vertical walls
            //
            if ((xtile - xtilestep) == xinttile && (ytile - ytilestep) == ytile)
                yinttile = ytile;

            if ((ytilestep == -1 && yinttile <= ytile) || (ytilestep == 1 && yinttile >= ytile))
                goto horizentry;
vertentry:
#ifdef REVEALMAP
            mapseen[xtile][yinttile] = true;
#endif
            tilehit = tilemap[xtile][yinttile];

            if (tilehit)
            {
                if (tilehit & BIT_DOOR)
                {
                    //
                    // hit a vertical door, so find which coordinate the door would be
                    // intersected at, and check to see if the door is open past that point
                    //
                    door = &doorobjlist[tilehit & ~BIT_DOOR];

                    if (door->action == dr_open)
                        goto passvert;                       // door is open, continue tracing

                    yinttemp = yintercept + (ystep >> 1);    // add halfstep to current intercept position

                    //
                    // midpoint is outside tile, so it hit the side of the wall before a door
                    //
                    if (yinttemp >> TILESHIFT != yinttile)
                        goto passvert;

                    if (door->action != dr_closed)
                    {
                        //
                        // the trace hit the door plane at pixel position yintercept, see if the door is
                        // closed that much
                        //
                        if ((word)yinttemp < door->position)
                            goto passvert;
                    }

                    yintercept = yinttemp;
                    xintercept = ((fixed)xtile << TILESHIFT) + (TILEGLOBAL/2);

                    HitVertDoor();
                }
                else if (tilehit == BIT_WALL)
                {
                    //
                    // hit a sliding vertical wall
                    //
                    if (pwalldir == di_west || pwalldir == di_east)
                    {
                        if (pwalldir == di_west)
                        {
                            pwallposnorm = 64 - pwallpos;
                            pwallposinv = pwallpos;
                        }
                        else
                        {
                            pwallposnorm = pwallpos;
                            pwallposinv = 64 - pwallpos;
                        }

                        if ((pwalldir == di_east && xtile == pwallx && yinttile == pwally)
                         || (pwalldir == di_west && !(xtile == pwallx && yinttile == pwally)))
                        {
                            yinttemp = yintercept + ((ystep * pwallposnorm) >> 6);

                            if (yinttemp >> TILESHIFT != yinttile)
                                goto passvert;

                            yintercept = yinttemp;
                            xintercept = (((fixed)xtile << TILESHIFT) + TILEGLOBAL) - (pwallposinv << 10);
                            yinttile = yintercept >> TILESHIFT;
                            tilehit = pwalltile;

                            HitVertWall();
                        }
                        else
                        {
                            yinttemp = yintercept + ((ystep * pwallposinv) >> 6);

                            if (yinttemp >> TILESHIFT != yinttile)
                                goto passvert;

                            yintercept = yinttemp;
                            xintercept = ((fixed)xtile << TILESHIFT) - (pwallposinv << 10);
                            yinttile = yintercept >> TILESHIFT;
                            tilehit = pwalltile;

                            HitVertWall();
                        }
                    }
                    else
                    {
                        if (pwalldir == di_north)
                            pwallposi = 64 - pwallpos;
                        else
                            pwallposi = pwallpos;

                        if ((pwalldir == di_south && (word)yintercept < (pwallposi << 10))
                         || (pwalldir == di_north && (word)yintercept > (pwallposi << 10)))
                        {
                            if (xtile == pwallx && yinttile == pwally)
                            {
                                if ((pwalldir == di_south && (int32_t)((word)yintercept) + ystep < (pwallposi << 10))
                                 || (pwalldir == di_north && (int32_t)((word)yintercept) + ystep > (pwallposi << 10)))
                                    goto passvert;

                                //
                                // set up a horizontal intercept position
                                //
                                if (pwalldir == di_south)
                                    yintercept = (yinttile << TILESHIFT) + (pwallposi << 10);
                                else
                                    yintercept = ((yinttile << TILESHIFT) - TILEGLOBAL) + (pwallposi << 10);

                                xintercept -= (xstep * (64 - pwallpos)) >> 6;
                                xinttile = xintercept >> TILESHIFT;
                                tilehit = pwalltile;

                                HitHorizWall();
                            }
                            else
                            {
                                texdelta = pwallposi << 10;
                                xintercept = (fixed)xtile << TILESHIFT;
                                tilehit = pwalltile;

                                HitVertWall();
                            }
                        }
                        else
                        {
                            if (xtile == pwallx && yinttile == pwally)
                            {
                                texdelta = pwallposi << 10;
                                xintercept = (fixed)xtile << TILESHIFT;
                                tilehit = pwalltile;

                                HitVertWall();
                            }
                            else
                            {
                                if ((pwalldir == di_south && (int32_t)((word)yintercept) + ystep > (pwallposi << 10))
                                 || (pwalldir == di_north && (int32_t)((word)yintercept) + ystep < (pwallposi << 10)))
                                    goto passvert;

                                //
                                // set up a horizontal intercept position
                                //
                                if (pwalldir == di_south)
                                    yintercept = (yinttile << TILESHIFT) - ((64 - pwallpos) << 10);
                                else
                                    yintercept = (yinttile << TILESHIFT) + ((64 - pwallpos) << 10);

                                xintercept -= (xstep * pwallpos) >> 6;
                                xinttile = xintercept >> TILESHIFT;
                                tilehit = pwalltile;

                                HitHorizWall();
                            }
                        }
                    }
                }
                else
                {
                    xintercept = (fixed)xtile << TILESHIFT;

                    HitVertWall();
                }

                break;
            }

passvert:
            //
            // mark the tile as visible and setup for next step
            //
            spotvis[xtile][yinttile] = true;
            xtile += xtilestep;
            yintercept += ystep;
            yinttile = yintercept >> TILESHIFT;
        }

        continue;

        while (1)
        {
            //
            // check intersections with horizontal walls
            //
            if ((xtile - xtilestep) == xinttile && (ytile - ytilestep) == yinttile)
                xinttile = xtile;

            if ((xtilestep == -1 && xinttile <= xtile) || (xtilestep == 1 && xinttile >= xtile))
                goto vertentry;

horizentry:
#ifdef REVEALMAP
            mapseen[xinttile][ytile] = true;
#endif
            tilehit = tilemap[xinttile][ytile];

            if (tilehit)
            {
                if (tilehit & BIT_DOOR)
                {
                    //
                    // hit a horizontal door, so find which coordinate the door would be
                    // intersected at, and check to see if the door is open past that point
                    //
                    door = &doorobjlist[tilehit & ~BIT_DOOR];

                    if (door->action == dr_open)
                        goto passhoriz;                      // door is open, continue tracing

                    xinttemp = xintercept + (xstep >> 1);    // add half step to current intercept position

                    //
                    // midpoint is outside tile, so it hit the side of the wall before a door
                    //
                    if (xinttemp >> TILESHIFT != xinttile)
                        goto passhoriz;

                    if (door->action != dr_closed)
                    {
                        //
                        // the trace hit the door plane at pixel position xintercept, see if the door is
                        // closed that much
                        //
                        if ((word)xinttemp < door->position)
                            goto passhoriz;
                    }

                    xintercept = xinttemp;
                    yintercept = ((fixed)ytile << TILESHIFT) + (TILEGLOBAL/2);

                    HitHorizDoor();
                }
                else if (tilehit == BIT_WALL)
                {
                    //
                    // hit a sliding horizontal wall
                    //
                    if (pwalldir == di_north || pwalldir == di_south)
                    {
                        if (pwalldir == di_north)
                        {
                            pwallposnorm = 64 - pwallpos;
                            pwallposinv = pwallpos;
                        }
                        else
                        {
                            pwallposnorm = pwallpos;
                            pwallposinv = 64 - pwallpos;
                        }

                        if ((pwalldir == di_south && xinttile == pwallx && ytile == pwally)
                         || (pwalldir == di_north && !(xinttile == pwallx && ytile == pwally)))
                        {
                            xinttemp = xintercept + ((xstep * pwallposnorm) >> 6);

                            if (xinttemp >> TILESHIFT != xinttile)
                                goto passhoriz;

                            xintercept = xinttemp;
                            yintercept = (((fixed)ytile << TILESHIFT) + TILEGLOBAL) - (pwallposinv << 10);
                            xinttile = xintercept >> TILESHIFT;
                            tilehit = pwalltile;

                            HitHorizWall();
                        }
                        else
                        {
                            xinttemp = xintercept + ((xstep * pwallposinv) >> 6);

                            if (xinttemp >> TILESHIFT != xinttile)
                                goto passhoriz;

                            xintercept = xinttemp;
                            yintercept = ((fixed)ytile << TILESHIFT) - (pwallposinv << 10);
                            xinttile = xintercept >> TILESHIFT;
                            tilehit = pwalltile;

                            HitHorizWall();
                        }
                    }
                    else
                    {
                        if (pwalldir == di_west)
                            pwallposi = 64 - pwallpos;
                        else
                            pwallposi = pwallpos;

                        if ((pwalldir == di_east && (word)xintercept < (pwallposi << 10))
                         || (pwalldir == di_west && (word)xintercept > (pwallposi << 10)))
                        {
                            if (xinttile == pwallx && ytile == pwally)
                            {
                                if ((pwalldir == di_east && (int32_t)((word)xintercept) + xstep < (pwallposi << 10))
                                 || (pwalldir == di_west && (int32_t)((word)xintercept) + xstep > (pwallposi << 10)))
                                    goto passhoriz;

                                //
                                // set up a vertical intercept position
                                //
                                yintercept -= (ystep * (64 - pwallpos)) >> 6;
                                yinttile = yintercept >> TILESHIFT;

                                if (pwalldir == di_east)
                                    xintercept = (xinttile << TILESHIFT) + (pwallposi << 10);
                                else
                                    xintercept = ((xinttile << TILESHIFT) - TILEGLOBAL) + (pwallposi << 10);

                                tilehit = pwalltile;

                                HitVertWall();
                            }
                            else
                            {
                                texdelta = pwallposi << 10;
                                yintercept = ytile << TILESHIFT;
                                tilehit = pwalltile;

                                HitHorizWall();
                            }
                        }
                        else
                        {
                            if (xinttile == pwallx && ytile == pwally)
                            {
                                texdelta = pwallposi << 10;
                                yintercept = ytile << TILESHIFT;
                                tilehit = pwalltile;

                                HitHorizWall();
                            }
                            else
                            {
                                if ((pwalldir == di_east && (int32_t)((word)xintercept) + xstep > (pwallposi << 10))
                                 || (pwalldir == di_west && (int32_t)((word)xintercept) + xstep < (pwallposi << 10)))
                                    goto passhoriz;

                                //
                                // set up a vertical intercept position
                                //
                                yintercept -= (ystep * pwallpos) >> 6;
                                yinttile = yintercept >> TILESHIFT;

                                if (pwalldir == di_east)
                                    xintercept = (xinttile << TILESHIFT) - ((64 - pwallpos) << 10);
                                else
                                    xintercept = (xinttile << TILESHIFT) + ((64 - pwallpos) << 10);

                                tilehit = pwalltile;

                                HitVertWall();
                            }
                        }
                    }
                }
                else
                {
                    yintercept = (fixed)ytile << TILESHIFT;

                    HitHorizWall();
                }

                break;
            }

passhoriz:
            //
            // mark the tile as visible and setup for next step
            //
            spotvis[xinttile][ytile] = true;
            ytile += ytilestep;
            xintercept += xstep;
            xinttile = xintercept >> TILESHIFT;
        }
    }
}


/*
====================
=
= Setup3DView
=
====================
*/

void Setup3DView (void)
{
    viewangle = player->angle;
    midangle = viewangle * (FINEANGLES / ANGLES);

    viewsin = sintable[viewangle];
    viewcos = costable[viewangle];

    viewx = player->x - FixedMul(focallength,viewcos);
    viewy = player->y + FixedMul(focallength,viewsin);

    focaltx = (short)(viewx>>TILESHIFT);
    focalty = (short)(viewy>>TILESHIFT);

    xpartialdown = viewx&(TILEGLOBAL-1);
    xpartialup = xpartialdown ^ (TILEGLOBAL - 1);
    ypartialdown = viewy&(TILEGLOBAL-1);
    ypartialup = ypartialdown ^ (TILEGLOBAL - 1);
}

//==========================================================================

/*
========================
=
= ThreeDRefresh
=
========================
*/

void ThreeDRefresh (void)
{
//
// clear out the traced array
//
    memset(spotvis,0,MAPAREA);
#ifdef PLAYDEMOLIKEORIGINAL      // ADDEDFIX 30 - Chris
    if (DEMOCOND_SDL)
#endif
    if (!tilemap[player->tilex][player->tiley] ||
         tilemap[player->tilex][player->tiley] & BIT_DOOR)
    spotvis[player->tilex][player->tiley] = true;       // Detect all sprites over player fix
#ifdef REVEALMAP
    mapseen[player->tilex][player->tiley] = true;
#endif

    vbuf = VL_LockSurface(screenBuffer);
    if(vbuf == NULL) return;

    vbuf += screenofs;

    Setup3DView ();

//
// follow the walls from there to the right, drawing as we go
//
    VGAClearScreen ();
#if defined(USE_FEATUREFLAGS) && defined(USE_STARSKY)
    if(GetFeatureFlags() & FF_STARSKY)
        DrawStarSky();
#endif

    WallRefresh ();

#if defined(USE_FEATUREFLAGS) && defined(USE_PARALLAX)
    if(GetFeatureFlags() & FF_PARALLAXSKY)
        DrawParallax();
#endif

#if defined(USE_FEATUREFLAGS) && defined(USE_CLOUDSKY)
    if(GetFeatureFlags() & FF_CLOUDSKY)
        DrawCloudPlanes ();
#endif

#ifdef USE_FLOORCEILINGTEX
    DrawPlanes ();
#endif

//
// draw all the scaled images
//
    DrawScaleds();                  // draw scaled stuff

#if defined(USE_FEATUREFLAGS) && defined(USE_RAIN)
    if(GetFeatureFlags() & FF_RAIN)
        DrawRain();
#endif
#if defined(USE_FEATUREFLAGS) && defined(USE_SNOW)
    if(GetFeatureFlags() & FF_SNOW)
        DrawSnow();
#endif

    DrawPlayerWeapon ();    // draw player's hands

    if(Keyboard[sc_Tab] && viewsize == 21 && gamestate.weapon != -1)
        ShowActStatus();

    VL_UnlockSurface(screenBuffer);
    vbuf = NULL;

//
// show screen and time last cycle
//

    if (fizzlein)
    {
        FizzleFade(screenBuffer, 0, 0, screenWidth, screenHeight, 20, false);
        fizzlein = false;

        lasttimecount = GetTimeCount();          // don't make a big tic count
    }
    else
    {
#ifndef REMDEBUG
        if (fpscounter)
        {
            fontnumber = 0;
            SETFONTCOLOR(7,127);
            PrintX=4; PrintY=1;
            VWB_Bar(0,0,50,10,bordercol);
            US_PrintSigned(fps);
            US_Print(" fps");
        }
#endif
        VW_UpdateScreen ();
    }

#ifndef REMDEBUG
    if (fpscounter)
    {
        fps_frames++;
        fps_time+=tics;

        if(fps_time>35)
        {
            fps_time-=35;
            fps=fps_frames<<1;
            fps_frames=0;
        }
    }
#endif
}
