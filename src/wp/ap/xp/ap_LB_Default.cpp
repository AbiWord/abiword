/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


// ********************************************************************************
// ********************************************************************************
// *** THIS FILE DEFINES THE DEFAULT KEYBOARD AND MOUSE BINDINGS FOR AbiWord 1. ***
// *** To define bindings for other emulations, clone this file and change the  ***
// *** various settings.  See ap_LoadBindings.cpp for more information.         ***
// ********************************************************************************
// ********************************************************************************

#include "ut_assert.h"
#include "ut_types.h"
#include "ev_EditBits.h"
#include "ev_EditBinding.h"
#include "ev_EditMethod.h"
#include "ev_NamedVirtualKey.h"
#include "ap_LoadBindings.h"
#include "ap_LB_Default.h"

// NOTE: on Win32 we cannot get ALT-TAB (but we can get ALT-F4 :-)

#define NrElements(a)	((sizeof(a)/sizeof(a[0])))

#define _S		| EV_EMS_SHIFT
#define _C		| EV_EMS_CONTROL
#define _A		| EV_EMS_ALT

/*****************************************************************
******************************************************************
** load top-level (non-prefixed) builtin bindings for the mouse
******************************************************************
*****************************************************************/

static struct ap_bs_Mouse s_MouseTable[] =
{
//  {   button w/modifiers,     { click				doubleclick		drag			release		}},
	{	EV_EMB_BUTTON1,			{ "singleClick",	"doubleClick",	"dragToXY",		"endDrag"	}},
	{	EV_EMB_BUTTON1 _S,		{ "extSelToXY",		"",				"dragToXY",		"endDrag"	}},
	{	EV_EMB_BUTTON1 _C,		{ "selectWord",		"",				"",				"endDrag"	}},
	{	EV_EMB_BUTTON1 _A,		{ "",				"",				"",				"endDrag"	}},
	{	EV_EMB_BUTTON1 _S _C,	{ "",				"",				"",				"endDrag"	}},
	{	EV_EMB_BUTTON1 _S _A,	{ "",				"",				"",				"endDrag"	}},
	{	EV_EMB_BUTTON1 _C _A,	{ "",				"",				"",				"endDrag"	}},
	{	EV_EMB_BUTTON1 _S _C _A,{ "",				"",				"",				"endDrag"	}},

// TODO fill in missing bindings for mouse-1
// TODO add bindings for mouse-2, mouse-3, ...
// TODO remove the test dump binding.

#if defined(PT_TEST) || defined(FMT_TEST) || defined(UT_TEST)
	{	EV_EMB_BUTTON3,			{ "Test_Dump",		"",				"",				""			}},
#endif
};


/*****************************************************************
******************************************************************
** load top-level (non-prefixed) builtin bindings for the NamedVirtualKeys
******************************************************************
*****************************************************************/

