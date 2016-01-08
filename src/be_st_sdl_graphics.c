/* Copyright (C) 2014-2015 NY00123
 *
 * This file is part of Reflection Keen.
 *
 * Reflection Keen is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <string.h>
#include "SDL.h"

#include "be_cross.h"
#include "be_gamever.h" // Enable VSync by default for EGA, not CGA
#include "be_st.h"
#include "be_st_sdl_private.h"
#include "be_title_and_version.h"

// Some of these are also used in launcher
SDL_Window *g_sdlWindow;
SDL_Renderer *g_sdlRenderer;
SDL_Texture *g_sdlTexture, *g_sdlTargetTexture;
SDL_Rect g_sdlAspectCorrectionRect, g_sdlAspectCorrectionBorderedRect;

static bool g_sdlIsSoftwareRendered;
static bool g_sdlDoRefreshGfxOutput;
bool g_sdlForceGfxControlUiRefresh;

void BE_ST_MarkGfxForUpdate(void)
{
	g_sdlDoRefreshGfxOutput = true;
}

#if !SDL_VERSION_ATLEAST(2,0,0)
#error "SDL <2.0 support is unimplemented!"
#endif

#define GFX_TEX_WIDTH 320
#define GFX_TEX_HEIGHT 200
#define VGA_TXT_TEX_WIDTH 720
#define VGA_TXT_TEX_HEIGHT 400
//#define EGACGA_TXT_TEX_WIDTH 640
//#define EGACGA_TXT_TEX_HEIGHT 200

// Overscan border dimensions (for each side of the screen)
#define ENGINE_VGA_GFX_OVERSCAN_LEFT_AFTER_DOUBLING 16 // Doubling from 8 for us
#define ENGINE_VGA_GFX_OVERSCAN_RIGHT_AFTER_DOUBLING 16 // Doubling from 8 for us
#define ENGINE_VGA_GFX_OVERSCAN_TOP_AFTER_DOUBLING 7    // 200-line doubling
#define ENGINE_VGA_GFX_OVERSCAN_BOTTOM_AFTER_DOUBLING 7 // 200-line doubling
#define ENGINE_VGA_TXT_OVERSCAN_LEFT 9
#define ENGINE_VGA_TXT_OVERSCAN_RIGHT 9
#define ENGINE_VGA_TXT_OVERSCAN_TOP 7
#define ENGINE_VGA_TXT_OVERSCAN_BOTTOM 7

#define TXT_COLS_NUM 80
#define TXT_ROWS_NUM 25

#define VGA_TXT_CHAR_PIX_WIDTH (VGA_TXT_TEX_WIDTH/TXT_COLS_NUM)
#define VGA_TXT_CHAR_PIX_HEIGHT (VGA_TXT_TEX_HEIGHT/TXT_ROWS_NUM)

#define VGA_TXT_CURSOR_BLINK_VERT_FRAME_RATE 8
#define VGA_TXT_BLINK_VERT_FRAME_RATE 16


extern const uint8_t g_vga_8x16TextFont[256*8*16];
// We can use a union because the memory contents are refreshed on screen mode change
// (well, not on change between modes 0xD and 0xE, both sharing planar A000:0000)
static union {
	uint8_t egaGfx[4][0x10000]; // Contents of A000:0000 (4 planes)
	uint8_t text[TXT_COLS_NUM*TXT_ROWS_NUM*2]; // Textual contents of B800:0000
} g_sdlVidMem;

// Used for simple caching of EGA graphics (due to page flipping and more)
// and similarly CGA graphics (modified only at one place)
static union {
	uint8_t egaGfx[2*GFX_TEX_WIDTH*GFX_TEX_HEIGHT]; // Support 640x200 mode for Catacomb Abyss
	uint8_t cgaGfx[GFX_TEX_WIDTH*GFX_TEX_HEIGHT];
} g_sdlHostScrMem, g_sdlHostScrMemCache;

static uint16_t g_sdlScreenStartAddress = 0;
static int g_sdlScreenMode = 3;
static int g_sdlTexWidth, g_sdlTexHeight;
static uint8_t g_sdlPelPanning = 0;
static uint8_t g_sdlLineWidth = 40;
static int16_t g_sdlSplitScreenLine = -1;
static int g_sdlTxtCursorPosX, g_sdlTxtCursorPosY;
static bool g_sdlTxtCursorEnabled = true;
static int g_sdlTxtColor = 7, g_sdlTxtBackground = 0;

/*** Game controller UI resource definitions ***/

#include "../rsrc/pad_font_mono.xpm"
#include "../rsrc/pad_thumb_buttons.xpm"
#include "../rsrc/pad_dpad.xpm"

#define ALTCONTROLLER_FONT_XPM_ROW_OFFSET 3
#define ALTCONTROLLER_PAD_XPM_ROW_OFFSET 8

#define ALTCONTROLLER_PAD_PIX_WIDTH 48
#define ALTCONTROLLER_PAD_PIX_HEIGHT 48
#define ALTCONTROLLER_CHAR_PIX_WIDTH 6
#define ALTCONTROLLER_CHAR_PIX_HEIGHT 8
#define ALTCONTROLLER_CHAR_TOTAL_PIX_WIDTH 570

#define ALTCONTROLLER_EDGE_PIX_DIST 2
#define ALTCONTROLLER_FACEBUTTONS_SCREEN_DIM_RATIO 5

// Measured in keys
#define ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH 22
#define ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT 12

#define ALTCONTROLLER_TEXTINPUT_KEYS_WIDTH 18
#define ALTCONTROLLER_TEXTINPUT_KEYS_HEIGHT 3

#define ALTCONTROLLER_TEXTINPUT_PIX_WIDTH (ALTCONTROLLER_TEXTINPUT_KEYS_WIDTH*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH)
#define ALTCONTROLLER_TEXTINPUT_PIX_HEIGHT (ALTCONTROLLER_TEXTINPUT_KEYS_HEIGHT*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT)

#define ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH 17
#define ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT 5

#define ALTCONTROLLER_DEBUGKEYS_PIX_WIDTH (ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH)
#define ALTCONTROLLER_DEBUGKEYS_PIX_HEIGHT (ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT)

// These are given as (x, y) offset pairs within the non-scaled,
// face buttons image, assuming longest texts possible (3 chars long)
static const int g_sdlControllerFaceButtonsTextLocs[] = {15, 34, 28, 21, 2, 21, 15, 8};

static SDL_Rect g_sdlControllerFaceButtonsRect, g_sdlControllerDpadRect, g_sdlControllerTextInputRect, g_sdlControllerDebugKeysRect;
static SDL_Texture *g_sdlFaceButtonsTexture, *g_sdlDpadTexture, *g_sdlTextInputTexture, *g_sdlDebugKeysTexture;
static bool g_sdlFaceButtonsAreShown, g_sdlDpadIsShown, g_sdlTextInputUIIsShown, g_sdlDebugKeysUIIsShown;

static int g_sdlFaceButtonsScanCodes[4], g_sdlDpadScanCodes[4];
static int g_sdlPointerSelectedPadButtonScanCode;
static bool g_sdlControllerUIPointerPressed;

// With alternative game controllers scheme, all UI is hidden if no controller is connected
bool g_sdlShowControllerUI;

// Shared among all kinds of keyboard UI
static int g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY;
static bool g_sdlKeyboardUIPointerUsed;
// Text input specific
static bool g_sdlTextInputIsKeyPressed, g_sdlTextInputIsShifted;
// Debug keys specific
static bool g_sdlDebugKeysPressed[ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT][ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH];


void BE_ST_InitGfx(void)
{
	if (g_refKeenCfg.sdlRendererDriver >= 0)
	{
		SDL_RendererInfo info;
		SDL_GetRenderDriverInfo(g_refKeenCfg.sdlRendererDriver, &info);
		g_sdlIsSoftwareRendered = (info.flags & SDL_RENDERER_SOFTWARE);
	}
	else
	{
		g_sdlIsSoftwareRendered = false;
	}

	if (g_refKeenCfg.isFullscreen)
	{
		if (g_refKeenCfg.fullWidth && g_refKeenCfg.fullHeight)
		{
			g_sdlWindow = SDL_CreateWindow(REFKEEN_TITLE_AND_VER_STRING, SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_refKeenCfg.displayNum), SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_refKeenCfg.displayNum), g_refKeenCfg.fullWidth, g_refKeenCfg.fullHeight, SDL_WINDOW_FULLSCREEN);
		}
		else
		{
			g_sdlWindow = SDL_CreateWindow(REFKEEN_TITLE_AND_VER_STRING, SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_refKeenCfg.displayNum), SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_refKeenCfg.displayNum), 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
		}
	}
	else
	{
		int actualWinWidth = g_refKeenCfg.winWidth, actualWinHeight = g_refKeenCfg.winHeight;
		if (!actualWinWidth || !actualWinHeight)
		{
			if (g_sdlIsSoftwareRendered)
			{
				actualWinWidth = 640;
				actualWinHeight = 480;
			}
			else
			{
				SDL_DisplayMode mode;
				SDL_GetDesktopDisplayMode(g_refKeenCfg.displayNum, &mode);
				// In the 200-lines modes on the VGA, where line doubling is in effect,
				// and after adding the overscan borders, the aspect ratio for the whole output
				// (after aspect correction i.e., multiplying height by 1.2) is 280:207.
				if (207*mode.w < 280*mode.h) // Thinner than 280:207
				{
					mode.h = mode.w*207/280;
				}
				else  // As wide as 280:207 at the least
				{
					mode.w = mode.h*280/207;
				}
				// Just for the sake of it, using the golden ratio...
				actualWinWidth = mode.w*500/809;
				actualWinHeight = mode.h*500/809;
			}
		}
		g_sdlWindow = SDL_CreateWindow(REFKEEN_TITLE_AND_VER_STRING, SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_refKeenCfg.displayNum), SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_refKeenCfg.displayNum), actualWinWidth, actualWinHeight, (!g_sdlIsSoftwareRendered || g_refKeenCfg.forceFullSoftScaling) ? SDL_WINDOW_RESIZABLE : 0);
	}
	if (!g_sdlWindow)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to create SDL2 window,\n%s\n", SDL_GetError());
		exit(0);
	}
	SDL_SetWindowIcon(g_sdlWindow, g_be_sdl_windowIconSurface);
	// Vanilla Keen Dreams and Keen 4-6 have no VSync in the CGA builds
#ifdef REFKEEN_VER_KDREAMS
	g_sdlRenderer = SDL_CreateRenderer(g_sdlWindow, g_refKeenCfg.sdlRendererDriver, SDL_RENDERER_ACCELERATED | (((g_refKeenCfg.vSync == VSYNC_ON) || ((g_refKeenCfg.vSync == VSYNC_AUTO) && (refkeen_current_gamever != BE_GAMEVER_KDREAMSC105))) ? SDL_RENDERER_PRESENTVSYNC : 0));
#else
	g_sdlRenderer = SDL_CreateRenderer(g_sdlWindow, g_refKeenCfg.sdlRendererDriver, SDL_RENDERER_ACCELERATED | ((g_refKeenCfg.vSync != VSYNC_OFF) ? SDL_RENDERER_PRESENTVSYNC : 0));
#endif
	if (!g_sdlRenderer)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to create SDL2 renderer,\n%s\n", SDL_GetError());
		//Destroy window?
		exit(0);
	}
	BE_ST_SetScreenMode(3); // Includes SDL_Texture handling and output rects preparation
}

void BE_ST_ShutdownGfx(void)
{
	SDL_DestroyTexture(g_sdlFaceButtonsTexture);
	g_sdlFaceButtonsTexture = NULL;
	SDL_DestroyTexture(g_sdlDpadTexture);
	g_sdlDpadTexture = NULL;
	SDL_DestroyTexture(g_sdlTextInputTexture);
	g_sdlTextInputTexture = NULL;
	SDL_DestroyTexture(g_sdlDebugKeysTexture);
	g_sdlDebugKeysTexture = NULL;
	SDL_DestroyTexture(g_sdlTexture);
	g_sdlTexture = NULL;
	SDL_DestroyTexture(g_sdlTargetTexture);
	g_sdlTargetTexture = NULL;
	SDL_DestroyRenderer(g_sdlRenderer);
	g_sdlRenderer = NULL;
	SDL_DestroyWindow(g_sdlWindow);
	g_sdlWindow = NULL;
}

static void BEL_ST_RecreateTexture(void)
{
	if (g_sdlTexture)
	{
		SDL_DestroyTexture(g_sdlTexture);
	}
	if (g_sdlTargetTexture)
	{
		SDL_DestroyTexture(g_sdlTargetTexture);
	}
	// Try using render target
	if ((g_refKeenCfg.scaleFactor > 1) && g_refKeenCfg.isBilinear)
	{
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
		g_sdlTexture = SDL_CreateTexture(g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, g_sdlTexWidth, g_sdlTexHeight);
		if (!g_sdlTexture)
		{
			BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 texture,\n%s\n", SDL_GetError());
			//Destroy window and renderer?
			exit(0);
		}
		// Try, if we fail then simply don't use this
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
		g_sdlTargetTexture = SDL_CreateTexture(g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, g_sdlTexWidth*g_refKeenCfg.scaleFactor, g_sdlTexHeight*g_refKeenCfg.scaleFactor);
	}
	else
	{
		// Use just a single texture
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, g_refKeenCfg.isBilinear ? "linear" : "nearest");
		g_sdlTexture = SDL_CreateTexture(g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, g_sdlTexWidth, g_sdlTexHeight);
		if (!g_sdlTexture)
		{
			BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 texture,\n%s\n", SDL_GetError());
			//Destroy window and renderer?
			exit(0);
		}
	}
}

