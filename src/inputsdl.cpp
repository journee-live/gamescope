// For the nested case, reads input from the SDL window and send to wayland

#include <thread>
#include <mutex>

#include <signal.h>
#include <linux/input-event-codes.h>

#include <SDL.h>

#include "inputsdl.hpp"
#include "wlserver.hpp"
#include "main.hpp"

std::mutex g_SDLInitLock;

//-----------------------------------------------------------------------------
// Adapted from the key table in SDL/src/input/evdev/SDL_evdev.c
//-----------------------------------------------------------------------------
static uint32_t s_ScancodeTable[] =
{
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	0 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	1 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	2 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	3 */
	KEY_A,	/* SDL_SCANCODE_A	4 */
	KEY_B,	/* SDL_SCANCODE_B	5 */
	KEY_C,	/* SDL_SCANCODE_C	6 */
	KEY_D,	/* SDL_SCANCODE_D	7 */
	KEY_E,	/* SDL_SCANCODE_E	8 */
	KEY_F,	/* SDL_SCANCODE_F	9 */
	KEY_G,	/* SDL_SCANCODE_G	10 */
	KEY_H,	/* SDL_SCANCODE_H	11 */
	KEY_I,	/* SDL_SCANCODE_I	12 */
	KEY_J,	/* SDL_SCANCODE_J	13 */
	KEY_K,	/* SDL_SCANCODE_K	14 */
	KEY_L,	/* SDL_SCANCODE_L	15 */
	KEY_M,	/* SDL_SCANCODE_M	16 */
	KEY_N,	/* SDL_SCANCODE_N	17 */
	KEY_O,	/* SDL_SCANCODE_O	18 */
	KEY_P,	/* SDL_SCANCODE_P	19 */
	KEY_Q,	/* SDL_SCANCODE_Q	20 */
	KEY_R,	/* SDL_SCANCODE_R	21 */
	KEY_S,	/* SDL_SCANCODE_S	22 */
	KEY_T,	/* SDL_SCANCODE_T	23 */
	KEY_U,	/* SDL_SCANCODE_U	24 */
	KEY_V,	/* SDL_SCANCODE_V	25 */
	KEY_W,	/* SDL_SCANCODE_W	26 */
	KEY_X,	/* SDL_SCANCODE_X	27 */
	KEY_Y,	/* SDL_SCANCODE_Y	28 */
	KEY_Z,	/* SDL_SCANCODE_Z	29 */
	KEY_1,	/* SDL_SCANCODE_1	30 */
	KEY_2,	/* SDL_SCANCODE_2	31 */
	KEY_3,	/* SDL_SCANCODE_3	32 */
	KEY_4,	/* SDL_SCANCODE_4	33 */
	KEY_5,	/* SDL_SCANCODE_5	34 */
	KEY_6,	/* SDL_SCANCODE_6	35 */
	KEY_7,	/* SDL_SCANCODE_7	36 */
	KEY_8,	/* SDL_SCANCODE_8	37 */
	KEY_9,	/* SDL_SCANCODE_9	38 */
	KEY_0,	/* SDL_SCANCODE_0	39 */
	KEY_ENTER,	/* SDL_SCANCODE_RETURN	40 */
	KEY_ESC,	/* SDL_SCANCODE_ESCAPE	41 */
	KEY_BACKSPACE,	/* SDL_SCANCODE_BACKSPACE	42 */
	KEY_TAB,	/* SDL_SCANCODE_TAB	43 */
	KEY_SPACE,	/* SDL_SCANCODE_SPACE	44 */
	KEY_MINUS,	/* SDL_SCANCODE_MINUS	45 */
	KEY_EQUAL,	/* SDL_SCANCODE_EQUALS	46 */
	KEY_LEFTBRACE,	/* SDL_SCANCODE_LEFTBRACKET	47 */
	KEY_RIGHTBRACE,	/* SDL_SCANCODE_RIGHTBRACKET	48 */
	KEY_BACKSLASH,	/* SDL_SCANCODE_BACKSLASH	49 */
	KEY_RESERVED,	/* SDL_SCANCODE_NONUSHASH	50 */
	KEY_SEMICOLON,	/* SDL_SCANCODE_SEMICOLON	51 */
	KEY_APOSTROPHE,	/* SDL_SCANCODE_APOSTROPHE	52 */
	KEY_GRAVE,	/* SDL_SCANCODE_GRAVE	53 */
	KEY_COMMA,	/* SDL_SCANCODE_COMMA	54 */
	KEY_DOT,	/* SDL_SCANCODE_PERIOD	55 */
	KEY_SLASH,	/* SDL_SCANCODE_SLASH	56 */
	KEY_CAPSLOCK,	/* SDL_SCANCODE_CAPSLOCK	57 */
	KEY_F1,	/* SDL_SCANCODE_F1	58 */
	KEY_F2,	/* SDL_SCANCODE_F2	59 */
	KEY_F3,	/* SDL_SCANCODE_F3	60 */
	KEY_F4,	/* SDL_SCANCODE_F4	61 */
	KEY_F5,	/* SDL_SCANCODE_F5	62 */
	KEY_F6,	/* SDL_SCANCODE_F6	63 */
	KEY_F7,	/* SDL_SCANCODE_F7	64 */
	KEY_F8,	/* SDL_SCANCODE_F8	65 */
	KEY_F9,	/* SDL_SCANCODE_F9	66 */
	KEY_F10,	/* SDL_SCANCODE_F10	67 */
	KEY_F11,	/* SDL_SCANCODE_F11	68 */
	KEY_F12,	/* SDL_SCANCODE_F12	69 */
	KEY_RESERVED,	/* SDL_SCANCODE_PRINTSCREEN	70 */
	KEY_SCROLLLOCK,	/* SDL_SCANCODE_SCROLLLOCK	71 */
	KEY_PAUSE,	/* SDL_SCANCODE_PAUSE	72 */
	KEY_INSERT,	/* SDL_SCANCODE_INSERT	73 */
	KEY_HOME,	/* SDL_SCANCODE_HOME	74 */
	KEY_PAGEUP,	/* SDL_SCANCODE_PAGEUP	75 */
	KEY_DELETE,	/* SDL_SCANCODE_DELETE	76 */
	KEY_END,	/* SDL_SCANCODE_END	77 */
	KEY_PAGEDOWN,	/* SDL_SCANCODE_PAGEDOWN	78 */
	KEY_RIGHT,	/* SDL_SCANCODE_RIGHT	79 */
	KEY_LEFT,	/* SDL_SCANCODE_LEFT	80 */
	KEY_DOWN,	/* SDL_SCANCODE_DOWN	81 */
	KEY_UP,	/* SDL_SCANCODE_UP	82 */
	KEY_NUMLOCK,	/* SDL_SCANCODE_NUMLOCKCLEAR	83 */
	KEY_KPSLASH,	/* SDL_SCANCODE_KP_DIVIDE	84 */
	KEY_KPASTERISK,	/* SDL_SCANCODE_KP_MULTIPLY	85 */
	KEY_KPMINUS,	/* SDL_SCANCODE_KP_MINUS	86 */
	KEY_KPPLUS,	/* SDL_SCANCODE_KP_PLUS	87 */
	KEY_KPENTER,	/* SDL_SCANCODE_KP_ENTER	88 */
	KEY_KP1,	/* SDL_SCANCODE_KP_1	89 */
	KEY_KP2,	/* SDL_SCANCODE_KP_2	90 */
	KEY_KP3,	/* SDL_SCANCODE_KP_3	91 */
	KEY_KP4,	/* SDL_SCANCODE_KP_4	92 */
	KEY_KP5,	/* SDL_SCANCODE_KP_5	93 */
	KEY_KP6,	/* SDL_SCANCODE_KP_6	94 */
	KEY_KP7,	/* SDL_SCANCODE_KP_7	95 */
	KEY_KP8,	/* SDL_SCANCODE_KP_8	96 */
	KEY_KP9,	/* SDL_SCANCODE_KP_9	97 */
	KEY_KP0,	/* SDL_SCANCODE_KP_0	98 */
	KEY_KPDOT,	/* SDL_SCANCODE_KP_PERIOD	99 */
	KEY_RESERVED,	/* SDL_SCANCODE_NONUSBACKSLASH	100 */
	KEY_COMPOSE,	/* SDL_SCANCODE_APPLICATION	101 */
	KEY_POWER,	/* SDL_SCANCODE_POWER	102 */
	KEY_KPEQUAL,	/* SDL_SCANCODE_KP_EQUALS	103 */
	KEY_F13,	/* SDL_SCANCODE_F13	104 */
	KEY_F14,	/* SDL_SCANCODE_F14	105 */
	KEY_F15,	/* SDL_SCANCODE_F15	106 */
	KEY_F16,	/* SDL_SCANCODE_F16	107 */
	KEY_F17,	/* SDL_SCANCODE_F17	108 */
	KEY_F18,	/* SDL_SCANCODE_F18	109 */
	KEY_F19,	/* SDL_SCANCODE_F19	110 */
	KEY_F20,	/* SDL_SCANCODE_F20	111 */
	KEY_F21,	/* SDL_SCANCODE_F21	112 */
	KEY_F22,	/* SDL_SCANCODE_F22	113 */
	KEY_F23,	/* SDL_SCANCODE_F23	114 */
	KEY_F24,	/* SDL_SCANCODE_F24	115 */
	KEY_RESERVED,	/* SDL_SCANCODE_EXECUTE	116 */
	KEY_HELP,	/* SDL_SCANCODE_HELP	117 */
	KEY_MENU,	/* SDL_SCANCODE_MENU	118 */
	KEY_RESERVED,	/* SDL_SCANCODE_SELECT	119 */
	KEY_STOP,	/* SDL_SCANCODE_STOP	120 */
	KEY_AGAIN,	/* SDL_SCANCODE_AGAIN	121 */
	KEY_UNDO,	/* SDL_SCANCODE_UNDO	122 */
	KEY_CUT,	/* SDL_SCANCODE_CUT	123 */
	KEY_COPY,	/* SDL_SCANCODE_COPY	124 */
	KEY_PASTE,	/* SDL_SCANCODE_PASTE	125 */
	KEY_FIND,	/* SDL_SCANCODE_FIND	126 */
	KEY_MUTE,	/* SDL_SCANCODE_MUTE	127 */
	KEY_VOLUMEUP,	/* SDL_SCANCODE_VOLUMEUP	128 */
	KEY_VOLUMEDOWN,	/* SDL_SCANCODE_VOLUMEDOWN	129 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	130 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	131 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	132 */
	KEY_KPJPCOMMA,	/* SDL_SCANCODE_KP_COMMA	133 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_EQUALSAS400	134 */
	KEY_RESERVED,	/* SDL_SCANCODE_INTERNATIONAL1	135 */
	KEY_RESERVED,	/* SDL_SCANCODE_INTERNATIONAL2	136 */
	KEY_YEN,	/* SDL_SCANCODE_INTERNATIONAL3	137 */
	KEY_RESERVED,	/* SDL_SCANCODE_INTERNATIONAL4	138 */
	KEY_RESERVED,	/* SDL_SCANCODE_INTERNATIONAL5	139 */
	KEY_RESERVED,	/* SDL_SCANCODE_INTERNATIONAL6	140 */
	KEY_RESERVED,	/* SDL_SCANCODE_INTERNATIONAL7	141 */
	KEY_RESERVED,	/* SDL_SCANCODE_INTERNATIONAL8	142 */
	KEY_RESERVED,	/* SDL_SCANCODE_INTERNATIONAL9	143 */
	KEY_HANGEUL,	/* SDL_SCANCODE_LANG1	144 */
	KEY_HANJA,	/* SDL_SCANCODE_LANG2	145 */
	KEY_KATAKANA,	/* SDL_SCANCODE_LANG3	146 */
	KEY_HIRAGANA,	/* SDL_SCANCODE_LANG4	147 */
	KEY_ZENKAKUHANKAKU,	/* SDL_SCANCODE_LANG5	148 */
	KEY_RESERVED,	/* SDL_SCANCODE_LANG6	149 */
	KEY_RESERVED,	/* SDL_SCANCODE_LANG7	150 */
	KEY_RESERVED,	/* SDL_SCANCODE_LANG8	151 */
	KEY_RESERVED,	/* SDL_SCANCODE_LANG9	152 */
	KEY_RESERVED,	/* SDL_SCANCODE_ALTERASE	153 */
	KEY_SYSRQ,	/* SDL_SCANCODE_SYSREQ	154 */
	KEY_RESERVED,	/* SDL_SCANCODE_CANCEL	155 */
	KEY_RESERVED,	/* SDL_SCANCODE_CLEAR	156 */
	KEY_RESERVED,	/* SDL_SCANCODE_PRIOR	157 */
	KEY_RESERVED,	/* SDL_SCANCODE_RETURN2	158 */
	KEY_RESERVED,	/* SDL_SCANCODE_SEPARATOR	159 */
	KEY_RESERVED,	/* SDL_SCANCODE_OUT	160 */
	KEY_RESERVED,	/* SDL_SCANCODE_OPER	161 */
	KEY_RESERVED,	/* SDL_SCANCODE_CLEARAGAIN	162 */
	KEY_RESERVED,	/* SDL_SCANCODE_CRSEL	163 */
	KEY_RESERVED,	/* SDL_SCANCODE_EXSEL	164 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	165 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	166 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	167 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	168 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	169 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	170 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	171 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	172 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	173 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	174 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	175 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_00	176 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_000	177 */
	KEY_RESERVED,	/* SDL_SCANCODE_THOUSANDSSEPARATOR	178 */
	KEY_RESERVED,	/* SDL_SCANCODE_DECIMALSEPARATOR	179 */
	KEY_RESERVED,	/* SDL_SCANCODE_CURRENCYUNIT	180 */
	KEY_RESERVED,	/* SDL_SCANCODE_CURRENCYSUBUNIT	181 */
	KEY_KPLEFTPAREN,	/* SDL_SCANCODE_KP_LEFTPAREN	182 */
	KEY_KPRIGHTPAREN,	/* SDL_SCANCODE_KP_RIGHTPAREN	183 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_LEFTBRACE	184 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_RIGHTBRACE	185 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_TAB	186 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_BACKSPACE	187 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_A	188 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_B	189 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_C	190 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_D	191 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_E	192 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_F	193 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_XOR	194 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_POWER	195 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_PERCENT	196 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_LESS	197 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_GREATER	198 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_AMPERSAND	199 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_DBLAMPERSAND	200 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_VERTICALBAR	201 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_DBLVERTICALBAR	202 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_COLON	203 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_HASH	204 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_SPACE	205 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_AT	206 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_EXCLAM	207 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_MEMSTORE	208 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_MEMRECALL	209 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_MEMCLEAR	210 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_MEMADD	211 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_MEMSUBTRACT	212 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_MEMMULTIPLY	213 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_MEMDIVIDE	214 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_PLUSMINUS	215 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_CLEAR	216 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_CLEARENTRY	217 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_BINARY	218 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_OCTAL	219 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_DECIMAL	220 */
	KEY_RESERVED,	/* SDL_SCANCODE_KP_HEXADECIMAL	221 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	222 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	223 */
	KEY_LEFTCTRL,	/* SDL_SCANCODE_LCTRL	224 */
	KEY_LEFTSHIFT,	/* SDL_SCANCODE_LSHIFT	225 */
	KEY_LEFTALT,	/* SDL_SCANCODE_LALT	226 */
	KEY_LEFTMETA,	/* SDL_SCANCODE_LGUI	227 */
	KEY_RIGHTCTRL,	/* SDL_SCANCODE_RCTRL	228 */
	KEY_RIGHTSHIFT,	/* SDL_SCANCODE_RSHIFT	229 */
	KEY_RIGHTALT,	/* SDL_SCANCODE_RALT	230 */
	KEY_RIGHTMETA,	/* SDL_SCANCODE_RGUI	231 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	232 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	233 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	234 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	235 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	236 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	237 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	238 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	239 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	240 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	241 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	242 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	243 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	244 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	245 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	246 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	247 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	248 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	249 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	250 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	251 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	252 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	253 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	254 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	255 */
	KEY_RESERVED,	/* SDL_SCANCODE_UNKNOWN	256 */
	KEY_RESERVED,	/* SDL_SCANCODE_MODE	257 */
	KEY_NEXTSONG,	/* SDL_SCANCODE_AUDIONEXT	258 */
	KEY_PREVIOUSSONG,	/* SDL_SCANCODE_AUDIOPREV	259 */
	KEY_STOPCD,	/* SDL_SCANCODE_AUDIOSTOP	260 */
	KEY_PLAYPAUSE,	/* SDL_SCANCODE_AUDIOPLAY	261 */
	KEY_RESERVED,	/* SDL_SCANCODE_AUDIOMUTE	262 */
	KEY_RESERVED,	/* SDL_SCANCODE_MEDIASELECT	263 */
	KEY_WWW,	/* SDL_SCANCODE_WWW	264 */
	KEY_MAIL,	/* SDL_SCANCODE_MAIL	265 */
	KEY_CALC,	/* SDL_SCANCODE_CALCULATOR	266 */
	KEY_COMPUTER,	/* SDL_SCANCODE_COMPUTER	267 */
	KEY_RESERVED,	/* SDL_SCANCODE_AC_SEARCH	268 */
	KEY_HOMEPAGE,	/* SDL_SCANCODE_AC_HOME	269 */
	KEY_BACK,	/* SDL_SCANCODE_AC_BACK	270 */
	KEY_FORWARD,	/* SDL_SCANCODE_AC_FORWARD	271 */
	KEY_RESERVED,	/* SDL_SCANCODE_AC_STOP	272 */
	KEY_REFRESH,	/* SDL_SCANCODE_AC_REFRESH	273 */
	KEY_BOOKMARKS,	/* SDL_SCANCODE_AC_BOOKMARKS	274 */
	KEY_RESERVED,	/* SDL_SCANCODE_BRIGHTNESSDOWN	275 */
	KEY_RESERVED,	/* SDL_SCANCODE_BRIGHTNESSUP	276 */
	KEY_RESERVED,	/* SDL_SCANCODE_DISPLAYSWITCH	277 */
	KEY_RESERVED,	/* SDL_SCANCODE_KBDILLUMTOGGLE	278 */
	KEY_RESERVED,	/* SDL_SCANCODE_KBDILLUMDOWN	279 */
	KEY_RESERVED,	/* SDL_SCANCODE_KBDILLUMUP	280 */
	KEY_EJECTCD,	/* SDL_SCANCODE_EJECT	281 */
	KEY_SLEEP,	/* SDL_SCANCODE_SLEEP	282 */
	KEY_PROG1,	/* SDL_SCANCODE_APP1	283 */
	KEY_RESERVED,	/* SDL_SCANCODE_APP2	284 */
};


