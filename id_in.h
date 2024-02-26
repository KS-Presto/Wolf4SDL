//
//	ID Engine
//	ID_IN.h - Header file for Input Manager
//	v1.0d1
//	By Jason Blochowiak
//

#ifndef	__ID_IN_H_
#define	__ID_IN_H_

#define TEXTINPUTSIZE   SDL_TEXTINPUTEVENT_TEXT_SIZE

#define	sc_None         SDL_SCANCODE_UNKNOWN
#define	sc_Bad          0x7fffffff
#define sc_Last         SDL_NUM_SCANCODES
#define	sc_Return       SDL_SCANCODE_RETURN

#define	sc_Escape       SDL_SCANCODE_ESCAPE
#define	sc_Space        SDL_SCANCODE_SPACE
#define	sc_BackSpace    SDL_SCANCODE_BACKSPACE
#define	sc_Tab          SDL_SCANCODE_TAB
#define sc_RAlt         SDL_SCANCODE_RALT
#define	sc_LAlt         SDL_SCANCODE_LALT
#define sc_RControl     SDL_SCANCODE_RCTRL
#define	sc_LControl     SDL_SCANCODE_LCTRL
#define	sc_CapsLock     SDL_SCANCODE_CAPSLOCK
#define	sc_LShift       SDL_SCANCODE_LSHIFT
#define	sc_RShift       SDL_SCANCODE_RSHIFT
#define	sc_UpArrow      SDL_SCANCODE_UP
#define	sc_DownArrow    SDL_SCANCODE_DOWN
#define	sc_LeftArrow    SDL_SCANCODE_LEFT
#define	sc_RightArrow   SDL_SCANCODE_RIGHT
#define	sc_Insert       SDL_SCANCODE_INSERT
#define	sc_Delete       SDL_SCANCODE_DELETE
#define	sc_Home         SDL_SCANCODE_HOME
#define	sc_End          SDL_SCANCODE_END
#define	sc_PgUp         SDL_SCANCODE_PAGEUP
#define	sc_PgDn         SDL_SCANCODE_PAGEDOWN
#define sc_KeyPad2      SDL_SCANCODE_KP_2
#define sc_KeyPad4      SDL_SCANCODE_KP_4
#define sc_KeyPad5      SDL_SCANCODE_KP_5
#define sc_KeyPad6      SDL_SCANCODE_KP_6
#define sc_KeyPad8      SDL_SCANCODE_KP_8
#define sc_KeyPadEnter  SDL_SCANCODE_KP_ENTER
#define	sc_F1           SDL_SCANCODE_F1
#define	sc_F2           SDL_SCANCODE_F2
#define	sc_F3           SDL_SCANCODE_F3
#define	sc_F4           SDL_SCANCODE_F4
#define	sc_F5           SDL_SCANCODE_F5
#define	sc_F6           SDL_SCANCODE_F6
#define	sc_F7           SDL_SCANCODE_F7
#define	sc_F8           SDL_SCANCODE_F8
#define	sc_F9           SDL_SCANCODE_F9
#define	sc_F10          SDL_SCANCODE_F10
#define	sc_F11          SDL_SCANCODE_F11
#define	sc_F12          SDL_SCANCODE_F12

#define sc_ScrollLock   SDL_SCANCODE_SCROLLLOCK
#define sc_PrintScreen  SDL_SCANCODE_PAUSE
#define sc_Pause        SDL_SCANCODE_PAUSE

#define	sc_1            SDL_SCANCODE_1
#define	sc_2            SDL_SCANCODE_2
#define	sc_3            SDL_SCANCODE_3
#define	sc_4            SDL_SCANCODE_4
#define	sc_5            SDL_SCANCODE_5
#define	sc_6            SDL_SCANCODE_6
#define	sc_7            SDL_SCANCODE_7
#define	sc_8            SDL_SCANCODE_8
#define	sc_9            SDL_SCANCODE_9
#define	sc_0            SDL_SCANCODE_0
#define sc_Equal        SDL_SCANCODE_EQUALS
#define sc_Minus        SDL_SCANCODE_MINUS

