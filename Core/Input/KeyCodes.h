#pragma once

// Key coded based on glfw.h key codes
namespace LSIS {

	/* The unknown key */
#define KEY_UNKNOWN            -1

/* Printable keys */
#define KEY_SPACE              32
#define KEY_APOSTROPHE         39  /* ' */
#define KEY_COMMA              44  /* , */
#define KEY_MINUS              45  /* - */
#define KEY_PERIOD             46  /* . */
#define KEY_SLASH              47  /* / */
#define KEY_0                  48
#define KEY_1                  49
#define KEY_2                  50
#define KEY_3                  51
#define KEY_4                  52
#define KEY_5                  53
#define KEY_6                  54
#define KEY_7                  55
#define KEY_8                  56
#define KEY_9                  57
#define KEY_SEMICOLON          59  /* ; */
#define KEY_EQUAL              61  /* = */
#define KEY_A                  65
#define KEY_B                  66
#define KEY_C                  67
#define KEY_D                  68
#define KEY_E                  69
#define KEY_F                  70
#define KEY_G                  71
#define KEY_H                  72
#define KEY_I                  73
#define KEY_J                  74
#define KEY_K                  75
#define KEY_L                  76
#define KEY_M                  77
#define KEY_N                  78
#define KEY_O                  79
#define KEY_P                  80
#define KEY_Q                  81
#define KEY_R                  82
#define KEY_S                  83
#define KEY_T                  84
#define KEY_U                  85
#define KEY_V                  86
#define KEY_W                  87
#define KEY_X                  88
#define KEY_Y                  89
#define KEY_Z                  90
#define KEY_LEFT_BRACKET       91  /* [ */
#define KEY_BACKSLASH          92  /* \ */
#define KEY_RIGHT_BRACKET      93  /* ] */
#define KEY_GRAVE_ACCENT       96  /* ` */
#define KEY_WORLD_1            161 /* non-US #1 */
#define KEY_WORLD_2            162 /* non-US #2 */

	inline bool isPrintable(int keycode) { return keycode >= 32 && keycode <= 96; }

	// Function keys
#define KEY_ESCAPE             256
#define KEY_ENTER              257
#define KEY_TAB                258
#define KEY_BACKSPACE          259
#define KEY_INSERT             260
#define KEY_DELETE             261
#define KEY_RIGHT              262
#define KEY_LEFT               263
#define KEY_DOWN               264
#define KEY_UP                 265
#define KEY_PAGE_UP            266
#define KEY_PAGE_DOWN          267
#define KEY_HOME               268
#define KEY_END                269
#define KEY_CAPS_LOCK          280
#define KEY_SCROLL_LOCK        281
#define KEY_NUM_LOCK           282
#define KEY_PRINT_SCREEN       283
#define KEY_PAUSE              284
#define KEY_F1                 290
#define KEY_F2                 291
#define KEY_F3                 292
#define KEY_F4                 293
#define KEY_F5                 294
#define KEY_F6                 295
#define KEY_F7                 296
#define KEY_F8                 297
#define KEY_F9                 298
#define KEY_F10                299
#define KEY_F11                300
#define KEY_F12                301
#define KEY_F13                302
#define KEY_F14                303
#define KEY_F15                304
#define KEY_F16                305
#define KEY_F17                306
#define KEY_F18                307
#define KEY_F19                308
#define KEY_F20                309
#define KEY_F21                310
#define KEY_F22                311
#define KEY_F23                312
#define KEY_F24                313
#define KEY_F25                314
#define KEY_KP_0               320
#define KEY_KP_1               321
#define KEY_KP_2               322
#define KEY_KP_3               323
#define KEY_KP_4               324
#define KEY_KP_5               325
#define KEY_KP_6               326
#define KEY_KP_7               327
#define KEY_KP_8               328
#define KEY_KP_9               329
#define KEY_KP_DECIMAL         330
#define KEY_KP_DIVIDE          331
#define KEY_KP_MULTIPLY        332
#define KEY_KP_SUBTRACT        333
#define KEY_KP_ADD             334
#define KEY_KP_ENTER           335
#define KEY_KP_EQUAL           336
#define KEY_LEFT_SHIFT         340
#define KEY_LEFT_CONTROL       341
#define KEY_LEFT_ALT           342
#define KEY_LEFT_SUPER         343
#define KEY_RIGHT_SHIFT        344
#define KEY_RIGHT_CONTROL      345
#define KEY_RIGHT_ALT          346
#define KEY_RIGHT_SUPER        347
#define KEY_MENU               348

// Modifier codes
#define MOD_SHIFT           0x0001
#define MOD_CONTROL         0x0002
#define MOD_ALT             0x0004
#define MOD_SUPER           0x0008
#define MOD_CAPS_LOCK       0x0010
#define MOD_NUM_LOCK        0x0020

// Mouse button codes
#define MOUSE_BUTTON_1         0
#define MOUSE_BUTTON_2         1
#define MOUSE_BUTTON_3         2
#define MOUSE_BUTTON_4         3
#define MOUSE_BUTTON_5         4
#define MOUSE_BUTTON_6         5
#define MOUSE_BUTTON_7         6
#define MOUSE_BUTTON_8         7
#define MOUSE_BUTTON_LAST      MOUSE_BUTTON_8
#define MOUSE_BUTTON_LEFT      MOUSE_BUTTON_1
#define MOUSE_BUTTON_RIGHT     MOUSE_BUTTON_2
#define MOUSE_BUTTON_MIDDLE    MOUSE_BUTTON_3

