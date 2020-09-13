/* Keen Dreams (SDL2/Steam Port) Source Code
 * Copyright (C) 2014 Javier M. Chavez
 * Copyright (C) 2015 David Gow <david@davidgow.net>
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
//	ID_IN.h - Header file for Input Manager
//	v1.0d1
//	By Jason Blochowiak
//

#ifndef	__TYPES__
#include "id_types.h"
#endif

#ifndef	__ID_IN__
#define	__ID_IN__

#ifdef	__DEBUG__
#define	__DEBUG_InputMgr__
#endif

#define	MaxPlayers	4
#define	MaxKbds		2
#define	MaxJoys		2
#define	NumCodes	0x10000

#include <SDL2/SDL.h>
typedef	uint16_t		ScanCode;
#define	sc_None			0
#define	sc_Bad			0xffff
#define	sc_Return		SDL_SCANCODE_RETURN
#define	sc_Escape		SDL_SCANCODE_ESCAPE
#define	sc_Space		SDL_SCANCODE_SPACE
#define	sc_BackSpace		SDL_SCANCODE_BACKSPACE
#define	sc_Tab			SDL_SCANCODE_TAB
#define sc_Backtick		SDL_SCANCODE_GRAVE
#define sc_BackSlash		SDL_SCANCODE_BACKSLASH
#define sc_Semicolon		SDL_SCANCODE_SEMICOLON
#define sc_Quote		SDL_SCANCODE_APOSTROPHE
#define sc_Minus		SDL_SCANCODE_MINUS
#define sc_Equals		SDL_SCANCODE_EQUALS
#define	sc_Alt			SDL_SCANCODE_LALT
#define	sc_Control		SDL_SCANCODE_LCTRL
#define	sc_CapsLock		SDL_SCANCODE_CAPSLOCK
#define	sc_LShift		SDL_SCANCODE_LSHIFT
#define	sc_RShift		SDL_SCANCODE_RSHIFT
#define	sc_UpArrow		SDL_SCANCODE_UP
#define	sc_DownArrow		SDL_SCANCODE_DOWN
#define	sc_LeftArrow		SDL_SCANCODE_LEFT
#define	sc_RightArrow		SDL_SCANCODE_RIGHT
#define	sc_Insert		SDL_SCANCODE_INSERT
#define	sc_Delete		SDL_SCANCODE_DELETE
#define	sc_Home			SDL_SCANCODE_HOME
#define	sc_End			SDL_SCANCODE_END
#define	sc_PgUp			SDL_SCANCODE_PAGEUP
#define	sc_PgDn			SDL_SCANCODE_PAGEDOWN
#define	sc_F1			SDL_SCANCODE_F1
#define	sc_F2			SDL_SCANCODE_F2
#define	sc_F3			SDL_SCANCODE_F3
#define	sc_F4			SDL_SCANCODE_F4
#define	sc_F5			SDL_SCANCODE_F5
#define	sc_F6			SDL_SCANCODE_F6
#define	sc_F7			SDL_SCANCODE_F7
#define	sc_F8			SDL_SCANCODE_F8
#define	sc_F9			SDL_SCANCODE_F9
#define	sc_F10			SDL_SCANCODE_F10
#define	sc_F11			SDL_SCANCODE_F11
#define	sc_F12			SDL_SCANCODE_F12

#define	sc_A			SDL_SCANCODE_A
#define	sc_B			SDL_SCANCODE_B
#define	sc_C			SDL_SCANCODE_C
#define	sc_D			SDL_SCANCODE_D
#define	sc_E			SDL_SCANCODE_E
#define	sc_F			SDL_SCANCODE_F
#define	sc_G			SDL_SCANCODE_G
#define	sc_H			SDL_SCANCODE_H
#define	sc_I			SDL_SCANCODE_I
#define	sc_J			SDL_SCANCODE_J
#define	sc_K			SDL_SCANCODE_K
#define	sc_L			SDL_SCANCODE_L
#define	sc_M			SDL_SCANCODE_M
#define	sc_N			SDL_SCANCODE_N
#define	sc_O			SDL_SCANCODE_O
#define	sc_P			SDL_SCANCODE_P
#define	sc_Q			SDL_SCANCODE_Q
#define	sc_R			SDL_SCANCODE_R
#define	sc_S			SDL_SCANCODE_S
#define	sc_T			SDL_SCANCODE_T
#define	sc_U			SDL_SCANCODE_U
#define	sc_V			SDL_SCANCODE_V
#define	sc_W			SDL_SCANCODE_W
#define	sc_X			SDL_SCANCODE_X
#define	sc_Y			SDL_SCANCODE_Y
#define	sc_Z			SDL_SCANCODE_Z

#define sc_1			SDL_SCANCODE_1
#define sc_2			SDL_SCANCODE_2
#define sc_3			SDL_SCANCODE_3
#define sc_4			SDL_SCANCODE_4
#define sc_5			SDL_SCANCODE_5
#define sc_6			SDL_SCANCODE_6
#define sc_7			SDL_SCANCODE_7
#define sc_8			SDL_SCANCODE_8
#define sc_9			SDL_SCANCODE_9
#define sc_0			SDL_SCANCODE_0

#define sc_Comma		SDL_SCANCODE_COMMA
#define sc_Fullstop		SDL_SCANCODE_PERIOD
#define sc_Slash		SDL_SCANCODE_SLASH
#define sc_LBracket		SDL_SCANCODE_LBRACKET
#define sc_RBracket		SDL_SCANCODE_RBRACKET

#define	key_None		0
#define	key_Return		SDL_SCANCODE_KP_ENTER
#define	key_Escape		SDL_SCANCODE_ESCAPE
#define	key_Space		SDL_SCANCODE_KP_SPACE
#define	key_BackSpace		SDL_SCANCODE_BACKSPACE
#define	key_Tab			SDL_SCANCODE_KP_TAB
#define	key_Delete		SDL_SCANCODE_DELETE
#define key_Asterisk		SDL_SCANCODE_KP_MULTIPLY
#define key_Plus		SDL_SCANCODE_KP_PLUS
#define key_Minus		SDL_SCANCODE_KP_MINUS

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
						boolean		button0,button1;
						int			x,y;
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
extern	boolean		Keyboard[],
					MousePresent,
					JoysPresent[];
extern	Demo		DemoMode;
extern	boolean		Paused;
extern	char		LastASCII;
extern	ScanCode	LastScan;
extern	KeyboardDef	KbdDefs[];
extern	JoystickDef	JoyDefs[];
extern	ControlType	Controls[MaxPlayers];

// Function prototypes
#define	IN_KeyDown(code)	(Keyboard[(code)])
#define	IN_ClearKey(code)	{Keyboard[code] = false;\
							if (code == LastScan) LastScan = sc_None;}

// DEBUG - put names in prototypes
extern	void		IN_Startup(void),IN_Shutdown(void),
					IN_Default(boolean gotit,ControlType in),
					IN_SetKeyHook(void (*)()),
					IN_ClearKeysDown(void),
					IN_ReadCursor(CursorInfo *),
					IN_ReadControl(int,ControlInfo *),
					IN_SetControlType(int,ControlType),
					IN_GetJoyAbs(word joy,int *xp,int *yp),
					IN_SetupJoy(word joy,word minx,word maxx,
								word miny,word maxy),
					IN_StartDemoPlayback(byte _seg *buffer,word bufsize),
					IN_StopDemo(void),IN_FreeDemoBuffer(void),
					IN_Ack(void),IN_AckBack(void);
extern	boolean		IN_UserInput(longword delay,boolean clear),
					IN_IsUserInput(void),
					IN_StartDemoRecord(word bufsize);
extern	char		*IN_GetScanName(ScanCode);
extern	char		IN_WaitForASCII(void);
extern	ScanCode	IN_WaitForKey(void);
extern	word		IN_GetJoyButtonsDB(word joy);
extern	int		IN_GetAllJoyButtons(word joy);
extern	const char*	IN_GetJoyName(word joy);
extern	int		IN_IsJoyController(word joy);
extern	void		IN_PumpEvents(void);
extern	int		IN_ReadWheel(void);
extern	void		IN_SetJoyDelay(int ms);

#endif