#define	sc_A            SDL_SCANCODE_A
#define	sc_B            SDL_SCANCODE_B
#define	sc_C            SDL_SCANCODE_C
#define	sc_D            SDL_SCANCODE_D
#define	sc_E            SDL_SCANCODE_E
#define	sc_F            SDL_SCANCODE_F
#define	sc_G            SDL_SCANCODE_G
#define	sc_H            SDL_SCANCODE_H
#define	sc_I            SDL_SCANCODE_I
#define	sc_J            SDL_SCANCODE_J
#define	sc_K            SDL_SCANCODE_K
#define	sc_L            SDL_SCANCODE_L
#define	sc_M            SDL_SCANCODE_M
#define	sc_N            SDL_SCANCODE_N
#define	sc_O            SDL_SCANCODE_O
#define	sc_P            SDL_SCANCODE_P
#define	sc_Q            SDL_SCANCODE_Q
#define	sc_R            SDL_SCANCODE_R
#define	sc_S            SDL_SCANCODE_S
#define	sc_T            SDL_SCANCODE_T
#define	sc_U            SDL_SCANCODE_U
#define	sc_V            SDL_SCANCODE_V
#define	sc_W            SDL_SCANCODE_W
#define	sc_X            SDL_SCANCODE_X
#define	sc_Y            SDL_SCANCODE_Y
#define	sc_Z            SDL_SCANCODE_Z

#define sc_Alt          sc_LAlt
#define sc_Control      sc_LControl
#define	sc_Enter        sc_Return
#define	key_None        sc_None

typedef	int32_t		ScanCode;

typedef	enum		{
						demo_Off,demo_Record,demo_Playback,demo_PlayDone
					} Demo;
typedef	enum		{
						ctrl_Keyboard,
						ctrl_Keyboard1 = ctrl_Keyboard,ctrl_Keyboard2,
						ctrl_Joystick,
						ctrl_Joystick1 = ctrl_Joystick,ctrl_Joystick2,
						ctrl_Mouse
					} ControlType;
typedef	enum		{
						motion_Left = -1,motion_Up = -1,
						motion_None = 0,
						motion_Right = 1,motion_Down = 1
					} Motion;
typedef	enum		{
						dir_North,dir_NorthEast,
						dir_East,dir_SouthEast,
						dir_South,dir_SouthWest,
						dir_West,dir_NorthWest,
						dir_None
					} Direction;
typedef	struct		{
						boolean		button0,button1,button2,button3;
						short		x,y;
						Motion		xaxis,yaxis;
						Direction	dir;
					} CursorInfo;
typedef	CursorInfo	ControlInfo;
typedef	struct		{
						ScanCode	button0,button1,
									upleft,		up,		upright,
									left,				right,
									downleft,	down,	downright;
					} KeyboardDef;
typedef	struct		{
						word		joyMinX,joyMinY,
									threshMinX,threshMinY,
									threshMaxX,threshMaxY,
									joyMaxX,joyMaxY,
									joyMultXL,joyMultYL,
									joyMultXH,joyMultYH;
					} JoystickDef;
// Global variables
extern  bool       Keyboard[sc_Last];
extern  char       textinput[TEXTINPUTSIZE];
extern  boolean    MousePresent;
extern  boolean    Paused;
extern  char       LastASCII;
extern  ScanCode   LastScan;
extern  int        JoyNumButtons;
extern  boolean    forcegrabmouse;


// Function prototypes
#define	IN_KeyDown(code)	(Keyboard[(code)])
#define	IN_ClearKey(code)	{Keyboard[code] = false;\
							if (code == LastScan) LastScan = sc_None;}

// DEBUG - put names in prototypes
extern	void		IN_Startup(void),IN_Shutdown(void);
extern	void		IN_ClearKeysDown(void);
extern	void		IN_ReadControl(int,ControlInfo *);
extern	void		IN_GetJoyAbs(word joy,word *xp,word *yp);
extern	void		IN_SetupJoy(word joy,word minx,word maxx,
								word miny,word maxy);
extern	void		IN_StopDemo(void),IN_FreeDemoBuffer(void),
					IN_Ack(void);
extern	boolean		IN_UserInput(longword delay);
extern	char		IN_WaitForASCII(void);
extern	ScanCode	IN_WaitForKey(void);
extern	word		IN_GetJoyButtonsDB(word joy);
extern	const char *IN_GetScanName(ScanCode);


void    IN_WaitAndProcessEvents();
void    IN_ProcessEvents();

int     IN_MouseButtons (void);

boolean IN_JoyPresent();
void    IN_SetJoyCurrent(int joyIndex);
int     IN_JoyButtons (void);
void    IN_GetJoyDelta(int *dx,int *dy);
void    IN_GetJoyFineDelta(int *dx, int *dy);

void    IN_StartAck(void);
boolean IN_CheckAck (void);
bool    IN_IsInputGrabbed();
void    IN_CenterMouse();

#endif