static struct ap_bs_NVK s_NVKTable[] =
{
//	{nvk,				{ none,					_S,					_C,				_S_C,		
//  					  _A,					_A_S,				_A_C,			_A_C_S				}},
	{EV_NVK_BACKSPACE,	{ "delLeft", 			"delLeft",			"delBOW",		"",			
						  "",					"",					"",				""					}},
	{EV_NVK_SPACE,		{ "insertSpace",		"insertSpace",		"togglePlain",	"insertNBSpace",
						  "",					"",					"",				""					}},
	{EV_NVK_TAB,		{ "insertTab",			"",					"cycleWindows",	"cycleWindowsBck",
						  "",					"",					"",				""					}},
	{EV_NVK_RETURN,		{ "insertParagraphBreak", "insertLineBreak", "insertPageBreak", "insertColumnBreak",
						  "insertSectionBreak",	"",					"",				""					}},
//	{EV_NVK_ESCAPE,		{ "",					"",					"",				"",
//						  "",					"",					"",				""					}},
	{EV_NVK_PAGEUP,		{ "scrollPageUp",		"extSelPageUp",		"",				"",
						  "",					"",					"",				""					}},
	{EV_NVK_PAGEDOWN,	{ "scrollPageDown",		"extSelPageDown",	"",				"",
						  "",					"",					"",				""					}},
	{EV_NVK_END,		{ "warpInsPtEOL",		"extSelEOL",		"warpInsPtEOD",	"extSelEOD",
						  "",					"",					"",				""					}},
	{EV_NVK_HOME,		{ "warpInsPtBOL",		"extSelBOL",		"warpInsPtBOD",	"extSelBOD",
						  "",					"",					"",				""					}},
	{EV_NVK_LEFT,		{ "warpInsPtLeft",		"extSelLeft",		"warpInsPtBOW",	"extSelBOW",
						  "",					"",					"",				""					}},
	{EV_NVK_UP,			{ "warpInsPtPrevLine",	"extSelPrevLine",	"warpInsPtBOB",	"extSelBOB",
						  "",					"",					"",				""					}},
	{EV_NVK_RIGHT,		{ "warpInsPtRight",		"extSelRight",		"warpInsPtEOW",	"extSelEOW",
						  "",					"",					"",				""					}},
	{EV_NVK_DOWN,		{ "warpInsPtNextLine",	"extSelNextLine",	"warpInsPtEOB",	"extSelEOB",
						  "",					"",					"",				""					}},
//	{EV_NVK_INSERT,		{ "",					"",					"",				"",
//						  "",					"",					"",				""					}},
	{EV_NVK_DELETE,		{ "delRight",			"delRight",			"delEOW",		"",
						  "",					"",					"",				""					}},
//	{EV_NVK_HELP,		{ "",					"",					"",				"",
//						  "",					"",					"",				""					}},
//	{EV_NVK_F1,			{ "",					"",					"",				"",
//						  "",					"",					"",				""					}},
//	{EV_NVK_F2,			{ "",					"",					"",				"",
//						  "",					"",					"",				""					}},
	{EV_NVK_F3,			{ "findAgain",			"",					"",				"",
						  "",					"",					"",				""					}},
	{EV_NVK_F4,			{ "",					"",					"closeWindow",	"",
						  "querySaveAndExit",	"",					"",				""					}},
//	{EV_NVK_F5,			{ "",					"",					"",				"",
//						  "",					"",					"",				""					}},
//	{EV_NVK_F6,			{ "",					"",					"",				"",
//						  "",					"",					"",				""					}},
//	{EV_NVK_F7,			{ "",					"",					"",				"",
//						  "",					"",					"",				""					}},
//	{EV_NVK_F8,			{ "",					"",					"",				"",
//						  "",					"",					"",				""					}},
//	{EV_NVK_F9,			{ "",					"",					"",				"",
//						  "",					"",					"",				""					}},
//	{EV_NVK_F10,		{ "",					"",					"",				"",
//						  "",					"",					"",				""					}},
//	{EV_NVK_F11,		{ "",					"",					"",				"",
//						  "",					"",					"",				""					}},
//	{EV_NVK_F12,		{ "",					"",					"",				"",
//						  "",					"",					"",				""					}},

// 	{EV_NVK_F13,		{
// 	{EV_NVK_F14,		{
// 	{EV_NVK_F15,		{
// 	{EV_NVK_F16,		{
// 	{EV_NVK_F17,		{
// 	{EV_NVK_F18,		{
// 	{EV_NVK_F19,		{
// 	{EV_NVK_F20,		{
// 	{EV_NVK_F21,		{
// 	{EV_NVK_F22,		{
// 	{EV_NVK_F23,		{
// 	{EV_NVK_F24,		{
// 	{EV_NVK_F25,		{
// 	{EV_NVK_F26,		{
// 	{EV_NVK_F27,		{
// 	{EV_NVK_F28,		{
// 	{EV_NVK_F29,		{
// 	{EV_NVK_F30,		{
// 	{EV_NVK_F31,		{
// 	{EV_NVK_F32,		{
// 	{EV_NVK_F33,		{
// 	{EV_NVK_F34,		{
// 	{EV_NVK_F35,		{

};

