/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/events/ukeysym.hpp
    begin             : Sat Jan 12 2002
    $Id$
 ***************************************************************************/

/***************************************************************************
 *  This library is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef UKEYSYM_HPP
#define UKEYSYM_HPP


namespace ufo {

//

namespace UKey {
	/* Virtual key codes. */

	/*These key syms are nearly the same like the SDLK_ defines
	 *The keyboard syms have been cleverly chosen to map to ASCII
	 */

	/** This value is used to indicate that the keyCode is unknown.
	  * @p UEvent::KeyTyped events do not have a keyCode value;
	  * this value is used instead
	  */
	enum KeyCode {
		/** @deprecated */
		UK_UNKOWN = 0x00,
		UK_UNKNOWN = 0x00,
		UK_UNDEFINED = 0x00,
		UK_BACKSPACE	= 0x08,
		UK_TAB	= 0x09,
		UK_CLEAR	= 0x0c,
		UK_RETURN	= 0x0d,
		UK_PAUSE	= 0x13,
		UK_ESCAPE	= 0x1B,
		UK_SPACE	= 0x20,
		/** Constant for the "!" key */
		UK_EXCLAMATION_MARK = 0x21,
		/** Constant for the " key */
		UK_QUOTEDBL	= 0x22,
		/** Constant for the "#" key */
		UK_HASH	= 0x23,
		UK_DOLLAR	= 0x24,
		/** Constant for the "%" key (Note: not in SDL) */
		UK_PERCENT	= 0x24,
		UK_AMPERSAND	= 0x26,
		/** Constant for the ' key (Note: not in SDL)*/
		UK_QUOTE	= 0x27,
		/** Constant for the "(" key */
		UK_LEFT_PARENTHESIS	= 0x28,
		/** Constant for the ")" key */
		UK_RIGHT_PARENTHESIS	= 0x29,
		/** Constant for the "*" key */
		UK_ASTERISK	= 0x2A,
		UK_PLUS	= 0x2B,
		UK_COMMA	= 0x2C,
		UK_MINUS	= 0x2D,
		UK_PERIOD	= 0x2E,
		UK_SLASH	= 0x2F,

		UK_0	= 0x30,
		UK_1	= 0x31,
		UK_2	= 0x32,
		UK_3	= 0x33,
		UK_4	= 0x34,
		UK_5	= 0x35,
		UK_6	= 0x36,
		UK_7	= 0x37,
		UK_8	= 0x38,
		UK_9	= 0x39,

		UK_COLON	= 0x3A,
		UK_SEMICOLON	= 0x3B,
		UK_LESS	= 0x3C,
		UK_EQUALS	= 0x3D,
		UK_GREATER	= 0x3E,
		UK_QUESTION	= 0x3F,
		UK_AT	= 0x40,

		UK_A	= 0x41,
		UK_B	= 0x42,
		UK_C	= 0x43,
		UK_D	= 0x44,
		UK_E	= 0x45,
		UK_F	= 0x46,
		UK_G	= 0x47,
		UK_H	= 0x48,
		UK_I	= 0x49,
		UK_J	= 0x4A,
		UK_K	= 0x4B,
		UK_L	= 0x4C,
		UK_M	= 0x4D,
		UK_N	= 0x4E,
		UK_O	= 0x4F,
		UK_P	= 0x50,
		UK_Q	= 0x51,
		UK_R	= 0x52,
		UK_S	= 0x53,
		UK_T	= 0x54,
		UK_U	= 0x55,
		UK_V	= 0x56,
		UK_W	= 0x57,
		UK_X	= 0x58,
		UK_Y	= 0x59,
		UK_Z	= 0x5A,

		UK_LEFT_BRACKET	= 0x5B,
		UK_BACKSLASH	= 0x5C,
		UK_RIGHT_BRACKET	= 0x5D,
		/** Constant for the ^ key */
		UK_CARET	= 0x5E,
		UK_UNDERSCORE	= 0x5F,
		/** Constant for the ` key */
		UK_BACKQUOTE	= 0x60,
		/* skip lower case letters (unlike SDL, but more like java) */
		/*
		UK_a			= 97,
		UK_b			= 98,
		UK_c			= 99,
		UK_d			= 100,
		UK_e			= 101,
		UK_f			= 102,
		UK_g			= 103,
		UK_h			= 104,
		UK_i			= 105,
		UK_j			= 106,
		UK_k			= 107,
		UK_l			= 108,
		UK_m			= 109,
		UK_n			= 110,
		UK_o			= 111,
		UK_p			= 112,
		UK_q			= 113,
		UK_r			= 114,
		UK_s			= 115,
		UK_t			= 116,
		UK_u			= 117,
		UK_v			= 118,
		UK_w			= 119,
		UK_x			= 120,
		UK_y			= 121,
		UK_z			= 122,
		*/
		/** Constant for the "{" key
		  * TODO: perhaps a shorter expression? */
		UK_LEFT_CURLY_BRACKET	= 0x7B,

		/** Constant for the "|" key
		  * TODO: What is the correct expression? */
		UK_V_LINE	= 0x7C,
		/** Constant for the "}" key
		  * TODO: perhaps a shorter expression?  */
		UK_RIGHT_CURLY_BRACKET	= 0x7D,
		/** Constant for the "~" key */
		UK_TILDE	= 0x7E,

		UK_DELETE	= 0x7F,
		/* End of ASCII mapped keysyms */

		/** international keyboard syms */
		UK_WORLD_0	= 0xA0,
		UK_WORLD_1	= 0xA1,
		UK_WORLD_2	= 0xA2,
		UK_WORLD_3	= 0xA3,
		UK_WORLD_4	= 0xA4,
		UK_WORLD_5	= 0xA5,
		UK_WORLD_6	= 0xA6,
		UK_WORLD_7	= 0xA7,
		UK_WORLD_8	= 0xA8,
		UK_WORLD_9	= 0xA9,
		UK_WORLD_10	= 0xAA,
		UK_WORLD_11	= 0xAB,
		UK_WORLD_12	= 0xAC,
		UK_WORLD_13	= 0xAD,
		UK_WORLD_14	= 0xAE,
		UK_WORLD_15	= 0xAF,
		UK_WORLD_16	= 0xB0,
		UK_WORLD_17	= 0xB1,
		UK_WORLD_18	= 0xB2,
		UK_WORLD_19	= 0xB3,
		UK_WORLD_20	= 0xB4,
		UK_WORLD_21	= 0xB5,
		UK_WORLD_22	= 0xB6,
		UK_WORLD_23	= 0xB7,
		UK_WORLD_24	= 0xB8,
		UK_WORLD_25	= 0xB9,
		UK_WORLD_26	= 0xBA,
		UK_WORLD_27	= 0xBB,
		UK_WORLD_28	= 0xBC,
		UK_WORLD_29	= 0xBD,
		UK_WORLD_30	= 0xBE,
		UK_WORLD_31	= 0xBF,
		UK_WORLD_32	= 0xC0,
		UK_WORLD_33	= 0xC1,
		UK_WORLD_34	= 0xC2,
		UK_WORLD_35	= 0xC3,
		UK_WORLD_36	= 0xC4,
		UK_WORLD_37	= 0xC5,
		UK_WORLD_38	= 0xC6,
		UK_WORLD_39	= 0xC7,
		UK_WORLD_40	= 0xC8,
		UK_WORLD_41	= 0xC9,
		UK_WORLD_42	= 0xCA,
		UK_WORLD_43	= 0xCB,
		UK_WORLD_44	= 0xCC,
		UK_WORLD_45	= 0xCD,
		UK_WORLD_46	= 0xCE,
		UK_WORLD_47	= 0xCF,
		UK_WORLD_48	= 0xD0,
		UK_WORLD_49	= 0xD1,
		UK_WORLD_50	= 0xD2,
		UK_WORLD_51	= 0xD3,
		UK_WORLD_52	= 0xD4,
		UK_WORLD_53	= 0xD5,
		UK_WORLD_54	= 0xD6,
		UK_WORLD_55	= 0xD7,
		UK_WORLD_56	= 0xD8,
		UK_WORLD_57	= 0xD9,
		UK_WORLD_58	= 0xDA,
		UK_WORLD_59	= 0xDB,
		UK_WORLD_60	= 0xDC,
		UK_WORLD_61	= 0xDD,
		UK_WORLD_62	= 0xDE,
		UK_WORLD_63	= 0xDF,
		UK_WORLD_64	= 0xE0,
		UK_WORLD_65	= 0xE1,
		UK_WORLD_66	= 0xE2,
		UK_WORLD_67	= 0xE3,
		UK_WORLD_68	= 0xE4,
		UK_WORLD_69	= 0xE5,
		UK_WORLD_70	= 0xE6,
		UK_WORLD_71	= 0xE7,
		UK_WORLD_72	= 0xE8,
		UK_WORLD_73	= 0xE9,
		UK_WORLD_74	= 0xEA,
		UK_WORLD_75	= 0xEB,
		UK_WORLD_76	= 0xEC,
		UK_WORLD_77	= 0xED,
		UK_WORLD_78	= 0xEE,
		UK_WORLD_79	= 0xEF,
		UK_WORLD_80	= 0xF0,
		UK_WORLD_81	= 0xF1,
		UK_WORLD_82	= 0xF2,
		UK_WORLD_83	= 0xF3,
		UK_WORLD_84	= 0xF4,
		UK_WORLD_85	= 0xF5,
		UK_WORLD_86	= 0xF6,
		UK_WORLD_87	= 0xF7,
		UK_WORLD_88	= 0xF8,
		UK_WORLD_89	= 0xF9,
		UK_WORLD_90	= 0xFA,
		UK_WORLD_91	= 0xFB,
		UK_WORLD_92	= 0xFC,
		UK_WORLD_93	= 0xFD,
		UK_WORLD_94	= 0xFE,
		UK_WORLD_95	= 0xFF,
		/* End of SDL mapped keysyms */


		/* Numeric keypad */
		UK_KP0	= 0x0200,
		UK_KP1	= 0x0201,
		UK_KP2	= 0x0202,
		UK_KP3	= 0x0203,
		UK_KP4	= 0x0204,
		UK_KP5	= 0x0205,
		UK_KP6	= 0x0206,
		UK_KP7	= 0x0207,
		UK_KP8	= 0x0208,
		UK_KP9	= 0x0209,

		UK_KP_PERIOD	= 0x020A,
		UK_KP_DIVIDE	= 0x020B,
		UK_KP_MULTIPLY	= 0x020C,
		UK_KP_MINUS	= 0x020D,
		UK_KP_PLUS	= 0x020E,
		UK_KP_ENTER	= 0x020F,
		UK_KP_EQUALS	= 0x0210,

		UK_KP_UP	= 0x0211,
		UK_KP_DOWN	= 0x0212,
		UK_KP_RIGHT	= 0x0213,
		UK_KP_LEFT	= 0x0214,

		UK_KP_INSERT	= 0x0215,
		UK_KP_HOME	= 0x0216,
		UK_KP_END	= 0x0217,
		UK_KP_PAGEUP	= 0x0218,
		UK_KP_PAGEDOWN	= 0x0219,

		/** Arrows + Home/End pad */
		UK_UP	= 0x0230,
		UK_DOWN	= 0x0231,
		UK_RIGHT	= 0x0232,
		UK_LEFT	= 0x0233,

		UK_INSERT	= 0x0234,
		UK_HOME	= 0x0235,
		UK_END	= 0x0236,
		UK_PAGEUP	= 0x0237,
		UK_PAGEDOWN	= 0x0238,

		/** Function keys */
		UK_F1	= 0x0300,
		UK_F2	= 0x0301,
		UK_F3	= 0x0302,
		UK_F4	= 0x0303,
		UK_F5	= 0x0304,
		UK_F6	= 0x0305,
		UK_F7	= 0x0306,
		UK_F8	= 0x0307,
		UK_F9	= 0x0308,
		UK_F10	= 0x0309,
		UK_F11	= 0x030A,
		UK_F12	= 0x030B,
		UK_F13	= 0x030C,
		UK_F14	= 0x030D,
		UK_F15	= 0x030E,
		UK_F16	= 0x030F,
		UK_F17	= 0x0310,
		UK_F18	= 0x0311,
		UK_F19	= 0x0312,
		UK_F20	= 0x0313,
		UK_F21	= 0x0314,
		UK_F22	= 0x0315,
		UK_F23	= 0x0316,
		UK_F24	= 0x0317,

		/** Key state modifier keys */
		UK_NUMLOCK	= 0xF000,
		UK_CAPSLOCK	= 0xF002,
		UK_SCROLLOCK	= 0xF003,
		UK_RSHIFT	= 0xF004,
		UK_LSHIFT	= 0xF005,
		UK_RCTRL	= 0xF006,
		UK_LCTRL	= 0xF007,
		UK_RALT	= 0xF008,
		UK_LALT	= 0xF009,
		UK_RMETA	= 0xF00A,
		UK_LMETA	= 0xF00B,
		/** Left "Windows" key */
		UK_LSUPER	= 0xF00C,
		/** Right "Windows" key */
		UK_RSUPER	= 0xF00D,
		/** "Alt Gr" key */
		UK_MODE	= 0xF00E,
		UK_ALT_GRAPH	= 0xF00E,
		/** Multi-key compose key */
		UK_COMPOSE	= 0xF00F,

		/** Miscellaneous function keys */
		UK_HELP	= 0xF020,
		UK_PRINT	= 0xF021,
		UK_SYSREQ	= 0xF022,
		UK_BREAK	= 0xF023,
		UK_MENU	= 0xF024,
		/** Power MacUKeyCode_tosh power key */
		UK_POWER	= 0xF025,
		/** Constant for the Euro currency sign key. */
		UK_EURO	= 0xF026,
		UK_LAST = UK_EURO
	};
} // namespace UKey

typedef UKey::KeyCode UKeyCode_t;


namespace UMod {
	enum Modifier {
		NoModifier = 0x00000000,
		NoButton = NoModifier,

		Shift = 0x0001,
		Ctrl = 0x0002,
		Alt = 0x0004,
		Meta = 0x0008,
		Super = 0x0010,

		Num = 0x0020,
		Caps = 0x0040,
		AltGraph = 0x0080,

		/** mouse buttons are modifiers, too */
		MouseButton1 = 0x0100,
		MouseButton2 = 0x0200,
		MouseButton3 = 0x0400,
		MouseButton4 = 0x0800,
		MouseButton5 = 0x1000,
		LeftButton = MouseButton1,
		MiddleButton = MouseButton2,
		RightButton = MouseButton3,

		KeyboardModifierMask = 0x00ff,
		MouseModifierMask  = 0xff00,
		MouseButtonMask = MouseModifierMask
	};
} // namespace UMod


typedef UMod::Modifier UMod_t;

} // namespace ufo

#endif // UKEYSYM_HPP
