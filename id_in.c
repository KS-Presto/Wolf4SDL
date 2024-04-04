//
//	ID Engine
//	ID_IN.c - Input Manager
//	v1.0d1
//	By Jason Blochowiak
//

#include "wl_def.h"

/*
=============================================================================

					GLOBAL VARIABLES

=============================================================================
*/


//
// configuration variables
//
boolean MousePresent;
boolean forcegrabmouse;


// 	Global variables
bool        Keyboard[sc_Last];
char        textinput[TEXTINPUTSIZE];
boolean	    Paused;
ScanCode	LastScan;

static SDL_Joystick *Joystick;
int JoyNumButtons;
static int JoyNumHats;

bool GrabInput = false;

/*
=============================================================================

					LOCAL VARIABLES

=============================================================================
*/

static	boolean		IN_Started;

static	byte    DirTable[] =        // Quick lookup for total direction
{
    dir_NorthWest,	dir_North,	dir_NorthEast,
    dir_West,		dir_None,	dir_East,
    dir_SouthWest,	dir_South,	dir_SouthEast
};


///////////////////////////////////////////////////////////////////////////
//
//	INL_GetMouseButtons() - Gets the status of the mouse buttons from the
//		mouse driver
//
///////////////////////////////////////////////////////////////////////////
static int INL_GetMouseButtons (void)
{
    int buttons = SDL_GetMouseState(NULL,NULL);
    int middlePressed = buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE);
    int rightPressed = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);

    buttons &= ~(SDL_BUTTON(SDL_BUTTON_MIDDLE) | SDL_BUTTON(SDL_BUTTON_RIGHT));

    if (middlePressed)
        buttons |= 1 << 2;
    if (rightPressed)
        buttons |= 1 << 1;

    return buttons;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_GetJoyDelta() - Returns the relative movement of the specified
//		joystick (from +/-127)
//
///////////////////////////////////////////////////////////////////////////
void IN_GetJoyDelta (int *dx, int *dy)
{
    if (!Joystick)
    {
        *dx = *dy = 0;

        return;
    }

    SDL_JoystickUpdate();
#ifdef _arch_dreamcast
    int x = 0;
    int y = 0;
#else
    int x = SDL_JoystickGetAxis(Joystick,0) >> 8;
    int y = SDL_JoystickGetAxis(Joystick,1) >> 8;
#endif

    if (param_joystickhat != -1)
    {
        uint8_t hatState = SDL_JoystickGetHat(Joystick,param_joystickhat);

        if (hatState & SDL_HAT_RIGHT)
            x += 127;
        else if (hatState & SDL_HAT_LEFT)
            x -= 127;

        if (hatState & SDL_HAT_DOWN)
            y += 127;
        else if (hatState & SDL_HAT_UP)
            y -= 127;

        x = MAX(-128,MIN(x,127));
        y = MAX(-128,MIN(y,127));
    }

    *dx = x;
    *dy = y;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_GetJoyFineDelta() - Returns the relative movement of the specified
//		joystick without dividing the results by 256 (from +/-127)
//
///////////////////////////////////////////////////////////////////////////
void IN_GetJoyFineDelta (int *dx, int *dy)
{
    if (!Joystick)
    {
        *dx = 0;
        *dy = 0;

        return;
    }

    SDL_JoystickUpdate();

    int x = SDL_JoystickGetAxis(Joystick,0);
    int y = SDL_JoystickGetAxis(Joystick,1);

    x = MAX(-128,MIN(x,127));
    y = MAX(-128,MIN(y,127));

    *dx = x;
    *dy = y;
}

/*
===================
=
= IN_JoyButtons
=
===================
*/

int IN_JoyButtons (void)
{
    int i;

    if (!Joystick)
        return 0;

    SDL_JoystickUpdate();

    int res = 0;

    for (i = 0; i < JoyNumButtons && i < 32; i++)
        res |= SDL_JoystickGetButton(Joystick,i) << i;

    return res;
}

boolean IN_JoyPresent (void)
{
    return Joystick != NULL;
}


/*
===================
=
= Clear accumulated mouse movement
=
===================
*/

void IN_CenterMouse (void)
{
    if (MousePresent && GrabInput)
        SDL_WarpMouseInWindow (window,screenWidth / 2,screenHeight / 2);
}


/*
===================
=
= Map certain keys to their defaults
=
===================
*/

ScanCode IN_MapKey (int key)
{
    ScanCode scan = key;

    switch (key)
    {
        case sc_KeyPadEnter: scan = sc_Enter; break;
        case sc_RShift: scan = sc_LShift; break;
        case sc_RAlt: scan = sc_LAlt; break;
        case sc_RControl: scan = sc_LControl; break;

        case sc_KeyPad2:
        case sc_KeyPad4:
        case sc_KeyPad6:
        case sc_KeyPad8:
            if (!(SDL_GetModState() & KMOD_NUM))
            {
                switch (key)
                {
                    case sc_KeyPad2: scan = sc_DownArrow; break;
                    case sc_KeyPad4: scan = sc_LeftArrow; break;
                    case sc_KeyPad6: scan = sc_RightArrow; break;
                    case sc_KeyPad8: scan = sc_UpArrow; break;
                }
            }
            break;
    }

    return scan;
}


/*
===================
=
= IN_SetWindowGrab
=
===================
*/

void IN_SetWindowGrab (SDL_Window *window)
{
    const char *which[] = {"hide","show"};

    if (SDL_ShowCursor(!GrabInput) < 0)
        Quit ("Unable to %s cursor: %s\n",which[!GrabInput],SDL_GetError());

    SDL_SetWindowGrab (window,GrabInput);

    if (SDL_SetRelativeMouseMode(GrabInput))
        Quit ("Unable to set relative mode for mouse: %s\n",SDL_GetError());
}


/*
=============================================================================

                          INPUT PROCESSING

=============================================================================
=
= Input processing is not done via interrupts anymore! Instead you have to call IN_ProcessEvents
= in order to process any events like key presses or mouse movements. Alternatively you can call
= IN_WaitAndProcessEvents, which waits for an event before processing if none were available.
=
= If you have a loop with a blinking cursor where you are waiting for some key to be pressed,
= this loop MUST contain an IN_ProcessEvents/IN_WaitAndProcessEvents call, otherwise the program
= will never notice any keypress.
=
= Don't write something like, for example, "while (!keyboard[sc_X]) IN_ProcessEvents();" to
= wait for the player to press the X key, as this would result in 100% CPU usage. Either
= use IN_WaitAndProcessEvents, or add SDL_Delay(5) to the loop, which waits for 5 ms before
= it continues.
=
===========================================
*/

static void IN_HandleEvent (SDL_Event *event)
{
    int key;

    key = event->key.keysym.scancode;

    switch (event->type)
    {
        case SDL_QUIT:
            Quit (NULL);
            break;

        case SDL_KEYDOWN:
            if (key == sc_ScrollLock || key == sc_F12)
            {
                GrabInput = !GrabInput;

                IN_SetWindowGrab (window);

                return;
            }

            LastScan = IN_MapKey(key);

            if (Keyboard[sc_Alt])
            {
                if (LastScan == sc_F4)
                    Quit (NULL);
            }

            if (LastScan < sc_Last)
                Keyboard[LastScan] = true;

            if (LastScan == sc_Pause)
                Paused = true;
            break;

        case SDL_KEYUP:
            key = IN_MapKey(key);

            if (key < sc_Last)
                Keyboard[key] = false;
            break;
#if defined(GP2X)
        case SDL_JOYBUTTONDOWN:
            GP2X_ButtonDown (event->jbutton.button);
            break;

        case SDL_JOYBUTTONUP:
            GP2X_ButtonUp (event->jbutton.button);
            break;
#endif
        case SDL_TEXTINPUT:
            snprintf (textinput,sizeof(textinput),"%s",event->text.text);
            break;
    }
}


void IN_ProcessEvents (void)
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
        IN_HandleEvent (&event);
}