	inline std::string GetKeyString(int key) {
		switch (key)
		{
		case KEY_UNKNOWN:
			return "UNKNOWN";
		case KEY_SPACE:
			return "SPACE";
		case KEY_APOSTROPHE:
			return "APOSTROPHE";
		case KEY_COMMA:
			return "COMMA";
		case KEY_MINUS:
			return "MINUS";
		case KEY_PERIOD:
			return "PERIOD";
		case KEY_SLASH:
			return "SLASH";
		case KEY_0:
			return "0";
			return "0";
		case KEY_1:
			return "1";
		case KEY_2:
			return "2";
		case KEY_3:
			return "3";
		case KEY_4:
			return "4";
		case KEY_5:
			return "5";
		case KEY_6:
			return "6";
		case KEY_7:
			return "7";
		case KEY_8:
			return "8";
		case KEY_9:
			return "9";
		case KEY_SEMICOLON:
			return "SEMICOLON";
		case KEY_EQUAL:
			return "EQUAL";
		case KEY_A:
			return "A";
		case KEY_B:
			return "B";
		case KEY_C:
			return "C";
		case KEY_D:
			return "D";
		case KEY_E:
			return "E";
		case KEY_F:
			return "F";
		case KEY_G:
			return "G";
		case KEY_H:
			return "H";
		case KEY_I:
			return "I";
		case KEY_J:
			return "J";
		case KEY_K:
			return "K";
		case KEY_L:
			return "L";
		case KEY_M:
			return "M";
		case KEY_N:
			return "N";
		case KEY_O:
			return "O";
		case KEY_P:
			return "P";
		case KEY_Q:
			return "Q";
		case KEY_R:
			return "R";
		case KEY_S:
			return "S";
		case KEY_T:
			return "T";
		case KEY_U:
			return "U";
		case KEY_V:
			return "V";
		case KEY_W:
			return "W";
		case KEY_X:
			return "X";
		case KEY_Y:
			return "Y";
		case KEY_Z:
			return "Z";
		case KEY_LEFT_BRACKET:
			return "LEFT_BRACKET";
		case KEY_BACKSLASH:
			return "BACKSLASH";
		case KEY_RIGHT_BRACKET:
			return "RIGHT_BRACKET";
		case KEY_GRAVE_ACCENT:
			return "GRAVE_ACCENT";
		case KEY_WORLD_1:
			return "WORLD_1";
		case KEY_WORLD_2:
			return "WORLD_2";
		case KEY_ESCAPE:
			return "ESCAPE";
		case KEY_ENTER:
			return "ENTER";
		case KEY_TAB:
			return "TAB";
		case KEY_BACKSPACE:
			return "BACKSPACE";
		case KEY_INSERT:
			return "INSERT";
		case KEY_DELETE:
			return "DELETE";
		case KEY_RIGHT:
			return "RIGHT";
		case KEY_LEFT:
			return "LEFT";
		case KEY_DOWN:
			return "DOWN";
		case KEY_UP:
			return "UP";
		case KEY_PAGE_UP:
			return "PAGE_UP";
		case KEY_PAGE_DOWN:
			return "PAGE_DOWN";
		case KEY_HOME:
			return "HOME";
		case KEY_END:
			return "END";
		case KEY_CAPS_LOCK:
			return "CAPS_LOCK";
		case KEY_SCROLL_LOCK:
			return "SCROLL_LOCK";
		case KEY_NUM_LOCK:
			return "NUM_LOCK";
		case KEY_PRINT_SCREEN:
			return "PRINT_SCREEN";
		case KEY_PAUSE:
			return "PAUSE";
		case KEY_F1:
			return "F1";
		case KEY_F2:
			return "F2";
		case KEY_F3:
			return "F3";
		case KEY_F4:
			return "F4";
		case KEY_F5:
			return "F5";
		case KEY_F6:
			return "F6";
		case KEY_F7:
			return "F7";
		case KEY_F8:
			return "F8";
		case KEY_F9:
			return "F9";
		case KEY_F10:
			return "F10";
		case KEY_F11:
			return "F11";
		case KEY_F12:
			return "F12";
		case KEY_F13:
			return "F13";
		case KEY_F14:
			return "F14";
		case KEY_F15:
			return "F15";
		case KEY_F16:
			return "F16";
		case KEY_F17:
			return "F17";
		case KEY_F18:
			return "F18";
		case KEY_F19:
			return "F19";
		case KEY_F20:
			return "F20";
		case KEY_F21:
			return "F21";
		case KEY_F22:
			return "F22";
		case KEY_F23:
			return "F23";
		case KEY_F24:
			return "F24";
		case KEY_F25:
			return "F25";
		case KEY_KP_0:
			return "KP_0";
		case KEY_KP_1:
			return "KP_1";
		case KEY_KP_2:
			return "KP_2";
		case KEY_KP_3:
			return "KP_3";
		case KEY_KP_4:
			return "KP_4";
		case KEY_KP_5:
			return "KP_5";
		case KEY_KP_6:
			return "KP_6";
		case KEY_KP_7:
			return "KP_7";
		case KEY_KP_8:
			return "KP_8";
		case KEY_KP_9:
			return "KP_9";
		case KEY_KP_DECIMAL:
			return "KP_DECIMAL";
		case KEY_KP_DIVIDE:
			return "KP_DIVIDE";
		case KEY_KP_MULTIPLY:
			return "KP_MULTIPLY";
		case KEY_KP_SUBTRACT:
			return "KP_SUBTRACT";
		case KEY_KP_ADD:
			return "KP_ADD";
		case KEY_KP_ENTER:
			return "KP_ENTER";
		case KEY_KP_EQUAL:
			return "KP_EQUAL";
		case KEY_LEFT_SHIFT:
			return "LEFT_SHIFT";
		case KEY_LEFT_CONTROL:
			return "LEFT_CONTROL";
		case KEY_LEFT_ALT:
			return "LEFT_ALT";
		case KEY_LEFT_SUPER:
			return "LEFT_SUPER";
		case KEY_RIGHT_SHIFT:
			return "RIGHT_SHIFT";
		case KEY_RIGHT_CONTROL:
			return "RIGHT_CONTROL";
		case KEY_RIGHT_ALT:
			return "RIGHT_ALT";
		case KEY_RIGHT_SUPER:
			return "RIGHT_SUPER";
		case KEY_MENU:
			return "MENU";
		default:
			return "UNKNOWN";
		}
	}
}