/*****************************************************************
******************************************************************
** load top-level prefixed builtin bindings for the NamedVirtualKeys
******************************************************************
*****************************************************************/

static struct ap_bs_NVK_Prefix s_NVKTable_P[] =
{
//	{nvk,						{ none,					_S,					_C,				_S_C,		
//  							  _A,					_A_S,				_A_C,			_A_C_S	}},

	{EV_NVK_DEAD_GRAVE,			{ "deadgrave",			"deadgrave",		"",	"", "",	"",	"",	""	}},
	{EV_NVK_DEAD_ACUTE,			{ "deadacute",			"deadacute",		"",	"", "",	"",	"",	""	}},
	{EV_NVK_DEAD_CIRCUMFLEX,	{ "deadcircumflex",		"deadcircumflex",	"",	"", "",	"",	"",	""	}},
	{EV_NVK_DEAD_TILDE,			{ "deadtilde",			"deadtilde",		"",	"", "", "", "",	""	}},
	{EV_NVK_DEAD_MACRON,		{ "deadmacron",			"deadmacron",		"",	"", "", "",	"", ""	}},
	{EV_NVK_DEAD_BREVE,			{ "deadbreve",			"deadbreve",		"",	"", "", "",	"",	""	}},
	{EV_NVK_DEAD_ABOVEDOT,		{ "deadabovedot",		"deadabovedot",		"",	"", "",	"",	"",	""	}},
	{EV_NVK_DEAD_DIAERESIS,		{ "deaddiaeresis",		"deaddiaeresis",	"",	"", "",	"",	"",	""	}},
	{EV_NVK_DEAD_DOUBLEACUTE,	{ "deaddoubleacute",	"deaddoubleacute",	"",	"", "",	"",	"",	""	}},
	{EV_NVK_DEAD_CARON,			{ "deadcaron",			"deadcaron",		"",	"", "", "", "", ""	}},
	{EV_NVK_DEAD_CEDILLA,		{ "deadcedilla",		"deadcedilla",		"",	"", "", "",	"",	""	}},
	{EV_NVK_DEAD_OGONEK,		{ "deadogonek",			"deadogonek",		"",	"", "",	"",	"",	""	}},
};

/*****************************************************************
******************************************************************
** load top-level (non-prefixed) builtin bindings for the non-nvk
** (character keys).  note that SHIFT-state is implicit in the
** character value and is not included in the table.  note that
** we do not include the ASCII control characters (\x00 -- \x1f
** and others) since we don't map keystrokes into them.
******************************************************************
*****************************************************************/