void IN_WaitEvent (void)
{
    if (!SDL_WaitEvent(NULL))
        Quit ("Error waiting for event: %s\n",SDL_GetError());
}


void IN_WaitAndProcessEvents (void)
{
    IN_WaitEvent ();
    IN_ProcessEvents ();
}


///////////////////////////////////////////////////////////////////////////
//
//	IN_Startup() - Starts up the Input Mgr
//
///////////////////////////////////////////////////////////////////////////
void IN_Startup(void)
{
	if (IN_Started)
		return;

    IN_ClearKeysDown();

    if (param_joystickindex >= 0 && param_joystickindex < SDL_NumJoysticks())
    {
        Joystick = SDL_JoystickOpen(param_joystickindex);

        if (Joystick)
        {
            JoyNumButtons = SDL_JoystickNumButtons(Joystick);

            if (JoyNumButtons > 32)
                JoyNumButtons = 32;      // only up to 32 buttons are supported

            JoyNumHats = SDL_JoystickNumHats(Joystick);

            if (param_joystickhat < -1 || param_joystickhat >= JoyNumHats)
                Quit ("The joystickhat param must be between 0 and %i!",JoyNumHats - 1);
        }
    }

    SDL_EventState (SDL_MOUSEMOTION,SDL_IGNORE);

    if (fullscreen || forcegrabmouse)
    {
        GrabInput = true;

        IN_SetWindowGrab (window);
    }

    // I didn't find a way to ask libSDL whether a mouse is present, yet...
#if defined(GP2X)
    MousePresent = false;
#elif defined(_arch_dreamcast)
    MousePresent = DC_MousePresent();
#else
    MousePresent = true;
#endif

    IN_Started = true;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_Shutdown() - Shuts down the Input Mgr
//
///////////////////////////////////////////////////////////////////////////
void IN_Shutdown(void)
{
	if (!IN_Started)
		return;

    if (Joystick)
        SDL_JoystickClose(Joystick);

	IN_Started = false;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_ClearKeysDown() - Clears the keyboard array
//
///////////////////////////////////////////////////////////////////////////
void IN_ClearKeysDown(void)
{
	LastScan = sc_None;

	memset (Keyboard,0,sizeof(Keyboard));
}


void IN_ClearTextInput (void)
{
    memset (textinput,0,sizeof(textinput));
}


///////////////////////////////////////////////////////////////////////////
//
//	IN_ReadControl() - Reads the device associated with the specified
//		player and fills in the control info struct
//
///////////////////////////////////////////////////////////////////////////
void IN_ReadControl (ControlInfo *info)
{
	word buttons;
	int  dx,dy;
	int  mx,my;

	dx = dy = 0;
	mx = my = 0;
	buttons = 0;

	IN_ProcessEvents();

    if (Keyboard[sc_Home])
    {
        mx = -1;
        my = -1;
    }
    else if (Keyboard[sc_PgUp])
    {
        mx = 1;
        my = -1;
    }
    else if (Keyboard[sc_End])
    {
        mx = -1;
        my = 1;
    }
    else if (Keyboard[sc_PgDn])
    {
        mx = 1;
        my = 1;
    }

    if (Keyboard[sc_UpArrow])
        my = -1;
    else if (Keyboard[sc_DownArrow])
        my = 1;

    if (Keyboard[sc_LeftArrow])
        mx = -1;
    else if (Keyboard[sc_RightArrow])
        mx = 1;

	dx = mx * 127;
	dy = my * 127;

	info->x = dx;
	info->xaxis = mx;
	info->y = dy;
	info->yaxis = my;
	info->button0 = (buttons & 1) != 0;
	info->button1 = (buttons & (1 << 1)) != 0;
	info->button2 = (buttons & (1 << 2)) != 0;
	info->button3 = (buttons & (1 << 3)) != 0;
	info->dir = DirTable[((my + 1) * 3) + (mx + 1)];
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_WaitForKey() - Waits for a scan code, then clears LastScan and
//		returns the scan code
//
///////////////////////////////////////////////////////////////////////////
ScanCode IN_WaitForKey (void)
{
	ScanCode result;

	for (result = LastScan; !result; result = LastScan)
		IN_WaitAndProcessEvents();

	LastScan = 0;

	return result;
}


///////////////////////////////////////////////////////////////////////////
//
//	IN_Ack() - waits for a button or key press.  If a button is down, upon
// calling, it must be released for it to be recognized
//
///////////////////////////////////////////////////////////////////////////

boolean	btnstate[NUMBUTTONS];

void IN_StartAck (void)
{
    int i;

    IN_ProcessEvents();
//
// get initial state of everything
//
	IN_ClearKeysDown();
	memset (btnstate,0,sizeof(btnstate));

	int buttons = IN_JoyButtons() << 4;

	if (MousePresent)
		buttons |= IN_MouseButtons();

	for (i = 0; i < NUMBUTTONS; i++, buttons >>= 1)
    {
		if (buttons & 1)
			btnstate[i] = true;
    }
}


boolean IN_CheckAck (void)
{
    int i;

    IN_ProcessEvents();
//
// see if something has been pressed
//
	if (LastScan)
		return true;

	int buttons = IN_JoyButtons() << 4;

	if (MousePresent)
		buttons |= IN_MouseButtons();

	for (i = 0; i < NUMBUTTONS; i++, buttons >>= 1)
	{
		if (buttons & 1)
		{
			if (!btnstate[i])
            {
                // Wait until button has been released
                do
                {
                    IN_WaitAndProcessEvents();

                    buttons = IN_JoyButtons() << 4;

                    if (MousePresent)
                        buttons |= IN_MouseButtons();

                } while (buttons & (1 << i));

				return true;
            }
		}
		else
			btnstate[i] = false;
	}

	return false;
}


void IN_Ack (void)
{
	IN_StartAck ();

    do
    {
        IN_WaitAndProcessEvents ();

    } while (!IN_CheckAck());
}


///////////////////////////////////////////////////////////////////////////
//
//	IN_UserInput() - Waits for the specified delay time (in ticks) or the
//		user pressing a key or a mouse button. If the clear flag is set, it
//		then either clears the key or waits for the user to let the mouse
//		button up.
//
///////////////////////////////////////////////////////////////////////////
boolean IN_UserInput (longword delay)
{
	longword	lasttime;

	lasttime = GetTimeCount();
	IN_StartAck ();

	do
	{
        IN_ProcessEvents();

		if (IN_CheckAck())
			return true;

        SDL_Delay(5);

	} while (GetTimeCount() - lasttime < delay);

	return false;
}

//===========================================================================

/*
===================
=
= IN_MouseButtons
=
===================
*/
int IN_MouseButtons (void)
{
	if (MousePresent)
		return INL_GetMouseButtons();
	else
		return 0;
}
