//
//	ID Engine
//	ID_IN.c - Input Manager
//	v1.0d1
//	By Jason Blochowiak
//

//
//	This module handles dealing with the various input devices
//
//	Depends on: Memory Mgr (for demo recording), Sound Mgr (for timing stuff),
//				User Mgr (for command line parms)
//
//	Globals:
//		LastScan - The keyboard scan code of the last key pressed
//		LastASCII - The ASCII value of the last key pressed
//	DEBUG - there are more globals
//

#include "wl_def.h"
#if SDL_MAJOR_VERSION == 1
#include "SDL_keysym.h"
#endif
/*
=============================================================================

					GLOBAL VARIABLES

=============================================================================
*/


#define UNKNOWN_KEY KEYCOUNT

//
// configuration variables
//
boolean MousePresent;
boolean forcegrabmouse;

volatile boolean KeyboardState[129];

// 	Global variables
volatile boolean	Paused;
volatile char		LastASCII;
volatile ScanCode	LastScan;

//KeyboardDef	KbdDefs = {0x1d,0x38,0x47,0x48,0x49,0x4b,0x4d,0x4f,0x50,0x51};
static KeyboardDef KbdDefs = {
    sc_Control,             // button0
    sc_Alt,                 // button1
    sc_Home,                // upleft
    sc_UpArrow,             // up
    sc_PgUp,                // upright
    sc_LeftArrow,           // left
    sc_RightArrow,          // right
    sc_End,                 // downleft
    sc_DownArrow,           // down
    sc_PgDn                 // downright
};

static SDL_Joystick *Joystick;
int JoyNumButtons;
static int JoyNumHats;

static bool GrabInput = false;

/*
=============================================================================

					LOCAL VARIABLES

=============================================================================
*/
byte        ASCIINames[] =		// Unshifted ASCII for scan codes       // TODO: keypad
{
//	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,8  ,9  ,0  ,0  ,0  ,13 ,0  ,0  ,	// 0
    0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,27 ,0  ,0  ,0  ,	// 1
	' ',0  ,0  ,0  ,0  ,0  ,0  ,39 ,0  ,0  ,'*','+',',','-','.','/',	// 2
	'0','1','2','3','4','5','6','7','8','9',0  ,';',0  ,'=',0  ,0  ,	// 3
	'`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',	// 4
	'p','q','r','s','t','u','v','w','x','y','z','[',92 ,']',0  ,0  ,	// 5
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 6
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0		// 7
};
byte ShiftNames[] =		// Shifted ASCII for scan codes
{
//	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,8  ,9  ,0  ,0  ,0  ,13 ,0  ,0  ,	// 0
    0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,27 ,0  ,0  ,0  ,	// 1
	' ',0  ,0  ,0  ,0  ,0  ,0  ,34 ,0  ,0  ,'*','+','<','_','>','?',	// 2
	')','!','@','#','$','%','^','&','*','(',0  ,':',0  ,'+',0  ,0  ,	// 3
	'~','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',	// 4
	'P','Q','R','S','T','U','V','W','X','Y','Z','{','|','}',0  ,0  ,	// 5
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 6
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0		// 7
};
byte SpecialNames[] =	// ASCII for 0xe0 prefixed codes
{
//	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 0
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,13 ,0  ,0  ,0  ,	// 1
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 2
	0  ,0  ,0  ,0  ,0  ,'/',0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 3
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 4
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 5
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 6
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0   	// 7
};


static	boolean		IN_Started;

static	Direction	DirTable[] =		// Quick lookup for total direction
{
    dir_NorthWest,	dir_North,	dir_NorthEast,
    dir_West,		dir_None,	dir_East,
    dir_SouthWest,	dir_South,	dir_SouthEast
};

boolean Keyboard(int key)
{
    int keyIndex = KeyboardLookup(key);
    return keyIndex != UNKNOWN_KEY ? KeyboardState[keyIndex] : false;
}

void KeyboardSet(int key, boolean state)
{
    int keyIndex = KeyboardLookup(key);
    if(keyIndex != UNKNOWN_KEY)
    {
        KeyboardState[keyIndex] = state;
    }
}