static struct ap_bs_Char s_CharTable[] =
{
//	{char, /* desc   */ { none,					_C,					_A,				_A_C				}},
	{0x21, /* !      */ { "insertData",			"",					"",				""					}},
	{0x22, /* "      */ { "insertData",			"",					"",				""					}},
	{0x23, /* #      */ { "insertData",			"",					"",				""					}},
	{0x24, /* $      */ { "insertData",			"",					"",				""					}},
	{0x25, /* %      */ { "insertData",			"",					"",				""					}},
	{0x26, /* &      */ { "insertData",			"",					"",				""					}},
	{0x27, /* '      */ { "insertData",			"",					"",				""					}},
	{0x28, /* (      */ { "insertData",			"",					"",				""					}},
	{0x29, /* )      */ { "insertData",			"",					"",				""					}},
	{0x2a, /* *      */ { "insertData",			"",					"",				""					}},
	{0x2b, /* +      */ { "insertData",			"",					"",				""					}},
	{0x2c, /* ,      */ { "insertData",			"",					"",				""					}},
	{0x2d, /* -      */ { "insertData",			"",					"",				""					}},
	{0x2e, /* .      */ { "insertData",			"",					"",				""					}},
	{0x2f, /* /      */ { "insertData",			"",					"",				""					}},
	{0x30, /* 0      */ { "insertData",			"",					"",				""					}},
	{0x31, /* 1      */ { "insertData",			"singleSpace",		"",				""					}},
	{0x32, /* 2      */ { "insertData",			"doubleSpace",		"",				""					}},
	{0x33, /* 3      */ { "insertData",			"",					"",				""					}},
	{0x34, /* 4      */ { "insertData",			"",					"",				""					}},
	{0x35, /* 5      */ { "insertData",			"middleSpace",		"",				""					}},
	{0x36, /* 6      */ { "insertData",			"",					"",				""					}},
	{0x37, /* 7      */ { "insertData",			"",					"",				""					}},
	{0x38, /* 8      */ { "insertData",			"",					"",				""					}},
	{0x39, /* 9      */ { "insertData",			"",					"",				""					}},
	{0x3a, /* :      */ { "insertData",			"",					"",				""					}},
	{0x3b, /* ;      */ { "insertData",			"",					"",				""					}},
	{0x3c, /* <      */ { "insertData",			"",					"",				""					}},
	{0x3d, /* =      */ { "insertData",			"",					"",				""					}},
	{0x3e, /* >      */ { "insertData",			"",					"",				""					}},
	{0x3f, /* ?      */ { "insertData",			"",					"",				""					}},
	{0x40, /* @      */ { "insertData",			"",					"",				""					}},
	{0x41, /* A      */ { "insertData",			"selectAll",		"",				""					}},
	{0x42, /* B      */ { "insertData",			"toggleBold",		"",				""					}},
	{0x43, /* C      */ { "insertData",			"copy",				"",				""					}},
	{0x44, /* D      */ { "insertData",			"dlgFont",			"",				""					}},
	{0x45, /* E      */ { "insertData",			"alignCenter",		"",				""					}},
	{0x46, /* F      */ { "insertData",			"find",				"",				""					}},
	{0x47, /* G      */ { "insertData",			"go",				"",				""					}},
	{0x48, /* H      */ { "insertData",			"replace",			"",				""					}},
	{0x49, /* I      */ { "insertData",			"toggleItalic",		"",				""					}},
	{0x4a, /* J      */ { "insertData",			"alignJustify",		"",				""					}},
	{0x4b, /* K      */ { "insertData",			"",					"",				""					}},
	{0x4c, /* L      */ { "insertData",			"alignLeft",		"",				""					}},
	{0x4d, /* M      */ { "insertData",			"",					"",				""					}},
	{0x4e, /* N      */ { "insertData",			"fileNew",			"",				""					}},
	{0x4f, /* O      */ { "insertData",			"fileOpen",			"",				""					}},
	{0x50, /* P      */ { "insertData",			"print",			"",				""					}},
	{0x51, /* Q      */ { "insertData",			"",					"",				""					}},
	{0x52, /* R      */ { "insertData",			"alignRight",		"",				""					}},
	{0x53, /* S      */ { "insertData",			"fileSave",			"",				""					}},
	{0x54, /* T      */ { "insertData",			"",					"",				""					}},
	{0x55, /* U      */ { "insertData",			"toggleUline",		"",				""					}},
	{0x56, /* V      */ { "insertData",			"paste",			"",				""					}},
	{0x57, /* W      */ { "insertData",			"closeWindow",		"",				""					}},
	{0x58, /* X      */ { "insertData",			"cut",				"",				""					}},
	{0x59, /* Y      */ { "insertData",			"redo",				"",				""					}},
	{0x5a, /* Z      */ { "insertData",			"undo",				"",				""					}},
	{0x5b, /* [      */ { "insertData",			"",					"",				""					}},
	{0x5c, /* \      */ { "insertData",			"",					"",				""					}},
	{0x5d, /* ]      */ { "insertData",			"",					"",				""					}},
	{0x5e, /* ^      */ { "insertData",			"",					"",				""					}},
	{0x5f, /* -      */ { "insertData",			"",					"",				""					}},
	{0x60, /* `      */ { "insertData",			"",					"",				""					}},
	{0x61, /* a      */ { "insertData",			"selectAll",		"",				""					}},
	{0x62, /* b      */ { "insertData",			"toggleBold",		"",				""					}},
	{0x63, /* c      */ { "insertData",			"copy",				"",				""					}},
	{0x64, /* d      */ { "insertData",			"dlgFont",			"",				""					}},
	{0x65, /* e      */ { "insertData",			"alignCenter",		"",				""					}},
	{0x66, /* f      */ { "insertData",			"find",				"",				""					}},
	{0x67, /* g      */ { "insertData",			"go",				"",				""					}},
	{0x68, /* h      */ { "insertData",			"replace",			"",				""					}},
	{0x69, /* i      */ { "insertData",			"toggleItalic",		"",				""					}},
	{0x6a, /* j      */ { "insertData",			"alignJustify",		"",				""					}},
	{0x6b, /* k      */ { "insertData",			"",					"",				""					}},
	{0x6c, /* l      */ { "insertData",			"alignLeft",		"",				""					}},
	{0x6d, /* m      */ { "insertData",			"",					"",				""					}},
	{0x6e, /* n      */ { "insertData",			"fileNew",			"",				""					}},
	{0x6f, /* o      */ { "insertData",			"fileOpen",			"",				""					}},
	{0x70, /* p      */ { "insertData",			"print",			"",				""					}},
	{0x71, /* q      */ { "insertData",			"",					"",				""					}},
	{0x72, /* r      */ { "insertData",			"alignRight",		"",				""					}},
	{0x73, /* s      */ { "insertData",			"fileSave",			"",				""					}},
	{0x74, /* t      */ { "insertData",			"",					"",				""					}},
	{0x75, /* u      */ { "insertData",			"toggleUline",		"",				""					}},
	{0x76, /* v      */ { "insertData",			"paste",			"",				""					}},
	{0x77, /* w      */ { "insertData",			"closeWindow",		"",				""					}},
	{0x78, /* x      */ { "insertData",			"cut",				"",				""					}},
	{0x79, /* y      */ { "insertData",			"redo",				"",				""					}},
	{0x7a, /* z      */ { "insertData",			"undo",				"",				""					}},
	{0x7b, /* {      */ { "insertData",			"",					"",				""					}},
	{0x7c, /* |      */ { "insertData",			"",					"",				""					}},
	{0x7d, /* }      */ { "insertData",			"",					"",				""					}},
	{0x7e, /* ~      */ { "insertData",			"",					"",				""					}},

//	Here are bindings for the portion of Latin-1 with the high-bit set.
//	I'm taking the list from /usr/include/X11/keysymdef.h, so I think
//	that the set is reasonably complete.
//	
//	I don't know how to test these on my EnUS system with my EnUS keyboard,
//	but I think they will work.
//
//	TODO For now I just bound them all to "insertData" which will cause
//	TODO the character to be inserted into the document.  I'm wondering
//	TODO if some of these (e.g., paragraph and section) should be bound
//	TODO to some other functions -- but rather than try guessing, let's
//	TODO wait and let some of our European friends comment.

//	{char, /* desc           */ { none,				_C,					_A,				_A_C				}},

