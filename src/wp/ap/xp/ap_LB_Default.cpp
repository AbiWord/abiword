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

// TODO We need something here to map Esc (when used as a prefix)
// TODO to ALT so that we don't have to load both bindings (ie.
// TODO ESC-f and ALT-f).

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

struct _iMouse
{
	EV_EditBits			m_eb;			// sans emo
	const char *		m_szMethod[EV_COUNT_EMO];
};

static struct _iMouse s_MouseTable[] =
{
//  {   button w/modifiers,     { click				doubleclick		drag			release		}},
	{	EV_EMB_BUTTON1,			{ "singleClick",	"doubleClick",	"extSelToXY",	""			}},
	{	EV_EMB_BUTTON1 _S,		{ "extSelToXY",		"",				"extSelToXY",	""			}},
	{	EV_EMB_BUTTON1 _C,		{ "selectWord",		"",				"",				""			}},
//	{	EV_EMB_BUTTON1 _A,		{ "",				"",				"",				""			}},
//	{	EV_EMB_BUTTON1 _S _C,	{ "",				"",				"",				""			}},
//	{	EV_EMB_BUTTON1 _S _A,	{ "",				"",				"",				""			}},
//	{	EV_EMB_BUTTON1 _C _A,	{ "",				"",				"",				""			}},
//	{	EV_EMB_BUTTON1 _S _C _A,{ "",				"",				"",				""			}},

// TODO fill in missing bindings for mouse-1
// TODO add bindings for mouse-2, mouse-3, ...
// TODO remove the test dump binding.

	{	EV_EMB_BUTTON3,			{ "Test_Dump",		"",				"",				""			}},
};

static void s_loadMouse(EV_EditBindingMap * pebm)
{
	int k, m;
	int kLimit = NrElements(s_MouseTable);

	for (k=0; k<kLimit; k++)
		for (m=0; m<EV_COUNT_EMO; m++)
			if (s_MouseTable[k].m_szMethod[m] && *s_MouseTable[k].m_szMethod[m])
			{
				EV_EditMouseOp emo = EV_EMO_FromNumber(m+1);
				pebm->setBinding(s_MouseTable[k].m_eb|emo,s_MouseTable[k].m_szMethod[m]);
			}
}

/*****************************************************************
******************************************************************
** load top-level (non-prefixed) builtin bindings for the NamedVirtualKeys
******************************************************************
*****************************************************************/

struct _iNVK
{
	EV_EditBits			m_eb;			// sans ems
	const char *		m_szMethod[EV_COUNT_EMS];
};

// TODO finish filling out this table.