int KeyboardLookup(int key)
{
    switch(key)
    {
        case SDLK_UNKNOWN		:  return 0;
	    case SDLK_BACKSPACE		: return 1;
	    case SDLK_TAB		: return 2;
	    case SDLK_CLEAR		: return 3;
	    case SDLK_RETURN		: return 4;
	    case SDLK_PAUSE		: return 5;
	    case SDLK_ESCAPE		: return 6;
	    case SDLK_SPACE		: return 7;
	    case SDLK_EXCLAIM		: return 8;
	    case SDLK_QUOTEDBL		: return 9;
	    case SDLK_HASH		: return 10;
	    case SDLK_DOLLAR		: return 11;
	    case SDLK_AMPERSAND		: return 12;
	    case SDLK_QUOTE		: return 13;
	    case SDLK_LEFTPAREN		: return 14;
	    case SDLK_RIGHTPAREN		: return 15;
	    case SDLK_ASTERISK		: return 16;
	    case SDLK_PLUS		: return 17;
	    case SDLK_COMMA		: return 18;
	    case SDLK_MINUS		: return 19;
	    case SDLK_PERIOD		: return 20;
	    case SDLK_SLASH		: return 21;
	    case SDLK_0			: return 22;
	    case SDLK_1			: return 23;
	    case SDLK_2			: return 24;
	    case SDLK_3			: return 25;
	    case SDLK_4			: return 26;
	    case SDLK_5			: return 27;
	    case SDLK_6			: return 28;
	    case SDLK_7			: return 29;
	    case SDLK_8			: return 30;
	    case SDLK_9			: return 31;
	    case SDLK_COLON		: return 32;
	    case SDLK_SEMICOLON		: return 33;
	    case SDLK_LESS		: return 34;
	    case SDLK_EQUALS		: return 35;
	    case SDLK_GREATER		: return 36;
	    case SDLK_QUESTION		: return 37;
	    case SDLK_AT			: return 38;
	    case SDLK_LEFTBRACKET	: return 39;
	    case SDLK_BACKSLASH		: return 40;
	    case SDLK_RIGHTBRACKET	: return 41;
	    case SDLK_CARET		: return 42;
	    case SDLK_UNDERSCORE		: return 43;
	    case SDLK_BACKQUOTE		: return 44;
	    case SDLK_a			: return 45;
	    case SDLK_b			: return 46;
	    case SDLK_c			: return 47;
	    case SDLK_d			: return 48;
	    case SDLK_e			: return 49;
	    case SDLK_f			: return 50;
	    case SDLK_g			: return 51;
	    case SDLK_h			: return 52;
	    case SDLK_i			: return 53;
	    case SDLK_j			: return 54;
	    case SDLK_k			: return 55;
	    case SDLK_l			: return 56;
	    case SDLK_m			: return 57;
	    case SDLK_n			: return 58;
	    case SDLK_o			: return 59;
	    case SDLK_p			: return 60;
	    case SDLK_q			: return 61;
	    case SDLK_r			: return 62;
	    case SDLK_s			: return 63;
	    case SDLK_t			: return 64;
	    case SDLK_u			: return 65;
	    case SDLK_v			: return 66;
	    case SDLK_w			: return 67;
	    case SDLK_x			: return 68;
	    case SDLK_y			: return 69;
	    case SDLK_z			: return 70;
	    case SDLK_DELETE		: return 71;
        case SDLK_KP_PERIOD		: return 72;
	    case SDLK_KP_DIVIDE		: return 73;
	    case SDLK_KP_MULTIPLY	: return 74;
	    case SDLK_KP_MINUS		: return 75;
	    case SDLK_KP_PLUS		: return 76;
	    case SDLK_KP_ENTER		: return 77;
	    case SDLK_KP_EQUALS		: return 78;
	    case SDLK_UP			: return 79;
	    case SDLK_DOWN		: return 80;
	    case SDLK_RIGHT		: return 81;
	    case SDLK_LEFT		: return 82;
	    case SDLK_INSERT		: return 83;
	    case SDLK_HOME		: return 84;
	    case SDLK_END		: return 85;
	    case SDLK_PAGEUP		: return 86;
	    case SDLK_PAGEDOWN		: return 87;
	    case SDLK_F1			: return 88;
	    case SDLK_F2			: return 89;
	    case SDLK_F3			: return 90;
	    case SDLK_F4			: return 91;
	    case SDLK_F5			: return 92;
	    case SDLK_F6			: return 93;
	    case SDLK_F7			: return 94;
	    case SDLK_F8			: return 95;
	    case SDLK_F9			: return 96;
	    case SDLK_F10		: return 97;
	    case SDLK_F11		: return 98;
	    case SDLK_F12		: return 99;
	    case SDLK_F13		: return 100;
	    case SDLK_F14		: return 101;
	    case SDLK_F15		: return 102;
	    case SDLK_CAPSLOCK		: return 103;
	    case SDLK_RSHIFT		: return 104;
	    case SDLK_LSHIFT		: return 105;
	    case SDLK_RCTRL		: return 106;
	    case SDLK_LCTRL		: return 107;
	    case SDLK_RALT		: return 108;
	    case SDLK_LALT		: return 109;
	    case SDLK_MODE		: return 110;		
	    case SDLK_HELP		: return 111;
	    case SDLK_SYSREQ		: return 112;
	    case SDLK_MENU		: return 113;
	    case SDLK_POWER		: return 114;
	    case SDLK_UNDO		: return 115;
        case SDLK_KP_0		: return 116;
	    case SDLK_KP_1		: return 117;
	    case SDLK_KP_2		: return 118;
	    case SDLK_KP_3		: return 119;
	    case SDLK_KP_4		: return 120;
	    case SDLK_KP_5		: return 121;
	    case SDLK_KP_6		: return 122;
	    case SDLK_KP_7		: return 123;
	    case SDLK_KP_8		: return 124;
	    case SDLK_KP_9		: return 125;
        case SDLK_PRINTSCREEN		: return 126;
        case SDLK_NUMLOCKCLEAR		: return 127;
        case SDLK_SCROLLLOCK		: return 128;
        default : return UNKNOWN_KEY;
    }
}


