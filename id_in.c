/* Keen Dreams Source Code
 * Copyright (C) 2014 Javier M. Chavez
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

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

#include "id_heads.h"
#include <SDL2/SDL.h>
#pragma	hdrstop

#define	KeyInt	9	// The keyboard ISR number

// 	Stuff for the mouse
#define	MReset		0
#define	MButtons	3
#define	MDelta		11

#define	MouseInt	0x33
#define	Mouse(x)	_AX = x,geninterrupt(MouseInt)

// Stuff for the joystick
#define	JoyScaleMax		32768
#define	JoyScaleShift	8
#define	MaxJoyValue		5000

// 	Global variables
		boolean		Keyboard[NumCodes],
					JoysPresent[MaxJoys],
					MousePresent;
		Demo		DemoMode = demo_Off;
		boolean		Paused;
		char		LastASCII;
		ScanCode	LastScan;
		KeyboardDef	KbdDefs[MaxKbds] = {{0x1d,0x38,0x47,0x48,0x49,0x4b,0x4d,0x4f,0x50,0x51}};
		JoystickDef	JoyDefs[MaxJoys];
		SDL_Joystick	*Joysticks[MaxJoys];
		SDL_GameController *JoyController[MaxJoys];
		ControlType	Controls[MaxPlayers];

//	Internal variables
static	boolean		IN_Started;
static	boolean		CapsLock;
static	ScanCode	CurCode,LastCode;
static	byte        ASCIINames[] =		// Unshifted ASCII for scan codes
					{
//	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	0  ,27 ,'1','2','3','4','5','6','7','8','9','0','-','=',8  ,9  ,	// 0
	'q','w','e','r','t','y','u','i','o','p','[',']',13 ,0  ,'a','s',	// 1
	'd','f','g','h','j','k','l',';',39 ,'`',0  ,92 ,'z','x','c','v',	// 2
	'b','n','m',',','.','/',0  ,'*',0  ,' ',0  ,0  ,0  ,0  ,0  ,0  ,	// 3
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,'7','8','9','-','4','5','6','+','1',	// 4
	'2','3','0',127,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 5
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 6
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0		// 7
					},
					ShiftNames[] =		// Shifted ASCII for scan codes
					{
//	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	0  ,27 ,'!','@','#','$','%','^','&','*','(',')','_','+',8  ,9  ,	// 0
	'Q','W','E','R','T','Y','U','I','O','P','{','}',13 ,0  ,'A','S',	// 1
	'D','F','G','H','J','K','L',':',34 ,'~',0  ,'|','Z','X','C','V',	// 2
	'B','N','M','<','>','?',0  ,'*',0  ,' ',0  ,0  ,0  ,0  ,0  ,0  ,	// 3
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,'7','8','9','-','4','5','6','+','1',	// 4
	'2','3','0',127,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 5
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 6
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0   	// 7
					},
					SpecialNames[] =	// ASCII for 0xe0 prefixed codes
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
					},

					*ScanNames[] =		// Scan code names with single chars
					{
	"?","?","1","2","3","4","5","6","7","8","9","0","-","+","?","?",
	"Q","W","E","R","T","Y","U","I","O","P","[","]","|","?","A","S",
	"D","F","G","H","J","K","L",";","\"","?","?","?","Z","X","C","V",
	"B","N","M",",",".","/","?","?","?","?","?","?","?","?","?","?",
	"?","?","?","?","?","?","?","?","\xf","?","-","\x15","5","\x11","+","?",
	"\x13","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?",
	"?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?",
	"?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?"
					},	// DEBUG - consolidate these
					ExtScanCodes[] =	// Scan codes with >1 char names
					{
	1,0xe,0xf,0x1d,0x2a,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,
	0x3f,0x40,0x41,0x42,0x43,0x44,0x57,0x59,0x46,0x1c,0x36,
	0x37,0x38,0x47,0x49,0x4f,0x51,0x52,0x53,0x45,0x48,
	0x50,0x4b,0x4d,0x00
					},
					*ExtScanNames[] =	// Names corresponding to ExtScanCodes
					{
	"Esc","BkSp","Tab","Ctrl","LShft","Space","CapsLk","F1","F2","F3","F4",
	"F5","F6","F7","F8","F9","F10","F11","F12","ScrlLk","Enter","RShft",
	"PrtSc","Alt","Home","PgUp","End","PgDn","Ins","Del","NumLk","Up",
	"Down","Left","Right",""
					};
static	Direction	DirTable[] =		// Quick lookup for total direction
					{
						dir_NorthWest,	dir_North,	dir_NorthEast,
						dir_West,		dir_None,	dir_East,
						dir_SouthWest,	dir_South,	dir_SouthEast
					};

static	byte 		*DemoBuffer;
static	word		DemoOffset,DemoSize;

static	void			(*INL_KeyHook)(void);

static	char			*ParmStrings[] = {"nojoys","nomouse",nil};

//	Internal routines

///////////////////////////////////////////////////////////////////////////
//
//	INL_KeyService() - Handles a keyboard interrupt (key up/down)
//
///////////////////////////////////////////////////////////////////////////
static void
INL_KeyService(ScanCode k)
{
static	boolean	special;
		byte	c, temp;
	

	if (k == 0xe0)		// Special key prefix
		special = true;
	else if (k == 0xe1)	// Handle Pause key
		Paused = true;
	else
	{
		if (k & 0x80)	// Break code
		{
			k &= 0x7f;

// DEBUG - handle special keys: ctl-alt-delete, print scrn

			Keyboard[k] = false;
		}
		else			// Make code
		{
			LastCode = CurCode;
			CurCode = LastScan = k;
			Keyboard[k] = true;

			if (special)
				c = SpecialNames[k];
			else
			{
				if (k == sc_CapsLock)
				{
					CapsLock ^= true;
					// DEBUG - make caps lock light work
				}

				if (Keyboard[sc_LShift] || Keyboard[sc_RShift])	// If shifted
				{
					c = ShiftNames[k];
					if ((c >= 'A') && (c <= 'Z') && CapsLock)
						c += 'a' - 'A';
				}
				else
				{
					c = ASCIINames[k];
					if ((c >= 'a') && (c <= 'z') && CapsLock)
						c -= 'a' - 'A';
				}
			}
			if (c)
				LastASCII = c;
		}

		special = false;
	}

	if (INL_KeyHook && !special)
		INL_KeyHook();

}
#define INL_MapKey(sdl,in_sc) case sdl: return (in_sc)

ScanCode INL_SDLKToScanCode(int sdlKey)
{
	switch (sdlKey)
	{
		INL_MapKey(SDLK_RETURN, sc_Return);
		INL_MapKey(SDLK_ESCAPE, sc_Escape);
		INL_MapKey(SDLK_SPACE, sc_Space);
		INL_MapKey(SDLK_BACKSPACE, sc_BackSpace);
		INL_MapKey(SDLK_TAB, sc_Tab);
		INL_MapKey(SDLK_LALT, sc_Alt);
		INL_MapKey(SDLK_RALT, sc_Alt);
		INL_MapKey(SDLK_LCTRL, sc_Control);
		INL_MapKey(SDLK_RCTRL, sc_Control);
		INL_MapKey(SDLK_CAPSLOCK, sc_CapsLock);
		INL_MapKey(SDLK_LSHIFT, sc_LShift);
		INL_MapKey(SDLK_RSHIFT, sc_RShift);
		INL_MapKey(SDLK_UP, sc_UpArrow);
		INL_MapKey(SDLK_LEFT, sc_LeftArrow);
		INL_MapKey(SDLK_RIGHT, sc_RightArrow);
		INL_MapKey(SDLK_DOWN, sc_DownArrow);
		INL_MapKey(SDLK_INSERT, sc_Insert);
		INL_MapKey(SDLK_DELETE, sc_Delete);
		INL_MapKey(SDLK_HOME, sc_Home);
		INL_MapKey(SDLK_END, sc_End);
		INL_MapKey(SDLK_PAGEUP, sc_PgUp);
		INL_MapKey(SDLK_PAGEDOWN, sc_PgDn);
		
		INL_MapKey(SDLK_F1, sc_F1);
		INL_MapKey(SDLK_F2, sc_F2);
		INL_MapKey(SDLK_F3, sc_F3);
		INL_MapKey(SDLK_F4, sc_F4);
		INL_MapKey(SDLK_F5, sc_F5);
		INL_MapKey(SDLK_F6, sc_F6);
		INL_MapKey(SDLK_F7, sc_F7);
		INL_MapKey(SDLK_F8, sc_F8);
		INL_MapKey(SDLK_F9, sc_F9);
		INL_MapKey(SDLK_F10, sc_F10);

		INL_MapKey(SDLK_F11, sc_F11);
		INL_MapKey(SDLK_F12, sc_F12);

		INL_MapKey(SDLK_a, sc_A);
		INL_MapKey(SDLK_b, sc_B);
		INL_MapKey(SDLK_c, sc_C);
		INL_MapKey(SDLK_d, sc_D);
		INL_MapKey(SDLK_e, sc_E);
		INL_MapKey(SDLK_f, sc_F);
		INL_MapKey(SDLK_g, sc_G);
		INL_MapKey(SDLK_h, sc_H);
		INL_MapKey(SDLK_i, sc_I);
		INL_MapKey(SDLK_j, sc_J);
		INL_MapKey(SDLK_k, sc_K);
		INL_MapKey(SDLK_l, sc_L);
		INL_MapKey(SDLK_m, sc_M);
		INL_MapKey(SDLK_n, sc_N);
		INL_MapKey(SDLK_o, sc_O);
		INL_MapKey(SDLK_p, sc_P);
		INL_MapKey(SDLK_q, sc_Q);
		INL_MapKey(SDLK_r, sc_R);
		INL_MapKey(SDLK_s, sc_S);
		INL_MapKey(SDLK_t, sc_T);
		INL_MapKey(SDLK_u, sc_U);
		INL_MapKey(SDLK_v, sc_V);
		INL_MapKey(SDLK_w, sc_W);
		INL_MapKey(SDLK_x, sc_X);
		INL_MapKey(SDLK_y, sc_Y);
		INL_MapKey(SDLK_z, sc_Z);
		
		INL_MapKey(SDLK_0, sc_0);
		INL_MapKey(SDLK_1, sc_1);
		INL_MapKey(SDLK_2, sc_2);
		INL_MapKey(SDLK_3, sc_3);
		INL_MapKey(SDLK_4, sc_4);
		INL_MapKey(SDLK_5, sc_5);
		INL_MapKey(SDLK_6, sc_6);
		INL_MapKey(SDLK_7, sc_7);
		INL_MapKey(SDLK_8, sc_8);
		INL_MapKey(SDLK_9, sc_9);
		
		INL_MapKey(SDLK_PAUSE, 0xe1);
	default: return sc_Bad;
	}
}

#undef INL_MapKey

///////////////////////////////////////////////////////////////////////////
//
//	INL_HandleSDLEvent() - Handles an event from the system SDL2
//
///////////////////////////////////////////////////////////////////////////
static void INL_HandleSDLEvent(SDL_Event *event)
{

	ScanCode sc;
	static boolean special;
	char c;

	switch (event->type)
	{
	case SDL_QUIT:
		Quit(0);
		break;
	case SDL_KEYDOWN:
		sc = INL_SDLKToScanCode(event->key.keysym.sym);
		INL_KeyService(sc);
		break;
	case SDL_KEYUP:
		sc = INL_SDLKToScanCode(event->key.keysym.sym);
		INL_KeyService(sc | 0x80);
		break;
	}
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_PumpEvents() - Handles system events, making sure input is up
//		-to-date
//
///////////////////////////////////////////////////////////////////////////
void IN_PumpEvents(void)
{
	SDL_Event evt;
	while (SDL_PollEvent(&evt))
		INL_HandleSDLEvent(&evt);
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_GetMouseDelta() - Gets the amount that the mouse has moved from the
//		mouse driver
//
///////////////////////////////////////////////////////////////////////////
static void
INL_GetMouseDelta(int *x,int *y)
{
	SDL_GetRelativeMouseState(x, y);
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_GetMouseButtons() - Gets the status of the mouse buttons from the
//		mouse driver
//
///////////////////////////////////////////////////////////////////////////
static word
INL_GetMouseButtons(void)
{
	return SDL_GetMouseState(0,0);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_GetJoyAbs() - Reads the absolute position of the specified joystick
//
///////////////////////////////////////////////////////////////////////////
void
IN_GetJoyAbs(word joy,int *xp,int *yp)
{
	int AxisX = 0;
	int AxisY = 1;
	if (JoyController[joy])
	{
		AxisX = SDL_GameControllerGetAxis(JoyController[joy], SDL_CONTROLLER_AXIS_LEFTX);
		AxisY = SDL_GameControllerGetAxis(JoyController[joy], SDL_CONTROLLER_AXIS_LEFTY);
	}
	if (xp)
		*xp = SDL_JoystickGetAxis(Joysticks[joy], AxisX);
	if (yp)
		*yp = SDL_JoystickGetAxis(Joysticks[joy], AxisY);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_GetJoyName() - Returns the name of the Joystick from the
//		system.
//
///////////////////////////////////////////////////////////////////////////
const char*
IN_GetJoyName(word joy)
{
	if (JoyController[joy])
	{
		return SDL_GameControllerName(JoyController[joy]);
	}
	else if (joy == 0)
		return "Joystick 1";
	else if (joy == 1)
		return "Joystick 2";
	return "Unknown Joystick";
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_GetJoyDelta() - Returns the relative movement of the specified
//		joystick (from +/-127, scaled adaptively)
//
///////////////////////////////////////////////////////////////////////////
static void
INL_GetJoyDelta(word joy,int *dx,int *dy,boolean adaptive)
{
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_GetJoyButtons() - Returns the button status of the specified
//		joystick
//
///////////////////////////////////////////////////////////////////////////
static word
INL_GetJoyButtons(word joy)
{
	int buttonA = 0, buttonB = 1;
	if (JoyController[joy])
	{
		buttonA = SDL_GameControllerGetAxis(JoyController[joy], SDL_CONTROLLER_BUTTON_A);
		buttonB = SDL_GameControllerGetAxis(JoyController[joy], SDL_CONTROLLER_BUTTON_B);
	}
	return (SDL_JoystickGetButton(Joysticks[joy], buttonA) | SDL_JoystickGetButton(Joysticks[joy],buttonB) << 1);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_GetJoyButtonsDB() - Returns the de-bounced button status of the
//		specified joystick
//
///////////////////////////////////////////////////////////////////////////
word
IN_GetJoyButtonsDB(word joy)
{
	return INL_GetJoyButtons(joy);
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_StartKbd() - Sets up my keyboard stuff for use
//
///////////////////////////////////////////////////////////////////////////
static void
INL_StartKbd(void)
{
	IN_ClearKeysDown();


	INL_KeyHook = 0;	// Clear key hook
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_ShutKbd() - Restores keyboard control to the BIOS
//
///////////////////////////////////////////////////////////////////////////
static void
INL_ShutKbd(void)
{
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_StartMouse() - Detects and sets up the mouse
//
///////////////////////////////////////////////////////////////////////////
static boolean
INL_StartMouse(void)
{
	return(true);
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_ShutMouse() - Cleans up after the mouse
//
///////////////////////////////////////////////////////////////////////////
static void
INL_ShutMouse(void)
{
}

//
//	INL_SetJoyScale() - Sets up scaling values for the specified joystick
//
static void
INL_SetJoyScale(word joy)
{
	JoystickDef	*def;

	def = &JoyDefs[joy];
	def->joyMultXL = JoyScaleMax / (def->threshMinX - def->joyMinX);
	def->joyMultXH = JoyScaleMax / (def->joyMaxX - def->threshMaxX);
	def->joyMultYL = JoyScaleMax / (def->threshMinY - def->joyMinY);
	def->joyMultYH = JoyScaleMax / (def->joyMaxY - def->threshMaxY);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_SetupJoy() - Sets up thresholding values and calls INL_SetJoyScale()
//		to set up scaling values
//
///////////////////////////////////////////////////////////////////////////
void
IN_SetupJoy(word joy,word minx,word maxx,word miny,word maxy)
{
	word		d,r;
	JoystickDef	*def;

	def = &JoyDefs[joy];

	def->joyMinX = minx;
	def->joyMaxX = maxx;
	r = maxx - minx;
	d = r / 5;
	def->threshMinX = ((r / 2) - d) + minx;
	def->threshMaxX = ((r / 2) + d) + minx;

	def->joyMinY = miny;
	def->joyMaxY = maxy;
	r = maxy - miny;
	d = r / 5;
	def->threshMinY = ((r / 2) - d) + miny;
	def->threshMaxY = ((r / 2) + d) + miny;

	INL_SetJoyScale(joy);
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_StartJoy() - Detects & auto-configures the specified joystick
//					The auto-config assumes the joystick is centered
//
///////////////////////////////////////////////////////////////////////////
static boolean
INL_StartJoy(word joy)
{
	int x,y;
	
	if (joy >= SDL_NumJoysticks()) return false;
	
	Joysticks[joy] = SDL_JoystickOpen(joy);
	if (SDL_IsGameController(joy))
	{
		JoyController[joy] = SDL_GameControllerOpen(joy);
	}
	else
		JoyController[joy] = 0;
	

	IN_GetJoyAbs(joy, &x, &y);

	//IN_SetupJoy(joy, 0, x*2, 0, y*2);
	return(true);
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_ShutJoy() - Cleans up the joystick stuff
//
///////////////////////////////////////////////////////////////////////////
static void
INL_ShutJoy(word joy)
{
	JoysPresent[joy] = false;
	SDL_JoystickClose(Joysticks[joy]);
	if (JoyController[joy])
		SDL_GameControllerClose(JoyController[joy]);
}

//	Public routines

///////////////////////////////////////////////////////////////////////////
//
//	IN_Startup() - Starts up the Input Mgr
//
///////////////////////////////////////////////////////////////////////////
void
IN_Startup(void)
{
	boolean	checkjoys,checkmouse;
	word	i;

	if (IN_Started)
		return;

	checkjoys = true;
	checkmouse = true;
	for (i = 1;i < _argc;i++)
	{
		switch (US_CheckParm(_argv[i],ParmStrings))
		{
		case 0:
			checkjoys = false;
			break;
		case 1:
			checkmouse = false;
			break;
		}
	}

	if (checkjoys)
		SDL_Init(SDL_INIT_JOYSTICK);
	INL_StartKbd();
	MousePresent = checkmouse? INL_StartMouse() : false;

	for (i = 0;i < MaxJoys;i++)
		JoysPresent[i] = checkjoys? INL_StartJoy(i) : false;

	IN_Started = true;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_Default() - Sets up default conditions for the Input Mgr
//
///////////////////////////////////////////////////////////////////////////
void
IN_Default(boolean gotit,ControlType in)
{
	if
	(
		(!gotit)
	|| 	((in == ctrl_Joystick1) && !JoysPresent[0])
	|| 	((in == ctrl_Joystick2) && !JoysPresent[1])
	|| 	((in == ctrl_Mouse) && !MousePresent)
	)
		in = ctrl_Keyboard1;
	IN_SetControlType(0,in);
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

	INL_ShutMouse();
	INL_ShutKbd();

	IN_Started = false;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_SetKeyHook() - Sets the routine that gets called by INL_KeyService()
//			everytime a real make/break code gets hit
//
///////////////////////////////////////////////////////////////////////////
void
IN_SetKeyHook(void (*hook)())
{
	INL_KeyHook = hook;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_ClearKeyDown() - Clears the keyboard array
//
///////////////////////////////////////////////////////////////////////////
void
IN_ClearKeysDown(void)
{
	int	i;

	LastScan = sc_None;
	LastASCII = key_None;
	for (i = 0;i < NumCodes;i++)
		Keyboard[i] = false;
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_AdjustCursor() - Internal routine of common code from IN_ReadCursor()
//
///////////////////////////////////////////////////////////////////////////
static void
INL_AdjustCursor(CursorInfo *info,word buttons,int dx,int dy)
{
	if (buttons & (1 << 0))
		info->button0 = true;
	if (buttons & (1 << 1))
		info->button1 = true;

	info->x += dx;
	info->y += dy;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_ReadCursor() - Reads the input devices and fills in the cursor info
//		struct
//
///////////////////////////////////////////////////////////////////////////
void
IN_ReadCursor(CursorInfo *info)
{
	word	i,
			buttons;
	int		dx,dy;

	info->x = info->y = 0;
	info->button0 = info->button1 = false;

	IN_PumpEvents();
	if (MousePresent)
	{
		if (cursorhw && cursorvisible)
		{
			int x, y;
			int buttons = SDL_GetMouseState(&x, &y);
			int vx, vy, vw, vh;
			VW_GL_GetViewport(&vx, &vy, &vw, &vh);
			info->x = (x-vx)*320/vw;
			info->y = (y-vy)*200/vh;
			info->button0 = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
			info->button1 = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);
		}
		else
		{
			buttons = INL_GetMouseButtons();
			dx /= 2;
			dy /= 2;
			INL_GetMouseDelta(&dx,&dy);
			INL_AdjustCursor(info,buttons,dx,dy);
		}
	}

	for (i = 0;i < MaxJoys;i++)
	{
		if (!JoysPresent[i])
			continue;

		buttons = INL_GetJoyButtons(i);
		INL_GetJoyDelta(i,&dx,&dy,true);
		dx /= 64;
		dy /= 64;
		INL_AdjustCursor(info,buttons,dx,dy);
	}
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
			boolean		realdelta;
			byte		dbyte;
			word		buttons;
			int			dx,dy;
			Motion		mx,my;
			ControlType	type;
register	KeyboardDef	*def;

	dx = dy = 0;
	mx = my = motion_None;
	buttons = 0;

	if (DemoMode == demo_Playback)
	{
		dbyte = DemoBuffer[DemoOffset + 1];
		dy = (dbyte & 3) - 1;
		dx = ((dbyte >> 2) & 3) - 1;
		buttons = (dbyte >> 4) & 3;

		if (!DemoBuffer[DemoOffset]--)
		{
			DemoOffset += 2;
			if (DemoOffset >= DemoSize)
				DemoMode = demo_PlayDone;
		}
	}
	else if (DemoMode == demo_PlayDone)
		Quit("Demo playback exceeded");
	else
	{
		switch (type = Controls[player])
		{
		case ctrl_Keyboard1:
		case ctrl_Keyboard2:
			def = &KbdDefs[type - ctrl_Keyboard];

			if (Keyboard[def->upleft])
				mx = motion_Left,my = motion_Up;
			else if (Keyboard[def->upright])
				mx = motion_Right,my = motion_Up;
			else if (Keyboard[def->downleft])
				mx = motion_Left,my = motion_Down;
			else if (Keyboard[def->downright])
				mx = motion_Right,my = motion_Down;

			if (Keyboard[def->up])
				my = motion_Up;
			else if (Keyboard[def->down])
				my = motion_Down;

			if (Keyboard[def->left])
				mx = motion_Left;
			else if (Keyboard[def->right])
				mx = motion_Right;

			if (Keyboard[def->button0])
				buttons += 1 << 0;
			if (Keyboard[def->button1])
				buttons += 1 << 1;
			realdelta = false;
			break;
		case ctrl_Joystick1:
		case ctrl_Joystick2:
			//INL_GetJoyDelta(type - ctrl_Joystick,&dx,&dy,false);
			IN_GetJoyAbs(type - ctrl_Joystick, &dx, &dy);
			buttons = INL_GetJoyButtons(type - ctrl_Joystick);
			realdelta = true;
			break;
		case ctrl_Mouse:
			INL_GetMouseDelta(&dx,&dy);
			buttons = INL_GetMouseButtons();
			realdelta = true;
			break;
		}
	}

	if (realdelta)
	{
		mx = (dx < 0)? motion_Left : ((dx > 0)? motion_Right : motion_None);
		my = (dy < 0)? motion_Up : ((dy > 0)? motion_Down : motion_None);
	}
	else
	{
		dx = mx * 127;
		dy = my * 127;
	}

	info->x = dx;
	info->xaxis = mx;
	info->y = dy;
	info->yaxis = my;
	info->button0 = buttons & (1 << 0);
	info->button1 = buttons & (1 << 1);
	info->dir = DirTable[((my + 1) * 3) + (mx + 1)];

	if (DemoMode == demo_Record)
	{
		// Pack the control info into a byte
		dbyte = (buttons << 4) | ((dx + 1) << 2) | (dy + 1);

		if
		(
			(DemoBuffer[DemoOffset + 1] == dbyte)
		&&	(DemoBuffer[DemoOffset] < 254)
		&&	DemoOffset
		)
			DemoBuffer[DemoOffset]++;
		else
		{
			if (DemoOffset)
				DemoOffset += 2;

			if (DemoOffset >= DemoSize)
				Quit("Demo buffer overflow");

			DemoBuffer[DemoOffset] = 1;
			DemoBuffer[DemoOffset + 1] = dbyte;
		}
	}
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_SetControlType() - Sets the control type to be used by the specified
//		player
//
///////////////////////////////////////////////////////////////////////////
void
IN_SetControlType(int player,ControlType type)
{
	// DEBUG - check that type is present?
	Controls[player] = type;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_StartDemoRecord() - Starts the demo recording, using a buffer the
//		size passed. Returns if the buffer allocation was successful
//
///////////////////////////////////////////////////////////////////////////
boolean
IN_StartDemoRecord(word bufsize)
{
	if (!bufsize)
		return(false);

	MM_GetPtr((memptr *)&DemoBuffer,bufsize);
	DemoMode = demo_Record;
	DemoSize = bufsize & ~1;
	DemoOffset = 0;

	return(true);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_StartDemoPlayback() - Plays back the demo pointed to of the given size
//
///////////////////////////////////////////////////////////////////////////
void
IN_StartDemoPlayback(byte _seg *buffer,word bufsize)
{
	DemoBuffer = buffer;
	DemoMode = demo_Playback;
	DemoSize = bufsize & ~1;
	DemoOffset = 0;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_StopDemoRecord() - Turns off demo mode
//
///////////////////////////////////////////////////////////////////////////
void
IN_StopDemo(void)
{
	DemoMode = demo_Off;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_FreeDemoBuffer() - Frees the demo buffer, if it's been allocated
//
///////////////////////////////////////////////////////////////////////////
void
IN_FreeDemoBuffer(void)
{
	if (DemoBuffer)
		MM_FreePtr((memptr *)&DemoBuffer);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_GetScanName() - Returns a string containing the name of the
//		specified scan code
//
///////////////////////////////////////////////////////////////////////////
byte *
IN_GetScanName(ScanCode scan)
{
	byte		**p;
	ScanCode	*s;

	for (s = ExtScanCodes,p = ExtScanNames;*s;p++,s++)
		if (*s == scan)
			return(*p);

	return(ScanNames[scan]);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_WaitForKey() - Waits for a scan code, then clears LastScan and
//		returns the scan code
//
///////////////////////////////////////////////////////////////////////////
ScanCode IN_WaitForKey(void)
{
	SDL_Event event;
	ScanCode	result;
	while (SDL_WaitEvent(&event))
	{
		INL_HandleSDLEvent(&event);
		if ((result = LastScan)) break;
		// Refresh the display to stop steam overlay hanging.
		VW_GL_Present();
	}
	LastScan = 0;
	return (result);
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
	SDL_Event event;
	char		result;
	while (SDL_WaitEvent(&event))
	{
		INL_HandleSDLEvent(&event);
		if (!(result = LastASCII)) break;
		// Refresh the display to stop steam overlay hanging.
		VW_GL_Present();
	}
	LastASCII = '\0';
	return(result);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_AckBack() - Waits for either an ASCII keypress or a button press
//
///////////////////////////////////////////////////////////////////////////
void
IN_AckBack(void)
{
	word	i;

	while (!LastScan)
	{
		SDL_Event evt;
		if (SDL_PollEvent(&evt))
		{
			INL_HandleSDLEvent(&evt);
		}
		
		if (MousePresent)
		{
			if (INL_GetMouseButtons())
			{
				while (INL_GetMouseButtons())
					;
				return;
			}
		}

		for (i = 0;i < MaxJoys;i++)
		{
			if (JoysPresent[i])
			{
				if (IN_GetJoyButtonsDB(i))
				{
					while (IN_GetJoyButtonsDB(i))
						;
					return;
				}
			}
		}
		// Make sure the screen is being refreshed so the steam overlay works.
		VW_GL_Present();
	}

	IN_ClearKey(LastScan);
	LastScan = sc_None;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_Ack() - Clears user input & then calls IN_AckBack()
//
///////////////////////////////////////////////////////////////////////////
void
IN_Ack(void)
{
	word	i;

	IN_ClearKey(LastScan);
	LastScan = sc_None;

	if (MousePresent)
		while (INL_GetMouseButtons())
					;
	for (i = 0;i < MaxJoys;i++)
		if (JoysPresent[i])
			while (IN_GetJoyButtonsDB(i))
				;

	IN_AckBack();
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_IsUserInput() - Returns true if a key has been pressed or a button
//		is down
//
///////////////////////////////////////////////////////////////////////////
boolean
IN_IsUserInput(void)
{
	boolean	result;
	word	i;
	SDL_Event evt;
	if (SDL_PollEvent(&evt))
		INL_HandleSDLEvent(&evt);

	result = LastScan;

	if (MousePresent)
		if (INL_GetMouseButtons())
			result = true;

	for (i = 0;i < MaxJoys;i++)
		if (JoysPresent[i])
			if (INL_GetJoyButtons(i))
				result = true;

	return(result);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_UserInput() - Waits for the specified delay time (in ticks) or the
//		user pressing a key or a mouse button. If the clear flag is set, it
//		then either clears the key or waits for the user to let the mouse
//		button up.
//
///////////////////////////////////////////////////////////////////////////
boolean
IN_UserInput(longword delay,boolean clear)
{
	longword	lasttime;

	lasttime = SD_GetTimeCount();
	do
	{
		if (IN_IsUserInput())
		{
			if (clear)
				IN_AckBack();
			return(true);
		}
	} while (SD_GetTimeCount() - lasttime < delay);
	return(false);
}