static struct _iNVK s_NVKTable[] =
{
//	{nvk,				{ none,					_S,					_C,				_S_C,		
//  					  _A,					_A_S,				_A_C,			_A_C_S				}},
	{EV_NVK_BACKSPACE,	{ "delLeft", 			"",					"delBOW",		"",			
						  "",					"",					"",				""					}},
	{EV_NVK_TAB,		{ "insertTab",			"",					"cycleWindows",	"cycleWindowsBck",
						  "",					"",					"",				""					}},
	{EV_NVK_RETURN,		{ "insertParagraphBreak", "insertLineBreak", "insertPageBreak", "insertColumnBreak",
						  "",					"",					"",				""					}},
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
	{EV_NVK_DELETE,		{ "delRight",			"",					"delEOW",		"",
						  "",					"",					"",				""					}},
	{EV_NVK_HELP,		{ "",					"",					"",				"",
						  "",					"",					"",				""					}},
// TODO the bindings are plain F1-F11 are just for testing.
// TODO remove them.
	{EV_NVK_F1,			{ "insFmtBold",			"",					"",				"",
						  "",					"",					"",				""					}},
	{EV_NVK_F2,			{ "insFmtItalic",		"",					"",				"",
						  "",					"",					"",				""					}},
	{EV_NVK_F3,			{ "insFmtUline",		"",					"",				"",
						  "",					"",					"",				""					}},
	{EV_NVK_F4,			{ "insFmtStrike",		"",					"closeWindow",	"",
						  "querySaveAndExit",	"",					"",				""					}},
	{EV_NVK_F5,			{ "insFmtSize08",		"",					"",				"",
						  "",					"",					"",				""					}},
	{EV_NVK_F6,			{ "insFmtSize10",		"",					"",				"",
						  "",					"",					"",				""					}},
	{EV_NVK_F7,			{ "insFmtSize12",		"",					"",				"",
						  "",					"",					"",				""					}},
	{EV_NVK_F8,			{ "insFmtSize14",		"",					"",				"",
						  "",					"",					"",				""					}},
	{EV_NVK_F9,			{ "insFmtColorRed",		"",					"",				"",
						  "",					"",					"",				""					}},
	{EV_NVK_F10,		{ "insFmtColorGreen",	"",					"",				"",
						  "",					"",					"",				""					}},
	{EV_NVK_F11,		{ "insFmtColorBlue",	"",					"",				"",
						  "",					"",					"",				""					}},
	{EV_NVK_F12,		{ "",					"",					"",				"",
						  "",					"",					"",				""					}},

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

static void s_loadNVK(EV_EditBindingMap * pebm)
{
	int k, m;
	int kLimit = NrElements(s_NVKTable);

	for (k=0; k<kLimit; k++)
		for (m=0; m<EV_COUNT_EMS; m++)
			if (s_NVKTable[k].m_szMethod[m] && *s_NVKTable[k].m_szMethod[m])
			{
				EV_EditModifierState ems = EV_EMS_FromNumber(m);
				pebm->setBinding(EV_EKP_PRESS|s_NVKTable[k].m_eb|ems,s_NVKTable[k].m_szMethod[m]);
			}
}

/*****************************************************************
******************************************************************
** load top-level (non-prefixed) builtin bindings for the non-nvk
** (character keys).  note that SHIFT-state is implicit in the
** character value and is not included in the table.  note that
** we do not include the ASCII control characters (\x00 -- \x1f
** and others) since we don't map keystrokes into them.
******************************************************************
*****************************************************************/

struct _iChar
{
	EV_EditBits			m_eb;			// sans ems & shift
	const char *		m_szMethod[EV_COUNT_EMS_NoShift];
};

// TODO finish filling out this table.

static struct _iChar s_CharTable[] =
{
//	{char, /* desc   */ { none,					_C,					_A,				_A_C				}},
	{0x20, /* space  */ { "insertData",			"",					"",				""					}},
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
	{0x31, /* 1      */ { "insertData",			"",					"",				""					}},
	{0x32, /* 2      */ { "insertData",			"",					"",				""					}},
	{0x33, /* 3      */ { "insertData",			"",					"",				""					}},
	{0x34, /* 4      */ { "insertData",			"",					"",				""					}},
	{0x35, /* 5      */ { "insertData",			"",					"",				""					}},
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
	{0x42, /* B      */ { "insertData",			"insFmtBold",		"",				""					}},
	{0x43, /* C      */ { "insertData",			"copy",				"",				""					}},
	{0x44, /* D      */ { "insertData",			"",					"",				""					}},
	{0x45, /* E      */ { "insertData",			"",					"",				""					}},
	{0x46, /* F      */ { "insertData",			"",					"",				""					}},
	{0x47, /* G      */ { "insertData",			"",					"",				""					}},
	{0x48, /* H      */ { "insertData",			"",					"",				""					}},
	{0x49, /* I      */ { "insertData",			"insFmtItalic",		"",				""					}},
	{0x4a, /* J      */ { "insertData",			"",					"",				""					}},
	{0x4b, /* K      */ { "insertData",			"",					"",				""					}},
	{0x4c, /* L      */ { "insertData",			"",					"",				""					}},
	{0x4d, /* M      */ { "insertData",			"",					"",				""					}},
	{0x4e, /* N      */ { "insertData",			"fileNew",			"",				""					}},
	{0x4f, /* O      */ { "insertData",			"fileOpen",			"",				""					}},
	{0x50, /* P      */ { "insertData",			"print",			"",				""					}},
	{0x51, /* Q      */ { "insertData",			"",					"",				""					}},
	{0x52, /* R      */ { "insertData",			"",					"",				""					}},
	{0x53, /* S      */ { "insertData",			"fileSave",			"",				""					}},
	{0x54, /* T      */ { "insertData",			"",					"",				""					}},
	{0x55, /* U      */ { "insertData",			"insFmtUline",		"",				""					}},
	{0x56, /* V      */ { "insertData",			"paste",			"",				""					}},
	{0x57, /* W      */ { "insertData",			"",					"",				""					}},
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
	{0x62, /* b      */ { "insertData",			"insFmtBold",		"",				""					}},
	{0x63, /* c      */ { "insertData",			"copy",				"",				""					}},
	{0x64, /* d      */ { "insertData",			"",					"",				""					}},
	{0x65, /* e      */ { "insertData",			"",					"",				""					}},
	{0x66, /* f      */ { "insertData",			"",					"",				""					}},
	{0x67, /* g      */ { "insertData",			"",					"",				""					}},
	{0x68, /* h      */ { "insertData",			"",					"",				""					}},
	{0x69, /* i      */ { "insertData",			"insFmtItalic",		"",				""					}},
	{0x6a, /* j      */ { "insertData",			"",					"",				""					}},
	{0x6b, /* k      */ { "insertData",			"",					"",				""					}},
	{0x6c, /* l      */ { "insertData",			"",					"",				""					}},
	{0x6d, /* m      */ { "insertData",			"",					"",				""					}},
	{0x6e, /* n      */ { "insertData",			"fileNew",			"",				""					}},
	{0x6f, /* o      */ { "insertData",			"fileOpen",			"",				""					}},
	{0x70, /* p      */ { "insertData",			"print",			"",				""					}},
	{0x71, /* q      */ { "insertData",			"",					"",				""					}},
	{0x72, /* r      */ { "insertData",			"",					"",				""					}},
	{0x73, /* s      */ { "insertData",			"fileSave",			"",				""					}},
	{0x74, /* t      */ { "insertData",			"",					"",				""					}},
	{0x75, /* u      */ { "insertData",			"insFmtUline",		"",				""					}},
	{0x76, /* v      */ { "insertData",			"paste",			"",				""					}},
	{0x77, /* w      */ { "insertData",			"",					"",				""					}},
	{0x78, /* x      */ { "insertData",			"cut",				"",				""					}},
	{0x79, /* y      */ { "insertData",			"redo",				"",				""					}},
	{0x7a, /* z      */ { "insertData",			"undo",				"",				""					}},
	{0x7b, /* {      */ { "insertData",			"",					"",				""					}},
	{0x7c, /* |      */ { "insertData",			"",					"",				""					}},
	{0x7d, /* }      */ { "insertData",			"",					"",				""					}},
	{0x7e, /* ~      */ { "insertData",			"",					"",				""					}},