// Scancode names for controller face buttons and d-pad UI, based on DOS
// scancodes (doesn't include "special" keys for which 0xE0 is sent) and more
static const char *g_sdlDOSScanCodePadStrs[] = {
	NULL, "Esc",
	"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "\x11",
	"Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "Ent",
	"Ctl", "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "`",
	"\x1E", "\\", "Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", NULL,
	NULL, "Alt", "[_]", "Cap",
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "Num", "Scl",
	"Hom", "\x18", "PgU", NULL, "\x1B", NULL, "\x1A", NULL, "End", "\x19", "PgD",
	"Ins", "Del",
};

// Same, but for text input keyboard in non-shifted state
static const char *g_sdlDOSScanCodeTextInputNonShiftedStrs[] = {
	NULL, "Esc",
	"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "\x11",
	NULL, "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "Ent",
	NULL, "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "`",
	"\x1E", "\\", "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", NULL,
	NULL, NULL, "[_]", NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, "\x18", NULL, NULL, "\x1B", NULL, "\x1A", "+", NULL, "\x19", NULL,
	NULL, "Del",
};

// Same but shifted
static const char *g_sdlDOSScanCodeTextInputShiftedStrs[] = {
	NULL, "Esc",
	"!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+", "\x11",
	NULL, "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "Ent",
	NULL, "A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "\"", "~",
	"\x1E", "|", "Z", "X", "C", "V", "B", "N", "M", "<", ">", "?", NULL,
	NULL, NULL, "[_]", NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, "\x18", NULL, NULL, "\x1B", NULL, "\x1A", "+", NULL, "\x19", NULL,
	NULL, "Del",
};