	{0xa0, /* nbs            */ { "insertNBSpace",	"",					"",				""					}},
	{0xa1, /* exclamdown     */ { "insertData",		"",					"",				""					}},
	{0xa2, /* cent           */ { "insertData",		"",					"",				""					}},
	{0xa3, /* sterling       */ { "insertData",		"",					"",				""					}},
	{0xa4, /* currency       */ { "insertData",		"",					"",				""					}},
	{0xa5, /* yen            */ { "insertData",		"",					"",				""					}},
	{0xa6, /* brokenbar      */ { "insertData",		"",					"",				""					}},
	{0xa7, /* section        */ { "insertData",		"",					"",				""					}},
	{0xa8, /* diaeresis      */ { "insertData",		"",					"",				""					}},
	{0xa9, /* copyright      */ { "insertData",		"",					"",				""					}},
	{0xaa, /* ordfeminine    */ { "insertData",		"",					"",				""					}},
	{0xab, /* guillemotleft  */ { "insertData",		"",					"",				""					}},
	{0xac, /* notsign        */ { "insertData",		"",					"",				""					}},
	{0xad, /* hyphen         */ { "insertData",		"",					"",				""					}},
	{0xae, /* registered     */ { "insertData",		"",					"",				""					}},
	{0xaf, /* macron         */ { "insertData",		"",					"",				""					}},
	{0xb0, /* degree         */ { "insertData",		"",					"",				""					}},
	{0xb1, /* plusminus      */ { "insertData",		"",					"",				""					}},
	{0xb2, /* twosuperior    */ { "insertData",		"",					"",				""					}},
	{0xb3, /* threesuperior  */ { "insertData",		"",					"",				""					}},
	{0xb4, /* acute          */ { "insertData",		"",					"",				""					}},
	{0xb5, /* mu             */ { "insertData",		"",					"",				""					}},
	{0xb6, /* paragraph      */ { "insertData",		"",					"",				""					}},
	{0xb7, /* periodcentered */ { "insertData",		"",					"",				""					}},
	{0xb8, /* cedilla        */ { "insertData",		"",					"",				""					}},
	{0xb9, /* onesuperior    */ { "insertData",		"",					"",				""					}},
	{0xba, /* masculine      */ { "insertData",		"",					"",				""					}},
	{0xbb, /* guillemotright */ { "insertData",		"",					"",				""					}},
	{0xbc, /* onequarter     */ { "insertData",		"",					"",				""					}},
	{0xbd, /* onehalf        */ { "insertData",		"",					"",				""					}},
	{0xbe, /* threequarters  */ { "insertData",		"",					"",				""					}},
	{0xbf, /* questiondown   */ { "insertData",		"",					"",				""					}},
	{0xc0, /* Agrave         */ { "insertData",		"",					"",				""					}},
	{0xc1, /* Aacute         */ { "insertData",		"",					"",				""					}},
	{0xc2, /* Acircumflex    */ { "insertData",		"",					"",				""					}},
	{0xc3, /* Atilde         */ { "insertData",		"",					"",				""					}},
	{0xc4, /* Adiaeresis     */ { "insertData",		"",					"",				""					}},
	{0xc5, /* Aring          */ { "insertData",		"",					"",				""					}},
	{0xc6, /* AE             */ { "insertData",		"",					"",				""					}},
	{0xc7, /* Ccedilla       */ { "insertData",		"",					"",				""					}},
	{0xc8, /* Egrave         */ { "insertData",		"",					"",				""					}},
	{0xc9, /* Eacute         */ { "insertData",		"",					"",				""					}},
	{0xca, /* Ecircumflex    */ { "insertData",		"",					"",				""					}},
	{0xcb, /* Ediaeresis     */ { "insertData",		"",					"",				""					}},
	{0xcc, /* Igrave         */ { "insertData",		"",					"",				""					}},
	{0xcd, /* Iacute         */ { "insertData",		"",					"",				""					}},
	{0xce, /* Icircumflex    */ { "insertData",		"",					"",				""					}},
	{0xcf, /* Idiaeresis     */ { "insertData",		"",					"",				""					}},
	{0xd0, /* ETH            */ { "insertData",		"",					"",				""					}},
	{0xd1, /* Ntilde         */ { "insertData",		"",					"",				""					}},
	{0xd2, /* Ograve         */ { "insertData",		"",					"",				""					}},
	{0xd3, /* Oacute         */ { "insertData",		"",					"",				""					}},
	{0xd4, /* Ocircumflex    */ { "insertData",		"",					"",				""					}},
	{0xd5, /* Otilde         */ { "insertData",		"",					"",				""					}},
	{0xd6, /* Odiaeresis     */ { "insertData",		"",					"",				""					}},
	{0xd7, /* multiply       */ { "insertData",		"",					"",				""					}},
	{0xd8, /* Ooblique       */ { "insertData",		"",					"",				""					}},
	{0xd9, /* Ugrave         */ { "insertData",		"",					"",				""					}},
	{0xda, /* Uacute         */ { "insertData",		"",					"",				""					}},
	{0xdb, /* Ucircumflex    */ { "insertData",		"",					"",				""					}},
	{0xdc, /* Udiaeresis     */ { "insertData",		"",					"",				""					}},
	{0xdd, /* Yacute         */ { "insertData",		"",					"",				""					}},
	{0xde, /* THORN          */ { "insertData",		"",					"",				""					}},
	{0xdf, /* ssharp         */ { "insertData",		"",					"",				""					}},
	{0xe0, /* agrave         */ { "insertData",		"",					"",				""					}},
	{0xe1, /* aacute         */ { "insertData",		"",					"",				""					}},
	{0xe2, /* acircumflex    */ { "insertData",		"",					"",				""					}},
	{0xe3, /* atilde         */ { "insertData",		"",					"",				""					}},
	{0xe4, /* adiaeresis     */ { "insertData",		"",					"",				""					}},
	{0xe5, /* aring          */ { "insertData",		"",					"",				""					}},
	{0xe6, /* ae             */ { "insertData",		"",					"",				""					}},
	{0xe7, /* ccedilla       */ { "insertData",		"",					"",				""					}},
	{0xe8, /* egrave         */ { "insertData",		"",					"",				""					}},
	{0xe9, /* eacute         */ { "insertData",		"",					"",				""					}},
	{0xea, /* ecircumflex    */ { "insertData",		"",					"",				""					}},
	{0xeb, /* ediaeresis     */ { "insertData",		"",					"",				""					}},
	{0xec, /* igrave         */ { "insertData",		"",					"",				""					}},
	{0xed, /* iacute         */ { "insertData",		"",					"",				""					}},
	{0xee, /* icircumflex    */ { "insertData",		"",					"",				""					}},
	{0xef, /* idiaeresis     */ { "insertData",		"",					"",				""					}},
	{0xf0, /* eth            */ { "insertData",		"",					"",				""					}},
	{0xf1, /* ntilde         */ { "insertData",		"",					"",				""					}},
	{0xf2, /* ograve         */ { "insertData",		"",					"",				""					}},
	{0xf3, /* oacute         */ { "insertData",		"",					"",				""					}},
	{0xf4, /* ocircumflex    */ { "insertData",		"",					"",				""					}},
	{0xf5, /* otilde         */ { "insertData",		"",					"",				""					}},
	{0xf6, /* odiaeresis     */ { "insertData",		"",					"",				""					}},
	{0xf7, /* division       */ { "insertData",		"",					"",				""					}},
	{0xf8, /* oslash         */ { "insertData",		"",					"",				""					}},
	{0xf9, /* ugrave         */ { "insertData",		"",					"",				""					}},
	{0xfa, /* uacute         */ { "insertData",		"",					"",				""					}},
	{0xfb, /* ucircumflex    */ { "insertData",		"",					"",				""					}},
	{0xfc, /* udiaeresis     */ { "insertData",		"",					"",				""					}},
	{0xfd, /* yacute         */ { "insertData",		"",					"",				""					}},
	{0xfe, /* thorn          */ { "insertData",		"",					"",				""					}},
	{0xff, /* ydiaeresis     */ { "insertData",		"",					"",				""					}},
	
};

/*****************************************************************
******************************************************************
** put it all together and load the default bindings.
******************************************************************
*****************************************************************/

UT_Bool ap_LoadBindings_Default(AP_BindingSet * pThis, EV_EditBindingMap * pebm)
{
	pThis->_loadMouse(pebm,s_MouseTable,NrElements(s_MouseTable));
	pThis->_loadNVK(pebm,s_NVKTable,NrElements(s_NVKTable),s_NVKTable_P,NrElements(s_NVKTable_P));
	pThis->_loadChar(pebm,s_CharTable,NrElements(s_CharTable),NULL,0);

	return UT_TRUE;
}