	// TODO add the Latin1 characters with high bit set. (i'm tired of typing right now.)
};

static void s_loadChar(EV_EditBindingMap * pebm)
{
	int k, m;
	int kLimit = NrElements(s_CharTable);

	for (k=0; k<kLimit; k++)
		for (m=0; m<EV_COUNT_EMS_NoShift; m++)
			if (s_CharTable[k].m_szMethod[m] && *s_CharTable[k].m_szMethod[m])
			{
				EV_EditModifierState ems = EV_EMS_FromNumberNoShift(m);
				pebm->setBinding(EV_EKP_PRESS|s_CharTable[k].m_eb|ems,s_CharTable[k].m_szMethod[m]);
			}
}

/*****************************************************************
******************************************************************
** put it all together and load the default bindings.
******************************************************************
*****************************************************************/

UT_Bool ap_LoadBindings_Default(EV_EditMethodContainer * pemc,
								EV_EditBindingMap **ppebm)
{
	UT_ASSERT(pemc);
	UT_ASSERT(ppebm);

	*ppebm = 0;
	EV_EditBindingMap * pNewEBM = new EV_EditBindingMap(pemc);
	if (!pNewEBM)
		return UT_FALSE;

	s_loadMouse(pNewEBM);
	s_loadNVK(pNewEBM);
	s_loadChar(pNewEBM);

	*ppebm = pNewEBM;
	return UT_TRUE;
}