///////////////////////////////////////////////////////////////////////////
//
//	INL_GetMouseButtons() - Gets the status of the mouse buttons from the
//		mouse driver
//
///////////////////////////////////////////////////////////////////////////
static int
INL_GetMouseButtons(void)
{
    int buttons = SDL_GetMouseState(NULL, NULL);
    int middlePressed = buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE);
    int rightPressed = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);
    buttons &= ~(SDL_BUTTON(SDL_BUTTON_MIDDLE) | SDL_BUTTON(SDL_BUTTON_RIGHT));
    if(middlePressed) buttons |= 1 << 2;
    if(rightPressed) buttons |= 1 << 1;

    return buttons;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_GetJoyDelta() - Returns the relative movement of the specified
//		joystick (from +/-127)
//
///////////////////////////////////////////////////////////////////////////
void IN_GetJoyDelta(int *dx,int *dy)
{
    if(!Joystick)
    {
        *dx = *dy = 0;
        return;
    }

    SDL_JoystickUpdate();
#ifdef _arch_dreamcast
    int x = 0;
    int y = 0;
#else
    int x = SDL_JoystickGetAxis(Joystick, 0) >> 8;
    int y = SDL_JoystickGetAxis(Joystick, 1) >> 8;
#endif

    if(param_joystickhat != -1)
    {
        uint8_t hatState = SDL_JoystickGetHat(Joystick, param_joystickhat);
        if(hatState & SDL_HAT_RIGHT)
            x += 127;
        else if(hatState & SDL_HAT_LEFT)
            x -= 127;
        if(hatState & SDL_HAT_DOWN)
            y += 127;
        else if(hatState & SDL_HAT_UP)
            y -= 127;

        if(x < -128) x = -128;
        else if(x > 127) x = 127;

        if(y < -128) y = -128;
        else if(y > 127) y = 127;
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
void IN_GetJoyFineDelta(int *dx, int *dy)
{
    if(!Joystick)
    {
        *dx = 0;
        *dy = 0;
        return;
    }

    SDL_JoystickUpdate();
    int x = SDL_JoystickGetAxis(Joystick, 0);
    int y = SDL_JoystickGetAxis(Joystick, 1);

    if(x < -128) x = -128;
    else if(x > 127) x = 127;

    if(y < -128) y = -128;
    else if(y > 127) y = 127;

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

int IN_JoyButtons()
{
    int i;

    if(!Joystick) return 0;

    SDL_JoystickUpdate();

    int res = 0;
    for(i = 0; i < JoyNumButtons && i < 32; i++)
        res |= SDL_JoystickGetButton(Joystick, i) << i;
    return res;
}

boolean IN_JoyPresent()
{
    return Joystick != NULL;
}

static void processEvent(SDL_Event *event)
{
    switch (event->type)
    {
        // exit if the window is closed
        case SDL_QUIT:
            Quit(NULL);

        // check for keypresses
        case SDL_KEYDOWN:
        {
            if(event->key.keysym.sym==SDLK_SCROLLLOCK || event->key.keysym.sym==SDLK_F12)
            {
                GrabInput = !GrabInput;
#if SDL_MAJOR_VERSION == 1
                SDL_WM_GrabInput(GrabInput ? SDL_GRAB_ON : SDL_GRAB_OFF);
#else
                SDL_SetRelativeMouseMode(GrabInput ? SDL_TRUE : SDL_FALSE);
#endif
                return;
            }

            LastScan = event->key.keysym.sym;
            SDL_Keymod mod = SDL_GetModState();
            if(Keyboard(sc_Alt))
            {
                if(LastScan==SDLK_F4)
                    Quit(NULL);
            }

            if(LastScan == SDLK_KP_ENTER) LastScan = SDLK_RETURN;
            else if(LastScan == SDLK_RSHIFT) LastScan = SDLK_LSHIFT;
            else if(LastScan == SDLK_RALT) LastScan = SDLK_LALT;
            else if(LastScan == SDLK_RCTRL) LastScan = SDLK_LCTRL;
            else
            {
                if((mod & KMOD_NUM) == 0)
                {
                    switch(LastScan)
                    {
                        case SDLK_KP_2: LastScan = SDLK_DOWN; break;
                        case SDLK_KP_4: LastScan = SDLK_LEFT; break;
                        case SDLK_KP_6: LastScan = SDLK_RIGHT; break;
                        case SDLK_KP_8: LastScan = SDLK_UP; break;
                    }
                }
            }

            int sym = LastScan;
            if(sym >= 'a' && sym <= 'z')
                sym -= 32;  // convert to uppercase

            if(mod & (KMOD_SHIFT | KMOD_CAPS))
            {
                if(sym < lengthof(ShiftNames) && ShiftNames[sym])
                    LastASCII = ShiftNames[sym];
            }
            else
            {
                if(sym < lengthof(ASCIINames) && ASCIINames[sym])
                    LastASCII = ASCIINames[sym];
            }

            int intLastScan = LastScan;
            KeyboardSet(intLastScan, 1);

            if(LastScan == SDLK_PAUSE)
                Paused = true;
            break;
		}

        case SDL_KEYUP:
        {
            int key = event->key.keysym.sym;
            if(key == SDLK_KP_ENTER) key = SDLK_RETURN;
            else if(key == SDLK_RSHIFT) key = SDLK_LSHIFT;
            else if(key == SDLK_RALT) key = SDLK_LALT;
            else if(key == SDLK_RCTRL) key = SDLK_LCTRL;
            else
            {
                if((SDL_GetModState() & KMOD_NUM) == 0)
                {
                    switch(key)
                    {
                        case SDLK_KP_2: key = SDLK_DOWN; break;
                        case SDLK_KP_4: key = SDLK_LEFT; break;
                        case SDLK_KP_6: key = SDLK_RIGHT; break;
                        case SDLK_KP_8: key = SDLK_UP; break;
                    }
                }
            }

            KeyboardSet(key, 0);
        }

#if defined(GP2X)
        case SDL_JOYBUTTONDOWN:
            GP2X_ButtonDown(event->jbutton.button);
            break;

        case SDL_JOYBUTTONUP:
            GP2X_ButtonUp(event->jbutton.button);
            break;
#endif
    }
}

void IN_WaitAndProcessEvents()
{
    SDL_Event event;
    if(!SDL_WaitEvent(&event)) return;
    do
    {
        processEvent(&event);
    }
    while(SDL_PollEvent(&event));
}

void IN_ProcessEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        processEvent(&event);
    }
}


///////////////////////////////////////////////////////////////////////////
//
//	IN_Startup() - Starts up the Input Mgr
//
///////////////////////////////////////////////////////////////////////////
void
IN_Startup(void)
{
	if (IN_Started)
		return;

    IN_ClearKeysDown();

    if(param_joystickindex >= 0 && param_joystickindex < SDL_NumJoysticks())
    {
        Joystick = SDL_JoystickOpen(param_joystickindex);
        if(Joystick)
        {
            JoyNumButtons = SDL_JoystickNumButtons(Joystick);
            if(JoyNumButtons > 32) JoyNumButtons = 32;      // only up to 32 buttons are supported
            JoyNumHats = SDL_JoystickNumHats(Joystick);
            if(param_joystickhat < -1 || param_joystickhat >= JoyNumHats)
                Quit("The joystickhat param must be between 0 and %i!", JoyNumHats - 1);
        }
    }

    SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);

    if(fullscreen || forcegrabmouse)
    {
        GrabInput = true;
#if SDL_MAJOR_VERSION == 1
        SDL_WM_GrabInput(SDL_GRAB_ON);
#else
        SDL_SetRelativeMouseMode(SDL_TRUE);
#endif
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
void
IN_Shutdown(void)
{
	if (!IN_Started)
		return;

    if(Joystick)
        SDL_JoystickClose(Joystick);

	IN_Started = false;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_ClearKeysDown() - Clears the keyboard array
//
///////////////////////////////////////////////////////////////////////////
void
IN_ClearKeysDown(void)
{
	LastScan = sc_None;
	LastASCII = key_None;
	memset ((void *) KeyboardState,0,sizeof(KeyboardState));
}


///////////////////////////////////////////////////////////////////////////
//
//	IN_ReadControl() - Reads the device associated with the specified
//		player and fills in the control info struct
//
///////////////////////////////////////////////////////////////////////////
void
IN_ReadControl(int player,ControlInfo *info)
{
	word		buttons;
	int			dx,dy;
	Motion		mx,my;

	dx = dy = 0;
	mx = my = motion_None;
	buttons = 0;

	IN_ProcessEvents();

    if (Keyboard(KbdDefs.upleft))
        mx = motion_Left,my = motion_Up;
    else if (Keyboard(KbdDefs.upright))
        mx = motion_Right,my = motion_Up;
    else if (Keyboard(KbdDefs.downleft))
        mx = motion_Left,my = motion_Down;
    else if (Keyboard(KbdDefs.downright))
        mx = motion_Right,my = motion_Down;

    if (Keyboard(KbdDefs.up))
        my = motion_Up;
    else if (Keyboard(KbdDefs.down))
        my = motion_Down;

    if (Keyboard(KbdDefs.left))
        mx = motion_Left;
    else if (Keyboard(KbdDefs.right))
        mx = motion_Right;

    if (Keyboard(KbdDefs.button0))
        buttons += 1 << 0;
    if (Keyboard(KbdDefs.button1))
        buttons += 1 << 1;

	dx = mx * 127;
	dy = my * 127;

	info->x = dx;
	info->xaxis = mx;
	info->y = dy;
	info->yaxis = my;
	info->button0 = (buttons & (1 << 0)) != 0;
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
ScanCode
IN_WaitForKey(void)
{
	ScanCode	result;

	while ((result = LastScan)==0)
		IN_WaitAndProcessEvents();
	LastScan = 0;
	return(result);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_WaitForASCII() - Waits for an ASCII char, then clears LastASCII and
//		returns the ASCII value
//
///////////////////////////////////////////////////////////////////////////
char
IN_WaitForASCII(void)
{
	char		result;

	while ((result = LastASCII)==0)
		IN_WaitAndProcessEvents();
	LastASCII = '\0';
	return(result);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_Ack() - waits for a button or key press.  If a button is down, upon
// calling, it must be released for it to be recognized
//
///////////////////////////////////////////////////////////////////////////

boolean	btnstate[NUMBUTTONS];

void IN_StartAck(void)
{
    int i;

    IN_ProcessEvents();
//
// get initial state of everything
//
	IN_ClearKeysDown();
	memset(btnstate, 0, sizeof(btnstate));

	int buttons = IN_JoyButtons() << 4;

	if(MousePresent)
		buttons |= IN_MouseButtons();

	for(i = 0; i < NUMBUTTONS; i++, buttons >>= 1)
		if(buttons & 1)
			btnstate[i] = true;
}


boolean IN_CheckAck (void)
{
    int i;

    IN_ProcessEvents();
//
// see if something has been pressed
//
	if(LastScan)
		return true;

	int buttons = IN_JoyButtons() << 4;

	if(MousePresent)
		buttons |= IN_MouseButtons();

	for(i = 0; i < NUMBUTTONS; i++, buttons >>= 1)
	{
		if(buttons & 1)
		{
			if(!btnstate[i])
            {
                // Wait until button has been released
                do
                {
                    IN_WaitAndProcessEvents();
                    buttons = IN_JoyButtons() << 4;

                    if(MousePresent)
                        buttons |= IN_MouseButtons();
                }
                while(buttons & (1 << i));

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
        IN_WaitAndProcessEvents();
    }
	while(!IN_CheckAck ());
}


///////////////////////////////////////////////////////////////////////////
//
//	IN_UserInput() - Waits for the specified delay time (in ticks) or the
//		user pressing a key or a mouse button. If the clear flag is set, it
//		then either clears the key or waits for the user to let the mouse
//		button up.
//
///////////////////////////////////////////////////////////////////////////
boolean IN_UserInput(longword delay)
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
	return(false);
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

bool IN_IsInputGrabbed()
{
    return GrabInput;
}

void IN_CenterMouse()
{
#if SDL_MAJOR_VERSION == 1
    SDL_WarpMouse(screenWidth / 2, screenHeight / 2);
#endif
}