//-----------------------------------------------------------------------------
// Purpose: Convert from the remote scancode to a Linux event keycode
//-----------------------------------------------------------------------------
static inline uint32_t SDLScancodeToLinuxKey( uint32_t nScancode )
{
	if ( nScancode < sizeof( s_ScancodeTable ) / sizeof( s_ScancodeTable[0] ) )
	{
		return s_ScancodeTable[ nScancode ];
	}
	return KEY_RESERVED;
}

static inline int SDLButtonToLinuxButton( int SDLButton )
{
	switch ( SDLButton )
	{
		case SDL_BUTTON_LEFT: return BTN_LEFT;
		case SDL_BUTTON_MIDDLE: return BTN_MIDDLE;
		case SDL_BUTTON_RIGHT: return BTN_RIGHT;
		case SDL_BUTTON_X1: return BTN_FORWARD;
		case SDL_BUTTON_X2: return BTN_BACK;
		default: return 0;
	}
}

void updateOutputRefresh( void )
{
	int display_index = 0;
	SDL_DisplayMode mode = { SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0 };
	
	display_index = SDL_GetWindowDisplayIndex( window );
	if ( SDL_GetDesktopDisplayMode( display_index, &mode ) == 0 )
	{
		g_nOutputRefresh = mode.refresh_rate;
	}
}