// Same, but for debug keys
static const char *g_sdlDOSScanCodeDebugKeysStrs[] = {
	NULL, "Esc",
	"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "\x11",
	"Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "Ent",
	"Ctl", "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "`",
	"L\x1E", "\\", "Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", "R\x1E",
	"*  ", // SPECIAL - Scancode is also used by Print Screen key
	"Alt", "[_]", "Cap",
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "Num", "Scl",
	"7  ", "8 \x18", "9  ", "-  ", "4 \x1B", "5  ", "6 \x1A", "+  ", "1  ", "2 \x19", "3  ",
	"0  ", ".  ",
	NULL, NULL, NULL,
	"F11", "F12",
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0x59-0x5F
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0x60-0x6F
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0x70-0x7F
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0x80-0x8F
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0x90-0x9F
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0xA0-0xAF
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0xB0-0xBF
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0xC0-0xCF
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 0xD0-0xDF
	NULL, // 0xE0
	"| |", // Pause key, 0xE1 (worth to add empty space above just for this?)
};

// Text input: One of the shifted and non-shifted variants
static const char **g_sdlDOSScanCodeTextInputStrs_Ptr;

// Text input keyboard layout definition (probably better we don't use "special" keys e.g., with scancode 0xE0 sent, even though there shouldn't be a difference)

static const BE_ST_ScanCode_T g_sdlDOSScanCodeTextInputLayout[ALTCONTROLLER_TEXTINPUT_KEYS_HEIGHT][ALTCONTROLLER_TEXTINPUT_KEYS_WIDTH] = {
	{BE_ST_SC_KP_4/*LEFT*/, BE_ST_SC_Q, BE_ST_SC_W, BE_ST_SC_E, BE_ST_SC_R, BE_ST_SC_T, BE_ST_SC_Y, BE_ST_SC_U, BE_ST_SC_I, BE_ST_SC_O, BE_ST_SC_P, BE_ST_SC_LBRACKET, BE_ST_SC_RBRACKET, BE_ST_SC_7, BE_ST_SC_8, BE_ST_SC_9, BE_ST_SC_0, BE_ST_SC_MINUS},

	{BE_ST_SC_KP_6/*RIGHT*/, BE_ST_SC_A, BE_ST_SC_S, BE_ST_SC_D, BE_ST_SC_F, BE_ST_SC_G, BE_ST_SC_H, BE_ST_SC_J, BE_ST_SC_K, BE_ST_SC_L, BE_ST_SC_SEMICOLON, BE_ST_SC_QUOTE, BE_ST_SC_BACKSLASH, BE_ST_SC_4, BE_ST_SC_5, BE_ST_SC_6, BE_ST_SC_SPACE, BE_ST_SC_EQUALS},

	{BE_ST_SC_LSHIFT, BE_ST_SC_Z, BE_ST_SC_X, BE_ST_SC_C, BE_ST_SC_V, BE_ST_SC_B, BE_ST_SC_N, BE_ST_SC_M, BE_ST_SC_COMMA, BE_ST_SC_PERIOD, BE_ST_SC_SLASH, BE_ST_SC_GRAVE, BE_ST_SC_ENTER, BE_ST_SC_1, BE_ST_SC_2, BE_ST_SC_3, BE_ST_SC_KP_PERIOD/*DEL*/, BE_ST_SC_BSPACE},
};

// Debug keys keyboard layout definition (again not using "special" keys, but the Pause key, for which 6 scancodes are sent on press only, is here)

static const BE_ST_ScanCode_T g_sdlDOSScanCodeDebugKeysLayout[ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT][ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH] = {
	{BE_ST_SC_ESC, BE_ST_SC_F1, BE_ST_SC_F2, BE_ST_SC_F3, BE_ST_SC_F4, BE_ST_SC_F5, BE_ST_SC_F6, BE_ST_SC_F7, BE_ST_SC_F8, BE_ST_SC_F9, BE_ST_SC_F10, BE_ST_SC_F11, BE_ST_SC_F12, BE_ST_SC_PAUSE, BE_ST_SC_NUMLOCK, BE_ST_SC_KP_MULTIPLY, BE_ST_SC_KP_MINUS},

	{BE_ST_SC_GRAVE, BE_ST_SC_1, BE_ST_SC_2, BE_ST_SC_3, BE_ST_SC_4, BE_ST_SC_5, BE_ST_SC_6, BE_ST_SC_7, BE_ST_SC_8, BE_ST_SC_9, BE_ST_SC_0, BE_ST_SC_MINUS, BE_ST_SC_EQUALS, BE_ST_SC_BSPACE, BE_ST_SC_KP_7, BE_ST_SC_KP_8, BE_ST_SC_KP_9},

	{BE_ST_SC_TAB, BE_ST_SC_Q, BE_ST_SC_W, BE_ST_SC_E, BE_ST_SC_R, BE_ST_SC_T, BE_ST_SC_Y, BE_ST_SC_U, BE_ST_SC_I, BE_ST_SC_O, BE_ST_SC_P, BE_ST_SC_LBRACKET, BE_ST_SC_RBRACKET, BE_ST_SC_BACKSLASH, BE_ST_SC_KP_4, BE_ST_SC_KP_5, BE_ST_SC_KP_6},

	{BE_ST_SC_CAPSLOCK, BE_ST_SC_A, BE_ST_SC_S, BE_ST_SC_D, BE_ST_SC_F, BE_ST_SC_G, BE_ST_SC_H, BE_ST_SC_J, BE_ST_SC_K, BE_ST_SC_L, BE_ST_SC_SEMICOLON, BE_ST_SC_QUOTE, BE_ST_SC_SPACE, BE_ST_SC_SCROLLLOCK, BE_ST_SC_KP_1, BE_ST_SC_KP_2, BE_ST_SC_KP_3},

	{BE_ST_SC_LSHIFT, BE_ST_SC_Z, BE_ST_SC_X, BE_ST_SC_C, BE_ST_SC_V, BE_ST_SC_B, BE_ST_SC_N, BE_ST_SC_M, BE_ST_SC_COMMA, BE_ST_SC_PERIOD, BE_ST_SC_SLASH, BE_ST_SC_RSHIFT, BE_ST_SC_LALT, BE_ST_SC_LCTRL, BE_ST_SC_KP_0, BE_ST_SC_KP_PERIOD, BE_ST_SC_KP_PLUS},
};


// Colors in BGRA format/order (on certain platforms)

static const uint32_t g_sdlCGAGfxBGRAScreenColors[] = {
	0xff000000/*black*/,
	0xff00ffff/*light cyan*/,
	0xffff00ff/*light magenta*/,
	0xffffffff/*white*/
};

// Same but for the EGA/VGA (and colored text modes on CGA/EGA/VGA)
// Note: Also used in launcher; extern is added for C++
#ifdef __cplusplus
extern
#endif
const uint32_t g_sdlEGABGRAScreenColors[] = {
	0xff000000/*black*/, 0xff0000aa/*blue*/, 0xff00aa00/*green*/, 0xff00aaaa/*cyan*/,
	0xffaa0000/*red*/, 0xffaa00aa/*magenta*/, 0xffaa5500/*brown*/, 0xffaaaaaa/*light gray*/,
	0xff555555/*gray*/, 0xff5555ff/*light blue*/, 0xff55ff55/*light green*/, 0xff55ffff/*light cyan*/,
	0xffff5555/*light red*/, 0xffff55ff/*light magenta*/, 0xffffff55/*yellow*/, 0xffffffff/*white*/
};


void BEL_ST_SetRelativeMouseMotion(bool enable);

/*static*/ void BEL_ST_ConditionallyShowAltInputPointer(void)
{
	BEL_ST_SetRelativeMouseMotion(!g_sdlShowControllerUI || !(g_sdlFaceButtonsAreShown || g_sdlDpadIsShown || g_sdlTextInputUIIsShown || g_sdlDebugKeysUIIsShown));
}


static void BEL_ST_CreatePadTextureIfNeeded(SDL_Texture **padTexturePtrPtr)
{
	if (*padTexturePtrPtr)
	{
		return;
	}
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	*padTexturePtrPtr = SDL_CreateTexture(g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, ALTCONTROLLER_PAD_PIX_WIDTH, ALTCONTROLLER_PAD_PIX_HEIGHT);
	if (!(*padTexturePtrPtr))
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 pad texture,\n%s\n", SDL_GetError());
		//Destroy window and renderer?
		exit(0);
	}
	SDL_SetTextureBlendMode(*padTexturePtrPtr, SDL_BLENDMODE_BLEND); // Yes there's some Alpha
}

static void BEL_ST_RedrawTextToBuffer(uint32_t *picPtr, int picWidth, const char *text)
{
	for (int currRow = 0, fontXpmIndex = ALTCONTROLLER_FONT_XPM_ROW_OFFSET; currRow < ALTCONTROLLER_CHAR_PIX_HEIGHT; ++currRow, picPtr += picWidth, ++fontXpmIndex)
	{
		const char *fontRowPtr = pad_font_mono_xpm[fontXpmIndex];
		uint32_t *currPtr = picPtr;
		for (const char *chPtr = text; *chPtr; ++chPtr)
		{
			const char *fontPixPtr = fontRowPtr + ALTCONTROLLER_CHAR_PIX_WIDTH * (*chPtr);
			for (int currCharCol = 0; currCharCol < ALTCONTROLLER_CHAR_PIX_WIDTH; ++currCharCol, ++currPtr, ++fontPixPtr)
			{
				if (*fontPixPtr == '+')
				{
					*currPtr = g_sdlCGAGfxBGRAScreenColors[0];
				}
			}
		}
	}
}

static void BEL_ST_PrepareToShowOnePad(const int *scanCodes, const char **padXpm, SDL_Texture **padTexturePtrPtr, bool *areButtonsShownPtr)
{
	BEL_ST_CreatePadTextureIfNeeded(padTexturePtrPtr);

	uint32_t pixels[ALTCONTROLLER_PAD_PIX_WIDTH*ALTCONTROLLER_PAD_PIX_HEIGHT];
	uint32_t *currPtr = pixels;
	for (int currRow = 0, xpmIndex = ALTCONTROLLER_PAD_XPM_ROW_OFFSET; currRow < ALTCONTROLLER_PAD_PIX_HEIGHT; ++currRow, ++xpmIndex)
	{
		const char *xpmRowPtr = padXpm[xpmIndex];
		for (int currCol = 0; currCol < ALTCONTROLLER_PAD_PIX_WIDTH; ++currCol, ++currPtr, ++xpmRowPtr)
		{
			switch (*xpmRowPtr)
			{
			case ' ':
				*currPtr = 0x00000000; // HACK (BGRA, working with any order) because we don't have it defined elsewhere
				break;
			case '.':
				*currPtr = g_sdlEGABGRAScreenColors[8]; // Gray
				break;
			case '+':
				*currPtr = g_sdlEGABGRAScreenColors[7]; // Light gray
				break;
			// HACK - Compress 4 XPM colors into one
			case '@':
			case '#':
			case '$':
			case '%':
				*currPtr = g_sdlEGABGRAScreenColors[15]; // White
				break;
			}
		}
	}
	for (int counter = 0; counter < 4; ++scanCodes, ++counter)
	{
		if (!(*scanCodes))
			continue;

		const char *str = g_sdlDOSScanCodePadStrs[*scanCodes];
		BEL_ST_RedrawTextToBuffer(pixels + g_sdlControllerFaceButtonsTextLocs[2*counter] + g_sdlControllerFaceButtonsTextLocs[2*counter+1]*ALTCONTROLLER_PAD_PIX_WIDTH + (3-strlen(str))*(ALTCONTROLLER_CHAR_PIX_WIDTH/2), ALTCONTROLLER_PAD_PIX_WIDTH, str);
	}
	// Add some alpha channel
	currPtr = pixels;
	for (int pixCounter = 0; pixCounter < ALTCONTROLLER_PAD_PIX_WIDTH*ALTCONTROLLER_PAD_PIX_HEIGHT; ++pixCounter, ++currPtr)
	{
		*currPtr &= 0xBFFFFFFF; // BGRA
	}
	SDL_UpdateTexture(*padTexturePtrPtr, NULL, pixels, 4*ALTCONTROLLER_PAD_PIX_WIDTH);
	*areButtonsShownPtr = true;

	g_sdlForceGfxControlUiRefresh = true;
}

/*static*/ void BEL_ST_PrepareToShowControllerUI(const BE_ST_ControllerMapping *mapping)
{
	const int faceButtonsScancodes[4] = {
		(mapping->buttons[BE_ST_CTRL_BUT_A].mapClass == BE_ST_CTRL_MAP_KEYSCANCODE) ? mapping->buttons[BE_ST_CTRL_BUT_A].val : '\0',
		(mapping->buttons[BE_ST_CTRL_BUT_B].mapClass == BE_ST_CTRL_MAP_KEYSCANCODE) ? mapping->buttons[BE_ST_CTRL_BUT_B].val : '\0',
		(mapping->buttons[BE_ST_CTRL_BUT_X].mapClass == BE_ST_CTRL_MAP_KEYSCANCODE) ? mapping->buttons[BE_ST_CTRL_BUT_X].val : '\0',
		(mapping->buttons[BE_ST_CTRL_BUT_Y].mapClass == BE_ST_CTRL_MAP_KEYSCANCODE) ? mapping->buttons[BE_ST_CTRL_BUT_Y].val : '\0'
	};
	const int dpadScancodes[4] = {
		(mapping->buttons[BE_ST_CTRL_BUT_DPAD_DOWN].mapClass == BE_ST_CTRL_MAP_KEYSCANCODE) ? mapping->buttons[BE_ST_CTRL_BUT_DPAD_DOWN].val : '\0',
		(mapping->buttons[BE_ST_CTRL_BUT_DPAD_RIGHT].mapClass == BE_ST_CTRL_MAP_KEYSCANCODE) ? mapping->buttons[BE_ST_CTRL_BUT_DPAD_RIGHT].val : '\0',
		(mapping->buttons[BE_ST_CTRL_BUT_DPAD_LEFT].mapClass == BE_ST_CTRL_MAP_KEYSCANCODE) ? mapping->buttons[BE_ST_CTRL_BUT_DPAD_LEFT].val : '\0',
		(mapping->buttons[BE_ST_CTRL_BUT_DPAD_UP].mapClass == BE_ST_CTRL_MAP_KEYSCANCODE) ? mapping->buttons[BE_ST_CTRL_BUT_DPAD_UP].val : '\0'
	};
	const int emptyScancodesArray[4] = {'\0'};

	memcpy(g_sdlFaceButtonsScanCodes, faceButtonsScancodes, sizeof(faceButtonsScancodes));
	if (memcmp(&faceButtonsScancodes, &emptyScancodesArray, sizeof(emptyScancodesArray)))
		BEL_ST_PrepareToShowOnePad(faceButtonsScancodes, pad_thumb_buttons_xpm, &g_sdlFaceButtonsTexture, &g_sdlFaceButtonsAreShown);

	memcpy(g_sdlDpadScanCodes, dpadScancodes, sizeof(dpadScancodes));
	if (memcmp(&dpadScancodes, &emptyScancodesArray, sizeof(emptyScancodesArray)))
		BEL_ST_PrepareToShowOnePad(dpadScancodes, pad_dpad_xpm, &g_sdlDpadTexture, &g_sdlDpadIsShown);

	g_sdlPointerSelectedPadButtonScanCode = 0;
	g_sdlControllerUIPointerPressed = false;

	BEL_ST_ConditionallyShowAltInputPointer();
}

static void BEL_ST_RedrawKeyToBuffer(uint32_t *picPtr, int picWidth, const char *text, bool isMarked, bool isPressed)
{
#if 0
	// This can happen for space that should be skipped
	if (!text)
		text = "";
#endif

	/*** Draw frame ***/
	uint32_t frameColor = g_sdlEGABGRAScreenColors[isMarked ? 15/*White*/ : 8/*Gray*/];
	uint32_t *currPtr = picPtr;
	// Frame top
	for (int currCol = 0; currCol < ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH; ++currCol, ++currPtr)
	{
		*currPtr = frameColor;
	}
	// Frame left + key + Frame right
	currPtr += picWidth-ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH;
	for (int currRow = 1; currRow < ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT-1; ++currRow)
	{
		// Frame left pixel
		*(currPtr++) = frameColor;
		// Line between frame sides
		for (int currCol = 1; currCol < ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH-1; ++currCol, ++currPtr)
		{
			*currPtr = g_sdlEGABGRAScreenColors[isPressed ? 12/*Light red*/ : 7/*Light gray*/];
		}
		// Frame right pixel
		*currPtr = frameColor;

		currPtr += picWidth-ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH+1;
	}
	// Frame bottom
	for (int currCol = 0; currCol < ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH; ++currCol, ++currPtr)
	{
		*currPtr = frameColor;
	}
	/*** Draw text ***/
	BEL_ST_RedrawTextToBuffer(picPtr + (ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH-ALTCONTROLLER_CHAR_PIX_WIDTH*strlen(text))/2 + picWidth*(ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT-ALTCONTROLLER_CHAR_PIX_HEIGHT)/2, picWidth, text);
	// Add some alpha channel (shouldn't be a lot)
	currPtr = picPtr;
	for (int currRow = 0; currRow < ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT; ++currRow, currPtr += picWidth-ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH)
	{
		for (int currCol = 0; currCol < ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH; ++currCol, ++currPtr)
		{
		*currPtr &= 0xDFFFFFFF; // BGRA
		}
	}
}

static void BEL_ST_CreateTextInputTextureIfNeeded(void)
{
	if (g_sdlTextInputTexture)
	{
		return;
	}
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	g_sdlTextInputTexture = SDL_CreateTexture(g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, ALTCONTROLLER_TEXTINPUT_PIX_WIDTH, ALTCONTROLLER_TEXTINPUT_PIX_HEIGHT);
	if (!g_sdlTextInputTexture)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 text input texture,\n%s\n", SDL_GetError());
		//Destroy window and renderer?
		exit(0);
	}
	SDL_SetTextureBlendMode(g_sdlTextInputTexture, SDL_BLENDMODE_BLEND); // Yes there's some Alpha
}

static void BEL_ST_CreateDebugKeysTextureIfNeeded(void)
{
	if (g_sdlDebugKeysTexture)
	{
		return;
	}
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	g_sdlDebugKeysTexture = SDL_CreateTexture(g_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, ALTCONTROLLER_DEBUGKEYS_PIX_WIDTH, ALTCONTROLLER_DEBUGKEYS_PIX_HEIGHT);
	if (!g_sdlDebugKeysTexture)
	{
		BE_Cross_LogMessage(BE_LOG_MSG_ERROR, "Failed to (re)create SDL2 debug keys texture,\n%s\n", SDL_GetError());
		//Destroy window and renderer?
		exit(0);
	}
	SDL_SetTextureBlendMode(g_sdlDebugKeysTexture, SDL_BLENDMODE_BLEND); // Yes there's some Alpha
}

static void BEL_ST_RedrawWholeTextInputUI(void)
{
	uint32_t pixels[ALTCONTROLLER_TEXTINPUT_PIX_WIDTH*ALTCONTROLLER_TEXTINPUT_PIX_HEIGHT];
	uint32_t *currPtr = pixels;

	for (int currKeyRow = 0; currKeyRow < ALTCONTROLLER_TEXTINPUT_KEYS_HEIGHT; ++currKeyRow, currPtr += ALTCONTROLLER_TEXTINPUT_PIX_WIDTH*(ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT-1))
	{
		for (int currKeyCol = 0; currKeyCol < ALTCONTROLLER_TEXTINPUT_KEYS_WIDTH; ++currKeyCol, currPtr += ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH)
		{
			BEL_ST_RedrawKeyToBuffer(currPtr, ALTCONTROLLER_TEXTINPUT_PIX_WIDTH, g_sdlDOSScanCodeTextInputStrs_Ptr[(int)g_sdlDOSScanCodeTextInputLayout[currKeyRow][currKeyCol]], false, false);
		}
	}
	// Simpler to do so outside the loop
	BEL_ST_RedrawKeyToBuffer(pixels + (ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH*g_sdlKeyboardUISelectedKeyX) + ALTCONTROLLER_TEXTINPUT_PIX_WIDTH*(ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT*g_sdlKeyboardUISelectedKeyY), ALTCONTROLLER_TEXTINPUT_PIX_WIDTH, g_sdlDOSScanCodeTextInputStrs_Ptr[(int)g_sdlDOSScanCodeTextInputLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]], true, g_sdlTextInputIsKeyPressed);

	SDL_UpdateTexture(g_sdlTextInputTexture, NULL, pixels, 4*ALTCONTROLLER_TEXTINPUT_PIX_WIDTH);
}

static void BEL_ST_RedrawWholeDebugKeysUI(void)
{
	uint32_t pixels[ALTCONTROLLER_DEBUGKEYS_PIX_WIDTH*ALTCONTROLLER_DEBUGKEYS_PIX_HEIGHT];
	uint32_t *currPtr = pixels;

	for (int currKeyRow = 0; currKeyRow < ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT; ++currKeyRow, currPtr += ALTCONTROLLER_DEBUGKEYS_PIX_WIDTH*(ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT-1))
	{
		for (int currKeyCol = 0; currKeyCol < ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH; ++currKeyCol, currPtr += ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH)
		{
			BEL_ST_RedrawKeyToBuffer(currPtr, ALTCONTROLLER_DEBUGKEYS_PIX_WIDTH, g_sdlDOSScanCodeDebugKeysStrs[(int)g_sdlDOSScanCodeDebugKeysLayout[currKeyRow][currKeyCol]], false, g_sdlDebugKeysPressed[currKeyRow][currKeyCol]);
		}
	}
	// Simpler to do so outside the loop
	BEL_ST_RedrawKeyToBuffer(pixels + (ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH*g_sdlKeyboardUISelectedKeyX) + ALTCONTROLLER_DEBUGKEYS_PIX_WIDTH*(ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT*g_sdlKeyboardUISelectedKeyY), ALTCONTROLLER_DEBUGKEYS_PIX_WIDTH, g_sdlDOSScanCodeDebugKeysStrs[(int)g_sdlDOSScanCodeDebugKeysLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]], true, g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]);

	SDL_UpdateTexture(g_sdlDebugKeysTexture, NULL, pixels, 4*ALTCONTROLLER_DEBUGKEYS_PIX_WIDTH);
}


/*static*/ void BEL_ST_PrepareToShowTextInputUI(void)
{
	BEL_ST_CreateTextInputTextureIfNeeded();

	g_sdlKeyboardUISelectedKeyX = 0;
	g_sdlKeyboardUISelectedKeyY = 0;
	g_sdlKeyboardUIPointerUsed = false;
	g_sdlTextInputIsKeyPressed = false;
	g_sdlTextInputIsShifted = false;
	g_sdlDOSScanCodeTextInputStrs_Ptr = g_sdlDOSScanCodeTextInputNonShiftedStrs;

	BEL_ST_RedrawWholeTextInputUI();
	g_sdlTextInputUIIsShown = true;

	g_sdlForceGfxControlUiRefresh = true;

	BEL_ST_ConditionallyShowAltInputPointer();
}

/*static*/ void BEL_ST_PrepareToShowDebugKeysUI(void)
{
	BEL_ST_CreateDebugKeysTextureIfNeeded();

	g_sdlKeyboardUISelectedKeyX = 0;
	g_sdlKeyboardUISelectedKeyY = 0;
	g_sdlKeyboardUIPointerUsed = false;

	for (int currKeyRow = 0; currKeyRow < ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT; ++currKeyRow)
		for (int currKeyCol = 0; currKeyCol < ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH; ++currKeyCol)
			g_sdlDebugKeysPressed[currKeyRow][currKeyCol] = false;

	BEL_ST_RedrawWholeDebugKeysUI();
	g_sdlDebugKeysUIIsShown = true;

	g_sdlForceGfxControlUiRefresh = true;

	BEL_ST_ConditionallyShowAltInputPointer();
}

static void BEL_ST_ToggleTextInputUIKey(int x, int y, bool isMarked, bool isPressed)
{
	uint32_t pixels[ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT];

	BEL_ST_RedrawKeyToBuffer(pixels, ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, g_sdlDOSScanCodeTextInputStrs_Ptr[(int)g_sdlDOSScanCodeTextInputLayout[y][x]], isMarked, isPressed);

	SDL_Rect outRect = {x*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, y*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT, ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT};

	SDL_UpdateTexture(g_sdlTextInputTexture, &outRect, pixels, 4*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH);
}

static void BEL_ST_ToggleDebugKeysKey(int x, int y, bool isMarked, bool isPressed)
{
	uint32_t pixels[ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT];

	BEL_ST_RedrawKeyToBuffer(pixels, ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, g_sdlDOSScanCodeDebugKeysStrs[(int)g_sdlDOSScanCodeDebugKeysLayout[y][x]], isMarked, isPressed);

	SDL_Rect outRect = {x*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, y*ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT, ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH, ALTCONTROLLER_KEYBOARD_KEY_PIXHEIGHT};

	SDL_UpdateTexture(g_sdlDebugKeysTexture, &outRect, pixels, 4*ALTCONTROLLER_KEYBOARD_KEY_PIXWIDTH);
}

int BEL_ST_MoveUpInTextInputUI(void)
{
	int origScanCode = g_sdlTextInputIsKeyPressed ? (int)g_sdlDOSScanCodeTextInputLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX] : 0;
	BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, false);
	g_sdlTextInputIsKeyPressed = false;

	--g_sdlKeyboardUISelectedKeyY;
	if (g_sdlKeyboardUISelectedKeyY < 0)
	{
		g_sdlKeyboardUISelectedKeyY = ALTCONTROLLER_TEXTINPUT_KEYS_HEIGHT-1;
	}
	BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, false);

	g_sdlForceGfxControlUiRefresh = true;
	return origScanCode;
}

int BEL_ST_MoveDownInTextInputUI(void)
{
	int origScanCode = g_sdlTextInputIsKeyPressed ? (int)g_sdlDOSScanCodeTextInputLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX] : 0;
	BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, false);
	g_sdlTextInputIsKeyPressed = false;

	++g_sdlKeyboardUISelectedKeyY;
	if (g_sdlKeyboardUISelectedKeyY >= ALTCONTROLLER_TEXTINPUT_KEYS_HEIGHT)
	{
		g_sdlKeyboardUISelectedKeyY = 0;
	}
	BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, false);

	g_sdlForceGfxControlUiRefresh = true;
	return origScanCode;
}

int BEL_ST_MoveLeftInTextInputUI(void)
{
	int origScanCode = g_sdlTextInputIsKeyPressed ? (int)g_sdlDOSScanCodeTextInputLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX] : 0;
	BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, false);
	g_sdlTextInputIsKeyPressed = false;

	--g_sdlKeyboardUISelectedKeyX;
	if (g_sdlKeyboardUISelectedKeyX < 0)
	{
		g_sdlKeyboardUISelectedKeyX = ALTCONTROLLER_TEXTINPUT_KEYS_WIDTH-1;
	}
	BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, false);

	g_sdlForceGfxControlUiRefresh = true;
	return origScanCode;
}

int BEL_ST_MoveRightInTextInputUI(void)
{
	int origScanCode = g_sdlTextInputIsKeyPressed ? (int)g_sdlDOSScanCodeTextInputLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX] : 0;
	BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, false);
	g_sdlTextInputIsKeyPressed = false;

	++g_sdlKeyboardUISelectedKeyX;
	if (g_sdlKeyboardUISelectedKeyX >= ALTCONTROLLER_TEXTINPUT_KEYS_WIDTH)
	{
		g_sdlKeyboardUISelectedKeyX = 0;
	}
	BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, false);

	g_sdlForceGfxControlUiRefresh = true;
	return origScanCode;
}

void BEL_ST_MoveUpInDebugKeysUI(void)
{
	BEL_ST_ToggleDebugKeysKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]);

	--g_sdlKeyboardUISelectedKeyY;
	if (g_sdlKeyboardUISelectedKeyY < 0)
	{
		g_sdlKeyboardUISelectedKeyY = ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT-1;
	}
	BEL_ST_ToggleDebugKeysKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]);

	g_sdlForceGfxControlUiRefresh = true;
}

void BEL_ST_MoveDownInDebugKeysUI(void)
{
	BEL_ST_ToggleDebugKeysKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]);

	++g_sdlKeyboardUISelectedKeyY;
	if (g_sdlKeyboardUISelectedKeyY >= ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT)
	{
		g_sdlKeyboardUISelectedKeyY = 0;
	}
	BEL_ST_ToggleDebugKeysKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]);

	g_sdlForceGfxControlUiRefresh = true;
}

void BEL_ST_MoveLeftInDebugKeysUI(void)
{
	BEL_ST_ToggleDebugKeysKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]);

	--g_sdlKeyboardUISelectedKeyX;
	if (g_sdlKeyboardUISelectedKeyX < 0)
	{
		g_sdlKeyboardUISelectedKeyX = ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH-1;
	}
	BEL_ST_ToggleDebugKeysKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]);

	g_sdlForceGfxControlUiRefresh = true;
}

void BEL_ST_MoveRightInDebugKeysUI(void)
{
	BEL_ST_ToggleDebugKeysKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]);

	++g_sdlKeyboardUISelectedKeyX;
	if (g_sdlKeyboardUISelectedKeyX >= ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH)
	{
		g_sdlKeyboardUISelectedKeyX = 0;
	}
	BEL_ST_ToggleDebugKeysKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]);

	g_sdlForceGfxControlUiRefresh = true;
}


int BEL_ST_ToggleShiftStateInTextInputUI(bool *pToggle)
{
	if (!(*pToggle))
		return 0;
	g_sdlTextInputIsShifted = !g_sdlTextInputIsShifted;
	*pToggle = g_sdlTextInputIsShifted;
	g_sdlDOSScanCodeTextInputStrs_Ptr = g_sdlTextInputIsShifted ? g_sdlDOSScanCodeTextInputShiftedStrs : g_sdlDOSScanCodeTextInputNonShiftedStrs;

	BEL_ST_RedrawWholeTextInputUI();

	g_sdlForceGfxControlUiRefresh = true;

	return BE_ST_SC_LSHIFT;
}

int BEL_ST_ToggleKeyPressInTextInputUI(bool *pToggle)
{
	if (g_sdlTextInputIsKeyPressed == *pToggle)
		return 0;
	g_sdlTextInputIsKeyPressed = *pToggle;

	BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, *pToggle);

	g_sdlForceGfxControlUiRefresh = true;

	// Shift key is a special case
	if ((g_sdlDOSScanCodeTextInputLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX] == BE_ST_SC_LSHIFT))
		return BEL_ST_ToggleShiftStateInTextInputUI(pToggle);

	return (int)g_sdlDOSScanCodeTextInputLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX];
}

int BEL_ST_ToggleKeyPressInDebugKeysUI(bool *pToggle)
{
	g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX] ^= true;
	*pToggle = g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX];

	BEL_ST_ToggleDebugKeysKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, *pToggle);

	g_sdlForceGfxControlUiRefresh = true;

	return (int)g_sdlDOSScanCodeDebugKeysLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX];
}


void BEL_ST_CheckMovedPointerInTextInputUI(int x, int y)
{
	if (!g_sdlKeyboardUIPointerUsed)
		return;

	if ((x < g_sdlControllerTextInputRect.x) || (x >= g_sdlControllerTextInputRect.x+g_sdlControllerTextInputRect.w)
	    || (y < g_sdlControllerTextInputRect.y) || (y >= g_sdlControllerTextInputRect.y+g_sdlControllerTextInputRect.h))
		return;

	BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, false);
	g_sdlTextInputIsKeyPressed = false;

	// Normalize coordinates to keys
	g_sdlKeyboardUISelectedKeyX = (x-g_sdlControllerTextInputRect.x)*ALTCONTROLLER_TEXTINPUT_KEYS_WIDTH/g_sdlControllerTextInputRect.w;
	g_sdlKeyboardUISelectedKeyY = (y-g_sdlControllerTextInputRect.y)*ALTCONTROLLER_TEXTINPUT_KEYS_HEIGHT/g_sdlControllerTextInputRect.h;

	BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, false);

	g_sdlForceGfxControlUiRefresh = true;
}

void BEL_ST_CheckPressedPointerInTextInputUI(int x, int y)
{
	g_sdlKeyboardUIPointerUsed = true;
	BEL_ST_CheckMovedPointerInTextInputUI(x, y);
}

int BEL_ST_CheckReleasedPointerInTextInputUI(int x, int y)
{
	if (!g_sdlKeyboardUIPointerUsed)
		return 0;

	g_sdlKeyboardUIPointerUsed = false;
	if ((x < g_sdlControllerTextInputRect.x) || (x >= g_sdlControllerTextInputRect.x+g_sdlControllerTextInputRect.w)
	    || (y < g_sdlControllerTextInputRect.y) || (y >= g_sdlControllerTextInputRect.y+g_sdlControllerTextInputRect.h))
		return 0;

	//BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, false);

	// Normalize coordinates to keys
	g_sdlKeyboardUISelectedKeyX = (x-g_sdlControllerTextInputRect.x)*ALTCONTROLLER_TEXTINPUT_KEYS_WIDTH/g_sdlControllerTextInputRect.w;
	g_sdlKeyboardUISelectedKeyY = (y-g_sdlControllerTextInputRect.y)*ALTCONTROLLER_TEXTINPUT_KEYS_HEIGHT/g_sdlControllerTextInputRect.h;
	// Hack for covering the special case of the shift key
	g_sdlTextInputIsKeyPressed = false;
	bool toggle = true;
	int result = BEL_ST_ToggleKeyPressInTextInputUI(&toggle);
	toggle = false;
	BEL_ST_ToggleKeyPressInTextInputUI(&toggle);

	return result;
}


void BEL_ST_CheckMovedPointerInDebugKeysUI(int x, int y)
{
	if (!g_sdlKeyboardUIPointerUsed)
		return;

	if ((x < g_sdlControllerDebugKeysRect.x) || (x >= g_sdlControllerDebugKeysRect.x+g_sdlControllerDebugKeysRect.w)
	    || (y < g_sdlControllerDebugKeysRect.y) || (y >= g_sdlControllerDebugKeysRect.y+g_sdlControllerDebugKeysRect.h))
		return;

	BEL_ST_ToggleDebugKeysKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]);

	// Normalize coordinates to keys
	g_sdlKeyboardUISelectedKeyX = (x-g_sdlControllerDebugKeysRect.x)*ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH/g_sdlControllerDebugKeysRect.w;
	g_sdlKeyboardUISelectedKeyY = (y-g_sdlControllerDebugKeysRect.y)*ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT/g_sdlControllerDebugKeysRect.h;

	BEL_ST_ToggleDebugKeysKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]);

	g_sdlForceGfxControlUiRefresh = true;
}

void BEL_ST_CheckPressedPointerInDebugKeysUI(int x, int y)
{
	g_sdlKeyboardUIPointerUsed = true;
	BEL_ST_CheckMovedPointerInDebugKeysUI(x, y);
}

int BEL_ST_CheckReleasedPointerInDebugKeysUI(int x, int y, bool *pToggle)
{
	if (!g_sdlKeyboardUIPointerUsed)
		return 0;

	g_sdlKeyboardUIPointerUsed = false;
	if ((x < g_sdlControllerDebugKeysRect.x) || (x >= g_sdlControllerDebugKeysRect.x+g_sdlControllerDebugKeysRect.w)
	    || (y < g_sdlControllerDebugKeysRect.y) || (y >= g_sdlControllerDebugKeysRect.y+g_sdlControllerDebugKeysRect.h))
		return 0;

	//BEL_ST_ToggleDebugKeysKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, g_sdlDebugKeysPressed[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX]);

	// Normalize coordinates to keys
	g_sdlKeyboardUISelectedKeyX = (x-g_sdlControllerDebugKeysRect.x)*ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH/g_sdlControllerDebugKeysRect.w;
	g_sdlKeyboardUISelectedKeyY = (y-g_sdlControllerDebugKeysRect.y)*ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT/g_sdlControllerDebugKeysRect.h;

	return BEL_ST_ToggleKeyPressInDebugKeysUI(pToggle);
}

// Returns matching scanCode if found (possibly 0 if not set), -1 if not
// pointed at button/dpad (also checking for transparent pixels, or 0 otherwise
static int BEL_ST_GetControllerUIScanCodeFromPointer(int x, int y)
{
	if ((x >= g_sdlControllerFaceButtonsRect.x) && (x < g_sdlControllerFaceButtonsRect.x+g_sdlControllerFaceButtonsRect.w)
	    && (y >= g_sdlControllerFaceButtonsRect.y) && (y < g_sdlControllerFaceButtonsRect.y+g_sdlControllerFaceButtonsRect.h))
	{
		// Normalize coordinates to pad
		x = (x-g_sdlControllerFaceButtonsRect.x)*ALTCONTROLLER_PAD_PIX_WIDTH/g_sdlControllerFaceButtonsRect.w;
		y = (y-g_sdlControllerFaceButtonsRect.y)*ALTCONTROLLER_PAD_PIX_HEIGHT/g_sdlControllerFaceButtonsRect.h;
		switch (pad_thumb_buttons_xpm[y+ALTCONTROLLER_PAD_XPM_ROW_OFFSET][x])
		{
			case '%':
				return g_sdlFaceButtonsScanCodes[0];
			case '$':
				return g_sdlFaceButtonsScanCodes[1];
			case '#':
				return g_sdlFaceButtonsScanCodes[2];
			case '@':
				return g_sdlFaceButtonsScanCodes[3];
			case ' ':
				return -1; // Totally transparent
			default:
				return 0;
		}
	}
	else if ((x >= g_sdlControllerDpadRect.x) && (x < g_sdlControllerDpadRect.x+g_sdlControllerDpadRect.w)
	         && (y >= g_sdlControllerDpadRect.y) && (y < g_sdlControllerDpadRect.y+g_sdlControllerDpadRect.h))
	{
		// Normalize coordinates to pad
		x = (x-g_sdlControllerDpadRect.x)*ALTCONTROLLER_PAD_PIX_WIDTH/g_sdlControllerDpadRect.w;
		y = (y-g_sdlControllerDpadRect.y)*ALTCONTROLLER_PAD_PIX_HEIGHT/g_sdlControllerDpadRect.h;
		switch (pad_dpad_xpm[y+ALTCONTROLLER_PAD_XPM_ROW_OFFSET][x])
		{
			case '%':
				return g_sdlDpadScanCodes[0];
			case '$':
				return g_sdlDpadScanCodes[1];
			case '#':
				return g_sdlDpadScanCodes[2];
			case '@':
				return g_sdlDpadScanCodes[3];
			case ' ':
				return -1; // Totally transparent
			default:
				return 0;
		}
	}
	return -1;
}

extern const BE_ST_ControllerMapping *g_sdlControllerMappingActualCurr;
void BEL_ST_ReplaceControllerMapping(const BE_ST_ControllerMapping *mapping);

void BEL_ST_CheckPressedPointerInControllerUI(int x, int y)
{
	if (g_sdlControllerUIPointerPressed)
		return;
	g_sdlControllerUIPointerPressed = true;

	g_sdlPointerSelectedPadButtonScanCode = BEL_ST_GetControllerUIScanCodeFromPointer(x,y);
	if (g_sdlPointerSelectedPadButtonScanCode > 0)
	{
		emulatedDOSKeyEvent dosKeyEvent;
		dosKeyEvent.isSpecial = false;
		dosKeyEvent.dosScanCode = g_sdlPointerSelectedPadButtonScanCode;
		BEL_ST_HandleEmuKeyboardEvent(true, false, dosKeyEvent);
	}
	else if (g_sdlPointerSelectedPadButtonScanCode < 0)
	{
		g_sdlPointerSelectedPadButtonScanCode = 0;
		if (g_sdlControllerMappingActualCurr->prevMapping)
			BEL_ST_ReplaceControllerMapping(g_sdlControllerMappingActualCurr->prevMapping);
	}

}

void BEL_ST_CheckReleasedPointerInControllerUI(void)
{
	if (!g_sdlControllerUIPointerPressed)
		return;
	g_sdlControllerUIPointerPressed = false;

	if (g_sdlPointerSelectedPadButtonScanCode)
	{
		emulatedDOSKeyEvent dosKeyEvent;
		dosKeyEvent.isSpecial = false;
		dosKeyEvent.dosScanCode = g_sdlPointerSelectedPadButtonScanCode;
		BEL_ST_HandleEmuKeyboardEvent(false, false, dosKeyEvent);
	}
	g_sdlPointerSelectedPadButtonScanCode = 0;
}

void BEL_ST_CheckMovedPointerInControllerUI(int x, int y)
{
	if (!g_sdlControllerUIPointerPressed)
		return;

	int oldScanCode = g_sdlPointerSelectedPadButtonScanCode;
	g_sdlPointerSelectedPadButtonScanCode = BEL_ST_GetControllerUIScanCodeFromPointer(x,y);

	if (g_sdlPointerSelectedPadButtonScanCode < 0)
		g_sdlPointerSelectedPadButtonScanCode = 0;

	if (oldScanCode != g_sdlPointerSelectedPadButtonScanCode)
	{
		emulatedDOSKeyEvent dosKeyEvent;
		dosKeyEvent.isSpecial = false;
		if (oldScanCode)
		{
			dosKeyEvent.dosScanCode = oldScanCode;
			BEL_ST_HandleEmuKeyboardEvent(false, false, dosKeyEvent);
		}
		if (g_sdlPointerSelectedPadButtonScanCode)
		{
			dosKeyEvent.dosScanCode = g_sdlPointerSelectedPadButtonScanCode;
			BEL_ST_HandleEmuKeyboardEvent(true, false, dosKeyEvent);
		}
	}
}



void BEL_ST_ReleasePressedKeysInTextInputUI(void)
{
	if (g_sdlTextInputIsKeyPressed)
	{
		emulatedDOSKeyEvent dosKeyEvent;
		dosKeyEvent.isSpecial = false;
		dosKeyEvent.dosScanCode = g_sdlDOSScanCodeTextInputLayout[g_sdlKeyboardUISelectedKeyY][g_sdlKeyboardUISelectedKeyX];
		// Don't forget to "release" a key pressed in the text input UI
		BEL_ST_HandleEmuKeyboardEvent(false, false, dosKeyEvent);

		BEL_ST_ToggleTextInputUIKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, false, false);
		g_sdlTextInputIsKeyPressed = false;
		// Shift key may further be held, don't forget this too!
		if ((dosKeyEvent.dosScanCode != BE_ST_SC_LSHIFT) && g_sdlTextInputIsShifted)
		{
			dosKeyEvent.dosScanCode = BE_ST_SC_LSHIFT;
			BEL_ST_HandleEmuKeyboardEvent(false, false, dosKeyEvent);

			g_sdlTextInputIsShifted = false;
			BEL_ST_RedrawWholeTextInputUI();

		}
		g_sdlForceGfxControlUiRefresh = true;
	}
}

void BEL_ST_ReleasePressedKeysInDebugKeysUI(void)
{
	emulatedDOSKeyEvent dosKeyEvent;
	dosKeyEvent.isSpecial = false;
	for (int currKeyRow = 0; currKeyRow < ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT; ++currKeyRow)
		for (int currKeyCol = 0; currKeyCol < ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH; ++currKeyCol)
			if (g_sdlDebugKeysPressed[currKeyRow][currKeyCol])
			{
				dosKeyEvent.dosScanCode = g_sdlDOSScanCodeDebugKeysLayout[currKeyRow][currKeyCol];
				// Don't forget to "release" a key pressed in the text input UI
				BEL_ST_HandleEmuKeyboardEvent(false, false, dosKeyEvent);

				g_sdlDebugKeysPressed[currKeyRow][currKeyCol] = false;
				BEL_ST_ToggleDebugKeysKey(currKeyCol, currKeyRow, false, false);
			}

	BEL_ST_ToggleDebugKeysKey(g_sdlKeyboardUISelectedKeyX, g_sdlKeyboardUISelectedKeyY, true, false);
	g_sdlForceGfxControlUiRefresh = true;
}

void BEL_ST_ReleasePressedKeysInControllerUI(void)
{
	if (g_sdlControllerUIPointerPressed)
		BEL_ST_CheckReleasedPointerInControllerUI();
}

/*static*/ void BEL_ST_HideAltInputUI(void)
{
	g_sdlFaceButtonsAreShown = false;
	g_sdlDpadIsShown = false;
	g_sdlTextInputUIIsShown = false;
	g_sdlDebugKeysUIIsShown = false;

	g_sdlForceGfxControlUiRefresh = true;

	BEL_ST_ConditionallyShowAltInputPointer();
}


void BEL_ST_SetGfxOutputRects(bool allowResize)
{
	int srcWidth = g_sdlTexWidth;
	int srcHeight = g_sdlTexHeight;
	if (g_sdlScreenMode != 3)
	{
		// On the VGA, line doubling is in effect for graphical 200 lines modes
		if (g_sdlScreenMode != 0xe) // Is source 320x200 (not 640x200)?
			srcWidth *= 2;
		srcHeight *= 2;
	}
	int srcBorderLeft, srcBorderRight, srcBorderTop, srcBorderBottom;
	if (g_sdlScreenMode == 3)
	{
		srcBorderLeft = ENGINE_VGA_TXT_OVERSCAN_LEFT;
		srcBorderRight = ENGINE_VGA_TXT_OVERSCAN_RIGHT;
		srcBorderTop = ENGINE_VGA_TXT_OVERSCAN_TOP;
		srcBorderBottom = ENGINE_VGA_TXT_OVERSCAN_BOTTOM;
	}
	else
	{
		srcBorderLeft = ENGINE_VGA_GFX_OVERSCAN_LEFT_AFTER_DOUBLING;
		srcBorderRight = ENGINE_VGA_GFX_OVERSCAN_RIGHT_AFTER_DOUBLING;
		srcBorderTop = ENGINE_VGA_GFX_OVERSCAN_TOP_AFTER_DOUBLING;
		srcBorderBottom = ENGINE_VGA_GFX_OVERSCAN_BOTTOM_AFTER_DOUBLING;
	}
	int srcBorderedWidth = srcBorderLeft+srcWidth+srcBorderRight;
	int srcBorderedHeight = srcBorderTop+srcHeight+srcBorderBottom;
	int winWidth, winHeight;
	SDL_GetWindowSize(g_sdlWindow, &winWidth, &winHeight);

	// Special case - We may resize window based on mode, but only if allowResize == true (to prevent any possible infinite resizes loop)
	if (allowResize && g_sdlIsSoftwareRendered && !g_refKeenCfg.forceFullSoftScaling && (!(SDL_GetWindowFlags(g_sdlWindow) & SDL_WINDOW_FULLSCREEN)))
	{
		if ((g_refKeenCfg.scaleFactor*srcBorderedWidth != winWidth) || (g_refKeenCfg.scaleFactor*srcBorderedHeight != winHeight))
		{
			SDL_SetWindowSize(g_sdlWindow, g_refKeenCfg.scaleFactor*srcBorderedWidth, g_refKeenCfg.scaleFactor*srcBorderedHeight);
			// Not sure this will help, but still trying...
			SDL_GetWindowSize(g_sdlWindow, &winWidth, &winHeight);
		}
	}

	// Save modified window size
	if (!(SDL_GetWindowFlags(g_sdlWindow) & SDL_WINDOW_FULLSCREEN))
	{
		g_refKeenCfg.winWidth = winWidth;
		g_refKeenCfg.winHeight = winHeight;
	}

	if (g_sdlIsSoftwareRendered && !g_refKeenCfg.forceFullSoftScaling)
	{
		if (g_refKeenCfg.scaleFactor*srcBorderedWidth >= winWidth)
		{
			g_sdlAspectCorrectionBorderedRect.w = winWidth;
			g_sdlAspectCorrectionBorderedRect.x = 0;
		}
		else
		{
			g_sdlAspectCorrectionBorderedRect.w = g_refKeenCfg.scaleFactor*srcBorderedWidth;
			g_sdlAspectCorrectionBorderedRect.x = (winWidth-g_sdlAspectCorrectionBorderedRect.w)/2;
		}

		if (g_refKeenCfg.scaleFactor*srcBorderedHeight >= winHeight)
		{
			g_sdlAspectCorrectionBorderedRect.h = winHeight;
			g_sdlAspectCorrectionBorderedRect.y = 0;
		}
		else
		{
			g_sdlAspectCorrectionBorderedRect.h = g_refKeenCfg.scaleFactor*srcBorderedHeight;
			g_sdlAspectCorrectionBorderedRect.y = (winHeight-g_sdlAspectCorrectionBorderedRect.h)/2;
		}
	}
	else if (g_refKeenCfg.scaleType == SCALE_FILL)
	{
		g_sdlAspectCorrectionBorderedRect.w = winWidth;
		g_sdlAspectCorrectionBorderedRect.h = winHeight;
		g_sdlAspectCorrectionBorderedRect.x = 0;
		g_sdlAspectCorrectionBorderedRect.y = 0;
	}
	else
	{
		// Aspect correct - Includes overscan borders, should multiply
		// the height by 1.2 first (or the width by 5 and height 6),
		// if in 320x200 gfx mode, cause 1.2 == (320*3)/(4*200).
		// More generally, (srcwidth*3)/(4*srcheight).
		// So multiply width by 4*srcheight and height by 3*srcwidth;
		int scaledSrcBorderedWidth = (4*srcHeight)*srcBorderedWidth, scaledSrcBorderedHeight = (3*srcWidth)*srcBorderedHeight;
		if (scaledSrcBorderedHeight*winWidth < scaledSrcBorderedWidth*winHeight) // Thinner than scaledSrcBorderedWidth:scaledSrcBorderedHeight
		{
			g_sdlAspectCorrectionBorderedRect.w = winWidth;
			g_sdlAspectCorrectionBorderedRect.h = winWidth*scaledSrcBorderedHeight/scaledSrcBorderedWidth;
			g_sdlAspectCorrectionBorderedRect.x = 0;
			g_sdlAspectCorrectionBorderedRect.y = (winHeight-g_sdlAspectCorrectionBorderedRect.h)/2;
		}
		else // As wide as scaledSrcBorderedWidth:scaledSrcBorderedHeight at the least
		{
			g_sdlAspectCorrectionBorderedRect.w = winHeight*scaledSrcBorderedWidth/scaledSrcBorderedHeight;
			g_sdlAspectCorrectionBorderedRect.h = winHeight;
			g_sdlAspectCorrectionBorderedRect.x = (winWidth-g_sdlAspectCorrectionBorderedRect.w)/2;
			g_sdlAspectCorrectionBorderedRect.y = 0;
		}
	}
	// Finish with internal (non-bordered) rect
	g_sdlAspectCorrectionRect.x = g_sdlAspectCorrectionBorderedRect.x + g_sdlAspectCorrectionBorderedRect.w*srcBorderLeft/srcBorderedWidth;
	g_sdlAspectCorrectionRect.y = g_sdlAspectCorrectionBorderedRect.y + g_sdlAspectCorrectionBorderedRect.h*srcBorderTop/srcBorderedHeight;
	g_sdlAspectCorrectionRect.w = g_sdlAspectCorrectionBorderedRect.w*srcWidth/srcBorderedWidth;
	g_sdlAspectCorrectionRect.h = g_sdlAspectCorrectionBorderedRect.h*srcHeight/srcBorderedHeight;
	// As a little addition have this too - but:
	// 1. Use same dimensions independently of scaling.
	// 2. The dimensions of the controller UI are picked relatively to the host window's internal contents (without borders), not directly related to the client window size.
	// 3. Also taking the whole window into account (this doesn't depend on "screen mode", borders and more).
	int offset;
	int minWinDim = (winWidth >= winHeight) ? winHeight : winWidth;
	g_sdlControllerFaceButtonsRect.w = g_sdlControllerFaceButtonsRect.h = minWinDim/ALTCONTROLLER_FACEBUTTONS_SCREEN_DIM_RATIO;
	offset = minWinDim/(16*ALTCONTROLLER_FACEBUTTONS_SCREEN_DIM_RATIO);
	g_sdlControllerFaceButtonsRect.x = winWidth-g_sdlControllerFaceButtonsRect.w-offset;
	g_sdlControllerFaceButtonsRect.y = winHeight-g_sdlControllerFaceButtonsRect.h-offset;
	// Repeat for D-pad (same dimensions as the face buttons, other side)
	g_sdlControllerDpadRect.w = g_sdlControllerDpadRect.h = g_sdlControllerFaceButtonsRect.w;
	g_sdlControllerDpadRect.x = offset;
	g_sdlControllerDpadRect.y = g_sdlControllerFaceButtonsRect.y;
	// Also this - text-input keyboard (somewhat different because the keyboard is rectangular, but not square-shaped)
	g_sdlControllerTextInputRect.w = minWinDim;
	g_sdlControllerTextInputRect.h = g_sdlControllerTextInputRect.w * ALTCONTROLLER_TEXTINPUT_KEYS_HEIGHT / ALTCONTROLLER_TEXTINPUT_KEYS_WIDTH;
	g_sdlControllerTextInputRect.x = (winWidth-g_sdlControllerTextInputRect.w)/2;
	g_sdlControllerTextInputRect.y = winHeight-g_sdlControllerTextInputRect.h;
	// Same with debug keys
	g_sdlControllerDebugKeysRect.w = minWinDim;
	g_sdlControllerDebugKeysRect.h = g_sdlControllerDebugKeysRect.w * ALTCONTROLLER_DEBUGKEYS_KEYS_HEIGHT / ALTCONTROLLER_DEBUGKEYS_KEYS_WIDTH;
	g_sdlControllerDebugKeysRect.x = (winWidth-g_sdlControllerDebugKeysRect.w)/2;
	g_sdlControllerDebugKeysRect.y = winHeight-g_sdlControllerDebugKeysRect.h;
}

void BEL_ST_ForceHostDisplayUpdate(void)
{
	g_sdlForceGfxControlUiRefresh = true; // HACK that technically does exactly what we want (even if controls are not drawn)
}

void BE_ST_SetScreenStartAddress(uint16_t crtc)
{
	g_sdlDoRefreshGfxOutput |= (g_sdlScreenStartAddress != crtc);
	g_sdlScreenStartAddress = crtc;
}

uint8_t *BE_ST_GetTextModeMemoryPtr(void)
{
	return g_sdlVidMem.text;
}

static uint32_t g_sdlEGACurrBGRAPaletteAndBorder[17], g_sdlEGACurrBGRAPaletteAndBorderCache[17];

/* Gets a value represeting 6 EGA signals determining a color number and
 * returns it in a "Blue Green Red Intensity" 4-bit format.
 * Usually, the 6 signals represented by the given input mean:
 * "Blue Green Red Secondary-Blue Secondary-Green Secondary-Red". However, for
 * the historical reason of compatibility with CGA monitors, on the 200-lines
 * modes used by Keen the Secondary-Green signal is treated as an Intensity
 * one and the two other intensity signals are ignored.
 */
static int BEL_ST_ConvertEGASignalToEGAEntry(int color)
{
	return (color & 7) | ((color & 16) >> 1);
}


void BE_ST_SetBorderColor(uint8_t color)
{
	g_sdlEGACurrBGRAPaletteAndBorder[16] = g_sdlEGABGRAScreenColors[BEL_ST_ConvertEGASignalToEGAEntry(color)];
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_EGASetPaletteAndBorder(const uint8_t *palette)
{
	for (int entry = 0; entry < 16; ++entry)
	{
		g_sdlEGACurrBGRAPaletteAndBorder[entry] =  g_sdlEGABGRAScreenColors[BEL_ST_ConvertEGASignalToEGAEntry(palette[entry])];
	}
	g_sdlEGACurrBGRAPaletteAndBorder[16] = g_sdlEGABGRAScreenColors[BEL_ST_ConvertEGASignalToEGAEntry(palette[16])];
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_EGASetPelPanning(uint8_t panning)
{
	g_sdlDoRefreshGfxOutput |= (g_sdlPelPanning != panning);
	g_sdlPelPanning = panning;
}

void BE_ST_EGASetLineWidth(uint8_t widthInBytes)
{
	g_sdlDoRefreshGfxOutput |= (g_sdlLineWidth != widthInBytes);
	g_sdlLineWidth = widthInBytes;
}

void BE_ST_EGASetSplitScreen(int16_t linenum)
{
	// VGA only for now (200-lines graphics modes)
	if (g_sdlTexHeight == GFX_TEX_HEIGHT)
	{
		// Because 200-lines modes are really double-scanned to 400,
		// a linenum of x was originally replaced with 2x-1 in id_vw.c.
		// In practice it should've probably been 2x+1, and this is how
		// we "correct" it here (one less black line in Catacomb Abyss
		// before gameplay begins in a map, above the status bar).
		g_sdlSplitScreenLine = linenum/2;
	}
	else
		g_sdlSplitScreenLine = linenum;
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_EGAUpdateGFXByte(uint16_t destOff, uint8_t srcVal, uint16_t planeMask)
{
	if (planeMask & 1)
		g_sdlVidMem.egaGfx[0][destOff] = srcVal;
	if (planeMask & 2)
		g_sdlVidMem.egaGfx[1][destOff] = srcVal;
	if (planeMask & 4)
		g_sdlVidMem.egaGfx[2][destOff] = srcVal;
	if (planeMask & 8)
		g_sdlVidMem.egaGfx[3][destOff] = srcVal;
	g_sdlDoRefreshGfxOutput = true;
}

// Same as BE_ST_EGAUpdateGFXByte but picking specific bits out of each byte, and WITHOUT plane mask
void BE_ST_EGAUpdateGFXBits(uint16_t destOff, uint8_t srcVal, uint8_t bitsMask)
{
	g_sdlVidMem.egaGfx[0][destOff] = (g_sdlVidMem.egaGfx[0][destOff] & ~bitsMask) | (srcVal & bitsMask); 
	g_sdlVidMem.egaGfx[1][destOff] = (g_sdlVidMem.egaGfx[1][destOff] & ~bitsMask) | (srcVal & bitsMask); 
	g_sdlVidMem.egaGfx[2][destOff] = (g_sdlVidMem.egaGfx[2][destOff] & ~bitsMask) | (srcVal & bitsMask); 
	g_sdlVidMem.egaGfx[3][destOff] = (g_sdlVidMem.egaGfx[3][destOff] & ~bitsMask) | (srcVal & bitsMask); 
	g_sdlDoRefreshGfxOutput = true;
}

// Based on BE_Cross_LinearToWrapped_MemCopy
static void BEL_ST_LinearToEGAPlane_MemCopy(uint8_t *planeDstPtr, uint16_t planeDstOff, const uint8_t *linearSrc, uint16_t num)
{
	uint16_t bytesToEnd = 0x10000-planeDstOff;
	if (num <= bytesToEnd)
	{
		memcpy(planeDstPtr+planeDstOff, linearSrc, num);
	}
	else
	{
		memcpy(planeDstPtr+planeDstOff, linearSrc, bytesToEnd);
		memcpy(planeDstPtr, linearSrc+bytesToEnd, num-bytesToEnd);
	}
	g_sdlDoRefreshGfxOutput = true;
}

// Based on BE_Cross_WrappedToLinear_MemCopy
static void BEL_ST_EGAPlaneToLinear_MemCopy(uint8_t *linearDst, const uint8_t *planeSrcPtr, uint16_t planeSrcOff, uint16_t num)
{
	uint16_t bytesToEnd = 0x10000-planeSrcOff;
	if (num <= bytesToEnd)
	{
		memcpy(linearDst, planeSrcPtr+planeSrcOff, num);
	}
	else
	{
		memcpy(linearDst, planeSrcPtr+planeSrcOff, bytesToEnd);
		memcpy(linearDst+bytesToEnd, planeSrcPtr, num-bytesToEnd);
	}
	//No need to since we just read screen data
	//g_sdlDoRefreshGfxOutput = true;
}

// Based on BE_Cross_WrappedToWrapped_MemCopy
static void BEL_ST_EGAPlaneToEGAPlane_MemCopy(uint8_t *planeCommonPtr, uint16_t planeDstOff, uint16_t planeSrcOff, uint16_t num)
{
	uint16_t srcBytesToEnd = 0x10000-planeSrcOff;
	uint16_t dstBytesToEnd = 0x10000-planeDstOff;
	if (num <= srcBytesToEnd)
	{
		// Source is linear: Same as BE_Cross_LinearToWrapped_MemCopy here
		if (num <= dstBytesToEnd)
		{
			memcpy(planeCommonPtr+planeDstOff, planeCommonPtr+planeSrcOff, num);
		}
		else
		{
			memcpy(planeCommonPtr+planeDstOff, planeCommonPtr+planeSrcOff, dstBytesToEnd);
			memcpy(planeCommonPtr, planeCommonPtr+planeSrcOff+dstBytesToEnd, num-dstBytesToEnd);
		}
	}
	// Otherwise, check if at least the destination is linear
	else if (num <= dstBytesToEnd)
	{
		// Destination is linear: Same as BE_Cross_WrappedToLinear_MemCopy, non-linear source
		memcpy(planeCommonPtr+planeDstOff, planeCommonPtr+planeSrcOff, srcBytesToEnd);
		memcpy(planeCommonPtr+planeDstOff+srcBytesToEnd, planeCommonPtr, num-srcBytesToEnd);
	}
	// BOTH buffers have wrapping. We don't check separately if
	// srcBytesToEnd==dstBytesToEnd (in such a case planeDstOff==planeSrcOff...)
	else if (srcBytesToEnd <= dstBytesToEnd)
	{
		memcpy(planeCommonPtr+planeDstOff, planeCommonPtr+planeSrcOff, srcBytesToEnd);
		memcpy(planeCommonPtr+planeDstOff+srcBytesToEnd, planeCommonPtr, dstBytesToEnd-srcBytesToEnd);
		memcpy(planeCommonPtr, planeCommonPtr+(dstBytesToEnd-srcBytesToEnd), num-dstBytesToEnd);
	}
	else // srcBytesToEnd > dstBytesToEnd
	{
		memcpy(planeCommonPtr+planeDstOff, planeCommonPtr+planeSrcOff, dstBytesToEnd);
		memcpy(planeCommonPtr, planeCommonPtr+planeSrcOff+dstBytesToEnd, srcBytesToEnd-dstBytesToEnd);
		memcpy(planeCommonPtr+(srcBytesToEnd-dstBytesToEnd), planeCommonPtr, num-srcBytesToEnd);
	}
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_EGAUpdateGFXBuffer(uint16_t destOff, const uint8_t *srcPtr, uint16_t num, uint16_t planeMask)
{
	if (planeMask & 1)
		BEL_ST_LinearToEGAPlane_MemCopy(g_sdlVidMem.egaGfx[0], destOff, srcPtr, num);
	if (planeMask & 2)
		BEL_ST_LinearToEGAPlane_MemCopy(g_sdlVidMem.egaGfx[1], destOff, srcPtr, num);
	if (planeMask & 4)
		BEL_ST_LinearToEGAPlane_MemCopy(g_sdlVidMem.egaGfx[2], destOff, srcPtr, num);
	if (planeMask & 8)
		BEL_ST_LinearToEGAPlane_MemCopy(g_sdlVidMem.egaGfx[3], destOff, srcPtr, num);
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_EGAUpdateGFXByteScrToScr(uint16_t destOff, uint16_t srcOff)
{
	g_sdlVidMem.egaGfx[0][destOff] = g_sdlVidMem.egaGfx[0][srcOff];
	g_sdlVidMem.egaGfx[1][destOff] = g_sdlVidMem.egaGfx[1][srcOff];
	g_sdlVidMem.egaGfx[2][destOff] = g_sdlVidMem.egaGfx[2][srcOff];
	g_sdlVidMem.egaGfx[3][destOff] = g_sdlVidMem.egaGfx[3][srcOff];
	g_sdlDoRefreshGfxOutput = true;
}

// Same as BE_ST_EGAUpdateGFXByteScrToScr but with plane mask (added for Catacomb Abyss vanilla bug reproduction/workaround)
void BE_ST_EGAUpdateGFXByteWithPlaneMaskScrToScr(uint16_t destOff, uint16_t srcOff, uint16_t planeMask)
{
	if (planeMask & 1)
		g_sdlVidMem.egaGfx[0][destOff] = g_sdlVidMem.egaGfx[0][srcOff];
	if (planeMask & 2)
		g_sdlVidMem.egaGfx[1][destOff] = g_sdlVidMem.egaGfx[1][srcOff];
	if (planeMask & 4)
		g_sdlVidMem.egaGfx[2][destOff] = g_sdlVidMem.egaGfx[2][srcOff];
	if (planeMask & 8)
		g_sdlVidMem.egaGfx[3][destOff] = g_sdlVidMem.egaGfx[3][srcOff];
	g_sdlDoRefreshGfxOutput = true;
}

// Same as BE_ST_EGAUpdateGFXByteScrToScr but picking specific bits out of each byte
void BE_ST_EGAUpdateGFXBitsScrToScr(uint16_t destOff, uint16_t srcOff, uint8_t bitsMask)
{
	g_sdlVidMem.egaGfx[0][destOff] = (g_sdlVidMem.egaGfx[0][destOff] & ~bitsMask) | (g_sdlVidMem.egaGfx[0][srcOff] & bitsMask); 
	g_sdlVidMem.egaGfx[1][destOff] = (g_sdlVidMem.egaGfx[1][destOff] & ~bitsMask) | (g_sdlVidMem.egaGfx[1][srcOff] & bitsMask); 
	g_sdlVidMem.egaGfx[2][destOff] = (g_sdlVidMem.egaGfx[2][destOff] & ~bitsMask) | (g_sdlVidMem.egaGfx[2][srcOff] & bitsMask); 
	g_sdlVidMem.egaGfx[3][destOff] = (g_sdlVidMem.egaGfx[3][destOff] & ~bitsMask) | (g_sdlVidMem.egaGfx[3][srcOff] & bitsMask); 
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_EGAUpdateGFXBufferScrToScr(uint16_t destOff, uint16_t srcOff, uint16_t num)
{
	BEL_ST_EGAPlaneToEGAPlane_MemCopy(g_sdlVidMem.egaGfx[0], destOff, srcOff, num);
	BEL_ST_EGAPlaneToEGAPlane_MemCopy(g_sdlVidMem.egaGfx[1], destOff, srcOff, num);
	BEL_ST_EGAPlaneToEGAPlane_MemCopy(g_sdlVidMem.egaGfx[2], destOff, srcOff, num);
	BEL_ST_EGAPlaneToEGAPlane_MemCopy(g_sdlVidMem.egaGfx[3], destOff, srcOff, num);
	g_sdlDoRefreshGfxOutput = true;
}

uint8_t BE_ST_EGAFetchGFXByte(uint16_t destOff, uint16_t planenum)
{
	return g_sdlVidMem.egaGfx[planenum][destOff];
}

void BE_ST_EGAFetchGFXBuffer(uint8_t *destPtr, uint16_t srcOff, uint16_t num, uint16_t planenum)
{
	BEL_ST_EGAPlaneToLinear_MemCopy(destPtr, g_sdlVidMem.egaGfx[planenum], srcOff, num);
}

void BE_ST_EGAUpdateGFXPixel4bpp(uint16_t destOff, uint8_t color, uint8_t bitsMask)
{
	g_sdlVidMem.egaGfx[0][destOff] = (g_sdlVidMem.egaGfx[0][destOff] & ~bitsMask) | ((-(color&1)) & bitsMask);
	g_sdlVidMem.egaGfx[1][destOff] = (g_sdlVidMem.egaGfx[1][destOff] & ~bitsMask) | ((-((color&2)>>1)) & bitsMask);
	g_sdlVidMem.egaGfx[2][destOff] = (g_sdlVidMem.egaGfx[2][destOff] & ~bitsMask) | ((-((color&4)>>2)) & bitsMask);
	g_sdlVidMem.egaGfx[3][destOff] = (g_sdlVidMem.egaGfx[3][destOff] & ~bitsMask) | ((-((color&8)>>3)) & bitsMask);
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_EGAUpdateGFXPixel4bppRepeatedly(uint16_t destOff, uint8_t color, uint16_t count, uint8_t bitsMask)
{
	for (uint16_t loopVar = 0; loopVar < count; ++loopVar, ++destOff)
	{
		BE_ST_EGAUpdateGFXPixel4bpp(destOff, color, bitsMask);
	}
}

void BE_ST_EGAXorGFXByte(uint16_t destOff, uint8_t srcVal, uint16_t planeMask)
{
	if (planeMask & 1)
		g_sdlVidMem.egaGfx[0][destOff] ^= srcVal;
	if (planeMask & 2)
		g_sdlVidMem.egaGfx[1][destOff] ^= srcVal;
	if (planeMask & 4)
		g_sdlVidMem.egaGfx[2][destOff] ^= srcVal;
	if (planeMask & 8)
		g_sdlVidMem.egaGfx[3][destOff] ^= srcVal;
	g_sdlDoRefreshGfxOutput = true;
}

// Like BE_ST_EGAXorGFXByte, but:
// - OR instead of XOR.
// - All planes are updated.
// - Only specific bits are updated in each plane's byte.
void BE_ST_EGAOrGFXBits(uint16_t destOff, uint8_t srcVal, uint8_t bitsMask)
{
	g_sdlVidMem.egaGfx[0][destOff] |= (srcVal & bitsMask); 
	g_sdlVidMem.egaGfx[1][destOff] |= (srcVal & bitsMask); 
	g_sdlVidMem.egaGfx[2][destOff] |= (srcVal & bitsMask); 
	g_sdlVidMem.egaGfx[3][destOff] |= (srcVal & bitsMask); 
	g_sdlDoRefreshGfxOutput = true;
}



void BE_ST_CGAUpdateGFXBufferFromWrappedMem(const uint8_t *segPtr, const uint8_t *offInSegPtr, uint16_t byteLineWidth)
{
	const uint8_t *endSegPtr = segPtr + 0x10000;
	uint8_t *cgaHostPtr = g_sdlHostScrMem.cgaGfx, *cgaHostCachePtr = g_sdlHostScrMemCache.cgaGfx;
	// Remember this is a wrapped copy, offInSegPtr points inside a 65536-bytes long segment beginning at segPtr;
	// But we still want to skip some bytes, assuming byteLineWidth sources bytes per line (picking 80 out of these)

	int lineBytesLeft = byteLineWidth - GFX_TEX_WIDTH/4;
	for (int line = 0, byteInLine; line < GFX_TEX_HEIGHT; ++line)
	{
		for (byteInLine = 0; byteInLine < GFX_TEX_WIDTH/4; ++byteInLine, ++offInSegPtr, offInSegPtr = (offInSegPtr == endSegPtr) ? segPtr : offInSegPtr)
		{
			*cgaHostPtr = ((*offInSegPtr) & 0xC0) >> 6;
			g_sdlDoRefreshGfxOutput |= (*cgaHostPtr != *cgaHostCachePtr);
			*(cgaHostCachePtr++) = *(cgaHostPtr++);
			*cgaHostPtr = ((*offInSegPtr) & 0x30) >> 4;
			g_sdlDoRefreshGfxOutput |= (*cgaHostPtr != *cgaHostCachePtr);
			*(cgaHostCachePtr++) = *(cgaHostPtr++);
			*cgaHostPtr = ((*offInSegPtr) & 0x0C) >> 2;
			g_sdlDoRefreshGfxOutput |= (*cgaHostPtr != *cgaHostCachePtr);
			*(cgaHostCachePtr++) = *(cgaHostPtr++);
			*cgaHostPtr = ((*offInSegPtr) & 0x03);
			g_sdlDoRefreshGfxOutput |= (*cgaHostPtr != *cgaHostCachePtr);
			*(cgaHostCachePtr++) = *(cgaHostPtr++);
		}
		offInSegPtr += lineBytesLeft;
		if (offInSegPtr >= endSegPtr)
		{
			offInSegPtr = segPtr + (uint16_t)(offInSegPtr-segPtr);
		}
	}
}


void BE_ST_SetScreenMode(int mode)
{
	g_sdlDoRefreshGfxOutput = true;
	switch (mode)
	{
	case 3:
		g_sdlTexWidth = VGA_TXT_TEX_WIDTH;
		g_sdlTexHeight = VGA_TXT_TEX_HEIGHT;
		g_sdlTxtColor = 7;
		g_sdlTxtBackground = 0;
		g_sdlTxtCursorPosX = g_sdlTxtCursorPosY = 0;
		BE_ST_clrscr();
		g_sdlEGACurrBGRAPaletteAndBorder[16] = g_sdlEGABGRAScreenColors[0];
		break;
	case 4:
		g_sdlTexWidth = GFX_TEX_WIDTH;
		g_sdlTexHeight = GFX_TEX_HEIGHT;
		memset(g_sdlHostScrMem.cgaGfx, 0, sizeof(g_sdlHostScrMem.cgaGfx));
		g_sdlEGACurrBGRAPaletteAndBorder[16] = g_sdlEGABGRAScreenColors[0];
		g_sdlHostScrMemCache.cgaGfx[0] = g_sdlHostScrMem.cgaGfx[0]^0xFF; // Force refresh
		break;
	case 0xD:
		g_sdlTexWidth = GFX_TEX_WIDTH;
		g_sdlTexHeight = GFX_TEX_HEIGHT;
		memcpy(g_sdlEGACurrBGRAPaletteAndBorder, g_sdlEGABGRAScreenColors, sizeof(g_sdlEGABGRAScreenColors));
		g_sdlEGACurrBGRAPaletteAndBorder[16] = g_sdlEGABGRAScreenColors[0];
		g_sdlPelPanning = 0;
		g_sdlLineWidth = 40;
		g_sdlSplitScreenLine = -1;
		// HACK: Looks like this shouldn't be done if changing gfx->gfx
		if (g_sdlScreenMode != 0xE)
		{
			memset(g_sdlVidMem.egaGfx, 0, sizeof(g_sdlVidMem.egaGfx));
		}
		memset(g_sdlHostScrMem.egaGfx, 0, sizeof(g_sdlHostScrMem.egaGfx));
		g_sdlHostScrMemCache.egaGfx[0] = g_sdlHostScrMem.egaGfx[0]^0xFF; // Force refresh
		break;
	case 0xE:
		g_sdlTexWidth = 2*GFX_TEX_WIDTH;
		g_sdlTexHeight = GFX_TEX_HEIGHT;
		memcpy(g_sdlEGACurrBGRAPaletteAndBorder, g_sdlEGABGRAScreenColors, sizeof(g_sdlEGABGRAScreenColors));
		g_sdlEGACurrBGRAPaletteAndBorder[16] = g_sdlEGABGRAScreenColors[0];
		g_sdlPelPanning = 0;
		g_sdlLineWidth = 80;
		g_sdlSplitScreenLine = -1;
		// HACK: Looks like this shouldn't be done if changing gfx->gfx
		if (g_sdlScreenMode != 0xD)
		{
			memset(g_sdlVidMem.egaGfx,  0, sizeof(g_sdlVidMem.egaGfx));
		}
		memset(g_sdlHostScrMem.egaGfx,  0, sizeof(g_sdlHostScrMem.egaGfx));
		g_sdlHostScrMemCache.egaGfx[0] = g_sdlHostScrMem.egaGfx[0]^0xFF; // Force refresh
		break;
	}
	g_sdlScreenMode = mode;
	BEL_ST_SetGfxOutputRects(true);
	BEL_ST_RecreateTexture();
}

void BE_ST_textcolor(int color)
{
	g_sdlTxtColor = color;
}

void BE_ST_textbackground(int color)
{
	g_sdlTxtBackground = color;
}

void BE_ST_clrscr(void)
{
	uint8_t *currMemByte = g_sdlVidMem.text;
	for (int i = 0; i < 2*TXT_COLS_NUM*TXT_ROWS_NUM; ++i)
	{
		*(currMemByte++) = ' ';
		*(currMemByte++) = g_sdlTxtColor | (g_sdlTxtBackground << 4);
	}
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_MoveTextCursorTo(int x, int y)
{
	g_sdlTxtCursorPosX = x;
	g_sdlTxtCursorPosY = y;
	g_sdlDoRefreshGfxOutput = true;
}

void BE_ST_ToggleTextCursor(bool isEnabled)
{
	g_sdlTxtCursorEnabled = isEnabled;
}

static uint8_t *BEL_ST_printchar(uint8_t *currMemByte, char ch, bool iscolored, bool requirecrchar)
{
	if (ch == '\t')
	{
		int nextCursorPosX = (g_sdlTxtCursorPosX & ~7) + 8;
		for (; g_sdlTxtCursorPosX != nextCursorPosX; ++g_sdlTxtCursorPosX)
		{
			*(currMemByte++) = ' ';
			*currMemByte = iscolored ? (g_sdlTxtColor | (g_sdlTxtBackground << 4)) : *currMemByte;
			++currMemByte;
		}
	}
	else if (ch == '\r')
	{
		if (!requirecrchar)
			return currMemByte; // Do nothing

		g_sdlTxtCursorPosX = 0; // Carriage return
		currMemByte = g_sdlVidMem.text + 2*TXT_COLS_NUM*g_sdlTxtCursorPosY;
	}
	else if (ch == '\n')
	{
		if (!requirecrchar)
		{
			g_sdlTxtCursorPosX = 0; // Carriage return
		}
		++g_sdlTxtCursorPosY;
		currMemByte = g_sdlVidMem.text + 2*(g_sdlTxtCursorPosX+TXT_COLS_NUM*g_sdlTxtCursorPosY);
	}
	else
	{
		*(currMemByte++) = ch;
		*currMemByte = iscolored ? (g_sdlTxtColor | (g_sdlTxtBackground << 4)) : *currMemByte;
		++currMemByte;
		++g_sdlTxtCursorPosX;
	}

	// Go to next line
	if (g_sdlTxtCursorPosX == TXT_COLS_NUM)
	{
		g_sdlTxtCursorPosX = 0; // Carriage return
		++g_sdlTxtCursorPosY; // Line feed
		currMemByte = g_sdlVidMem.text + 2*TXT_COLS_NUM*g_sdlTxtCursorPosY;
	}
	// Shift screen by one line
	if (g_sdlTxtCursorPosY == TXT_ROWS_NUM)
	{
		--g_sdlTxtCursorPosY;
		// Scroll one line down
		uint8_t lastAttr = g_sdlVidMem.text[sizeof(g_sdlVidMem.text)-1];
		memmove(g_sdlVidMem.text, g_sdlVidMem.text+2*TXT_COLS_NUM, sizeof(g_sdlVidMem.text)-2*TXT_COLS_NUM);
		currMemByte = g_sdlVidMem.text+sizeof(g_sdlVidMem.text)-2*TXT_COLS_NUM;
		// New empty line
		for (int i = 0; i < TXT_COLS_NUM; ++i)
		{
			*(currMemByte++) = ' ';
			*(currMemByte++) = lastAttr;
		}
		currMemByte -= 2*TXT_COLS_NUM; // Go back to beginning of line
	}

	return currMemByte;
}

void BE_ST_puts(const char *str)
{
	uint8_t *currMemByte = g_sdlVidMem.text + 2*(g_sdlTxtCursorPosX+TXT_COLS_NUM*g_sdlTxtCursorPosY);
	for (; *str; ++str)
	{
		currMemByte = BEL_ST_printchar(currMemByte, *str, true, false);
	}
	BEL_ST_printchar(currMemByte, '\n', true, false);

	g_sdlDoRefreshGfxOutput = true;
}

static void BEL_ST_vprintf_impl(const char *format, va_list args, bool iscolored, bool requirecrchar);

void BE_ST_printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	BEL_ST_vprintf_impl(format, args, false, false);
	va_end(args);
}

void BE_ST_cprintf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	BEL_ST_vprintf_impl(format, args, true, true);
	va_end(args);
}

void BE_ST_vprintf(const char *format, va_list args)
{
	BEL_ST_vprintf_impl(format, args, false, false);
}

// There's no colored version of vprintf in the original codebase

static void BEL_ST_vprintf_impl(const char *format, va_list args, bool iscolored, bool requirecrchar)
{
	uint8_t *currMemByte = g_sdlVidMem.text + 2*(g_sdlTxtCursorPosX+TXT_COLS_NUM*g_sdlTxtCursorPosY);
	while (*format)
	{
		if (*format == '%')
		{
			switch (*(++format))
			{
			case '%':
				currMemByte = BEL_ST_printchar(currMemByte, '%', iscolored, requirecrchar);
				break;
			case 's':
			{
				for (const char *str = va_arg(args, char *); *str; ++str)
				{
					currMemByte = BEL_ST_printchar(currMemByte, *str, iscolored, requirecrchar);
				}
				break;
			}
			default:
			{
				// Do NOT constify this cause of hack...
				char errorMsg[] = "BEL_ST_vprintf_impl: Unsupported format specifier flag: X\n";
				errorMsg[strlen(errorMsg)-2] = *format; // Hack
				BE_ST_ExitWithErrorMsg(errorMsg);
			}
			}
		}
		else
		{
			currMemByte = BEL_ST_printchar(currMemByte, *format, iscolored, requirecrchar);
		}
		++format;
	}
	g_sdlDoRefreshGfxOutput = true;
}

static void BEL_ST_FinishHostDisplayUpdate(void)
{
	g_sdlForceGfxControlUiRefresh = false;

	if (g_sdlShowControllerUI)
	{
		if (g_sdlFaceButtonsAreShown)
		{
			SDL_RenderCopy(g_sdlRenderer, g_sdlFaceButtonsTexture, NULL, &g_sdlControllerFaceButtonsRect);
		}
		if (g_sdlDpadIsShown)
		{
			SDL_RenderCopy(g_sdlRenderer, g_sdlDpadTexture, NULL, &g_sdlControllerDpadRect);
		}
		if (g_sdlTextInputUIIsShown)
		{
			SDL_RenderCopy(g_sdlRenderer, g_sdlTextInputTexture, NULL, &g_sdlControllerTextInputRect);
		}
		if (g_sdlDebugKeysUIIsShown)
		{
			SDL_RenderCopy(g_sdlRenderer, g_sdlDebugKeysTexture, NULL, &g_sdlControllerDebugKeysRect);
		}
	}

        SDL_RenderPresent(g_sdlRenderer);
}


void BEL_ST_UpdateHostDisplay(void)
{
	if (g_sdlScreenMode == 3) // Text mode
	{
		// For graphics modes we don't have to refresh if g_sdlDoRefreshGfxOutput is set to false,
		// but in textual mode we have blinking characters and cursor to take care of
		static bool wereBlinkingCharsShown;
		static bool wasBlinkingCursorShown;
		bool areBlinkingCharsShown = (((uint64_t)(70086*SDL_GetTicks()/1000)/(1000*VGA_TXT_BLINK_VERT_FRAME_RATE)) % 2);
		bool isBlinkingCursorShown = g_sdlTxtCursorEnabled && (((uint64_t)(70086*SDL_GetTicks()/1000)/(1000*VGA_TXT_CURSOR_BLINK_VERT_FRAME_RATE)) % 2);
		if (!g_sdlDoRefreshGfxOutput && (wereBlinkingCharsShown == areBlinkingCharsShown) && (wasBlinkingCursorShown == isBlinkingCursorShown))
		{
			if (g_sdlForceGfxControlUiRefresh)
				goto dorefresh;
			return;
		}
		/****** Do update ******/
		wereBlinkingCharsShown = areBlinkingCharsShown;
		wasBlinkingCursorShown = isBlinkingCursorShown;
		void *pixels;
		int pitch;
		SDL_LockTexture(g_sdlTexture, NULL, &pixels, &pitch);
		uint32_t *screenPixelPtr = (uint32_t *)pixels;
		uint8_t currChar;
		const uint8_t *currCharFontPtr;
		uint32_t *currScrPixelPtr, currBackgroundColor, currCharColor;
		int txtByteCounter = 0;
		int currCharPixX, currCharPixY;
		for (int currCharY = 0, currCharX; currCharY < TXT_ROWS_NUM; ++currCharY)
		{
			// Draw striped lines
			for (currCharX = 0; currCharX < TXT_COLS_NUM; ++currCharX)
			{
				currChar = g_sdlVidMem.text[txtByteCounter];
				// Luckily, the width*height product is always divisible by 8...
				// Note that the actual width is always 8,
				// even in VGA mode. We convert to 9 while drawing.
				currCharFontPtr = g_vga_8x16TextFont + currChar*16*8;
				++txtByteCounter;
				currBackgroundColor = g_sdlEGABGRAScreenColors[(g_sdlVidMem.text[txtByteCounter] >> 4) & 7];
				// Should the character blink?
				if (!(g_sdlVidMem.text[txtByteCounter] & 0x80) || areBlinkingCharsShown)
					currCharColor = g_sdlEGABGRAScreenColors[g_sdlVidMem.text[txtByteCounter] & 15];
				else
					currCharColor = currBackgroundColor;
				++txtByteCounter;
				currScrPixelPtr = screenPixelPtr;
				for (currCharPixY = 0; currCharPixY < VGA_TXT_CHAR_PIX_HEIGHT; ++currCharPixY)
				{
					/* NOTE: The char width is actually 8
					in both of the EGA and VGA fonts. On the
					VGA case, the 9th pixel is determined
					according to the 8th and char number. */
					for (currCharPixX = 0; currCharPixX < 8; ++currCharPixX, ++currCharFontPtr, ++currScrPixelPtr)
					{
						*currScrPixelPtr = (*currCharFontPtr) ? currCharColor : currBackgroundColor;
					}
					// Add an extra 9th column on VGA
					*currScrPixelPtr = ((currChar < 192) || (currChar > 223)) ? currBackgroundColor : *(currScrPixelPtr-1);
					currScrPixelPtr += (g_sdlTexWidth-VGA_TXT_CHAR_PIX_WIDTH+1);
				}
				screenPixelPtr += VGA_TXT_CHAR_PIX_WIDTH;
			}
			// Go to the character right below current one
			screenPixelPtr += g_sdlTexWidth*(VGA_TXT_CHAR_PIX_HEIGHT-1);
		}
		// Finish with outputting the cursor if required
		currCharColor = g_sdlEGABGRAScreenColors[g_sdlVidMem.text[1+((TXT_COLS_NUM*g_sdlTxtCursorPosY+g_sdlTxtCursorPosX)<<1)] & 15];
		if (isBlinkingCursorShown)
		{
			screenPixelPtr = (uint32_t *)pixels+g_sdlTexWidth;
			screenPixelPtr += g_sdlTxtCursorPosY*VGA_TXT_CHAR_PIX_HEIGHT*g_sdlTexWidth;
			screenPixelPtr += g_sdlTxtCursorPosX*VGA_TXT_CHAR_PIX_WIDTH;
			// Out of 3 last scanlines of char, draw to the first 2.
			screenPixelPtr += (VGA_TXT_CHAR_PIX_HEIGHT-3)*g_sdlTexWidth;
			for (currCharPixY = 0; currCharPixY < 2; currCharPixY++)
			{
				for (currCharPixX = 0; currCharPixX < VGA_TXT_CHAR_PIX_WIDTH; currCharPixX++, screenPixelPtr++)
					*screenPixelPtr = currCharColor;
				screenPixelPtr += g_sdlTexWidth - VGA_TXT_CHAR_PIX_WIDTH;
			}
		}
	}
	else if (g_sdlScreenMode == 4) // CGA graphics
	{
		if (!g_sdlDoRefreshGfxOutput)
		{
			if (g_sdlForceGfxControlUiRefresh)
				goto dorefresh;
			return;
		}
		// That's easy now since there isn't a lot that can be done...
		void *pixels;
		int pitch;
		SDL_LockTexture(g_sdlTexture, NULL, &pixels, &pitch);
		uint32_t *currPixPtr = (uint32_t *)pixels;
		uint8_t *currPalPixPtr = g_sdlHostScrMem.cgaGfx;
		for (int pixnum = 0; pixnum < GFX_TEX_WIDTH*GFX_TEX_HEIGHT; ++pixnum, ++currPixPtr, ++currPalPixPtr)
		{
			*currPixPtr = g_sdlCGAGfxBGRAScreenColors[*currPalPixPtr];
		}
	}
	else // EGA graphics mode 0xD or 0xE
	{
		if (!g_sdlDoRefreshGfxOutput)
		{
			if (g_sdlForceGfxControlUiRefresh)
				goto dorefresh;
			return;
		}
		uint16_t currLineFirstByte = (g_sdlScreenStartAddress + g_sdlPelPanning/8) % 0x10000;
		uint8_t panningWithinInByte = g_sdlPelPanning%8;
		uint8_t *currPalPixPtr, *currPalPixCachePtr;
		bool doUpdate = false;
		for (int line = 0, col; line < GFX_TEX_HEIGHT; ++line)
		{
			uint8_t currBitNum = 7-panningWithinInByte, currBitMask = 1<<currBitNum;
			uint16_t currByte = currLineFirstByte;
			currPalPixPtr = g_sdlHostScrMem.egaGfx + line*g_sdlTexWidth;
			currPalPixCachePtr = g_sdlHostScrMemCache.egaGfx + line*g_sdlTexWidth;
			for (col = 0; col < g_sdlTexWidth; ++col, ++currPalPixPtr, ++currPalPixCachePtr)
			{
				*currPalPixPtr = ((g_sdlVidMem.egaGfx[0][currByte]&currBitMask)>>currBitNum) |
				                 (((g_sdlVidMem.egaGfx[1][currByte]&currBitMask)>>currBitNum)<<1) |
				                 (((g_sdlVidMem.egaGfx[2][currByte]&currBitMask)>>currBitNum)<<2) |
				                 (((g_sdlVidMem.egaGfx[3][currByte]&currBitMask)>>currBitNum)<<3);
				doUpdate |= (*currPalPixPtr != *currPalPixCachePtr);
				*currPalPixCachePtr = *currPalPixPtr;
				if (!(1+col % 8*g_sdlLineWidth)) // Check if we need to re-scan line
				{
					currBitNum = 7-panningWithinInByte;
					currBitMask = 1<<currBitNum;
					currByte = currLineFirstByte;
				}
				else // Otherwise just update variables as usual
				{
					if (currBitNum == 0)
					{
						++currByte; // This is done mod 0x10000 (with an uint16_t)
						currBitNum = 7;
						currBitMask = 0x80;
					}
					else
					{
						--currBitNum;
						currBitMask >>= 1;
					}
				}
			}

			if (g_sdlSplitScreenLine == line)
			{
				currLineFirstByte = 0; // NEXT line begins split screen, NOT g_sdlSplitScreenLine
			}
			else
			{
				currLineFirstByte += g_sdlLineWidth;
				currLineFirstByte %= 0x10000;
			}
		}
		if (!doUpdate)
		{
			int paletteAndBorderEntry;
			for (paletteAndBorderEntry = 0; paletteAndBorderEntry < 17; ++paletteAndBorderEntry)
			{
				if (g_sdlEGACurrBGRAPaletteAndBorder[paletteAndBorderEntry] != g_sdlEGACurrBGRAPaletteAndBorderCache[paletteAndBorderEntry])
				{
					g_sdlEGACurrBGRAPaletteAndBorderCache[paletteAndBorderEntry] = g_sdlEGACurrBGRAPaletteAndBorder[paletteAndBorderEntry];
					doUpdate = true;
				}
			}
			if (!doUpdate)
			{
				g_sdlDoRefreshGfxOutput = false;
				if (g_sdlForceGfxControlUiRefresh)
					goto dorefresh;
				return;
			}
		}
		void *pixels;
		int pitch;
		SDL_LockTexture(g_sdlTexture, NULL, &pixels, &pitch);
		uint32_t *currPixPtr = (uint32_t *)pixels;
		currPalPixPtr = g_sdlHostScrMem.egaGfx;
		for (int pixnum = 0; pixnum < g_sdlTexWidth*GFX_TEX_HEIGHT; ++pixnum, ++currPixPtr, ++currPalPixPtr)
		{
			*currPixPtr = g_sdlEGACurrBGRAPaletteAndBorder[*currPalPixPtr];
		}
	}

	g_sdlDoRefreshGfxOutput = false;
	SDL_UnlockTexture(g_sdlTexture);

dorefresh:
	SDL_SetRenderDrawColor(g_sdlRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(g_sdlRenderer);
	SDL_SetRenderDrawColor(g_sdlRenderer, (g_sdlEGACurrBGRAPaletteAndBorder[16]>>16)&0xFF, (g_sdlEGACurrBGRAPaletteAndBorder[16]>>8)&0xFF, g_sdlEGACurrBGRAPaletteAndBorder[16]&0xFF, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(g_sdlRenderer, &g_sdlAspectCorrectionBorderedRect);
	if (g_sdlTargetTexture)
	{
		SDL_SetRenderTarget(g_sdlRenderer, g_sdlTargetTexture);
		SDL_RenderCopy(g_sdlRenderer, g_sdlTexture, NULL, NULL);
		SDL_SetRenderTarget(g_sdlRenderer, NULL);
		SDL_RenderCopy(g_sdlRenderer, g_sdlTargetTexture, NULL, &g_sdlAspectCorrectionRect);
	}
	else
	{
		SDL_RenderCopy(g_sdlRenderer, g_sdlTexture, NULL, &g_sdlAspectCorrectionRect);
	}

	BEL_ST_FinishHostDisplayUpdate();
}