void inputSDLThreadRun( void )
{
	// see wlroots xwayland startup and how wl_event_loop_add_signal works
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	SDL_Event event;
	SDL_Keymod mod;
	uint32_t key;
	static bool bFullscreen = false;
	
	SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS );
	SDL_SetRelativeMouseMode(SDL_TRUE);
	
	g_SDLInitLock.unlock();
	
	while( SDL_WaitEvent( &event ) )
	{
		switch( event.type )
		{
			case SDL_MOUSEMOTION:
				wlserver_lock();
				wlserver_mousemotion( event.motion.xrel, event.motion.yrel, event.motion.timestamp );
				wlserver_unlock();
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				wlserver_lock();
				wlserver_mousebutton( SDLButtonToLinuxButton( event.button.button ),
									  event.button.state == SDL_PRESSED,
									  event.button.timestamp );
				wlserver_unlock();
				break;
			case SDL_MOUSEWHEEL:
				wlserver_lock();
				wlserver_mousewheel( -event.wheel.x, -event.wheel.y, event.wheel.timestamp );
				wlserver_unlock();
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				mod = SDL_GetModState();
				key = SDLScancodeToLinuxKey( event.key.keysym.scancode );
				
				if ( event.type == SDL_KEYUP && mod & KMOD_LGUI )
				{
					switch ( key )
					{
						case KEY_F:
							bFullscreen = !bFullscreen;
							SDL_SetWindowFullscreen( window, bFullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0 );
							break;
						case KEY_N:
							g_bFilterGameWindow = !g_bFilterGameWindow;
							break;
						default:
							goto client;
						
					}
					break;
				}
client:
				wlserver_lock();
				wlserver_key( key, event.type == SDL_KEYDOWN, event.key.timestamp );
				wlserver_unlock();
				break;
			case SDL_WINDOWEVENT:
				switch( event.window.event )
				{
					default:
						break;
					case SDL_WINDOWEVENT_MOVED:
					case SDL_WINDOWEVENT_SHOWN:
						updateOutputRefresh();
						break;
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						g_nOutputWidth = event.window.data1;
						g_nOutputHeight = event.window.data2;
						
						updateOutputRefresh();
						
						break;
				}
				break;
			default:
				break;
		}
	}
}

bool inputsdl_init( void )
{
	g_SDLInitLock.lock();

	std::thread inputSDLThread( inputSDLThreadRun );
	inputSDLThread.detach();
	
	// When this returns SDL_Init should be over
	g_SDLInitLock.lock();

	return true;
}
