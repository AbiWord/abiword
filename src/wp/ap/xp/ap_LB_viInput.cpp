/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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


// **********************************************************************************
// **********************************************************************************
// *** THIS FILE DEFINES VI input mode KEYBOARD AND MOUSE BINDINGS FOR AbiWord 1. ***
// *** To define bindings for other emulations, clone this file and change the    ***
// *** various settings.  See ap_LoadBindings.cpp for more information.           ***
// **********************************************************************************
// **********************************************************************************

#include "ut_assert.h"
#include "ut_types.h"
#include "ev_EditBits.h"
#include "ev_EditBinding.h"
#include "ev_EditMethod.h"
#include "ev_NamedVirtualKey.h"
#include "ap_LoadBindings.h"
#include "ap_LB_viInput.h"

#define _S		| EV_EMS_SHIFT
#define _C		| EV_EMS_CONTROL
#define _A		| EV_EMS_ALT

extern const ap_bs_Mouse MouseTable[];

/*****************************************************************
******************************************************************
** load top-level (non-prefixed) builtin bindings for the NamedVirtualKeys
******************************************************************
*****************************************************************/

const ap_bs_NVK viIn_NVKTable[] =
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
	{EV_NVK_ESCAPE,		{ "setEditVI",			"setEditVI",		"setEditVI",	"setEditVI",
						  "setEditVI",			"setEditVI",		"setEditVI",	"setEditVI"			}},
	{EV_NVK_PAGEUP,		{ "scrollPageUp",		"extSelPageUp",		"warpInsPtPrevPage",	"",
						  "",					"",					"warpInsPtBOP",	""					}},
	{EV_NVK_PAGEDOWN,	{ "scrollPageDown",		"extSelPageDown",	"warpInsPtNextPage",	"",
						  "",					"",					"warpInsPtEOP",	""					}},
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
	{EV_NVK_MENU_SHORTCUT,	{ "contextMenu",	"",					"",				"",
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
	{EV_NVK_F10,		{ "",					"contextMenu",		"",				"",
						  "",					"",					"",				""					}},
//	{EV_NVK_F11,		{ "",					"",					"",				"",
//						  "",					"",					"",				""					}},
	{EV_NVK_F12,		{ "",					"",					"",				"",
						  FN_TEST_DUMP,			"",					"",				""					}},
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

extern const ap_bs_NVK_Prefix NVKTable_P[];
extern const ap_bs_Char CharTable[];

#if 0
/*****************************************************************
 ** non-nvk table of prefix keys
 ****************************************************************/

const ap_bs_Char_Prefix s_CharPrefixTable[] =
{
//  Warning: case is significant here Ctrl-x and Ctrl-X are different :-)	
//	{char, /* desc   */ { none,					_C,					_A,				_A_C				}},
//	{0x78, /* x      */ { "",					"",					"",				""					}},
};
#endif

/*****************************************************************
******************************************************************
** put it all together and load the default bindings.
******************************************************************
*****************************************************************/

UT_Bool ap_LoadBindings_viInput(AP_BindingSet * pThis, EV_EditBindingMap * pebm)
{
	extern UT_uint32 MouseTable_len, NVKTable_P_len, CharTable_len;
	pThis->_loadMouse(pebm,MouseTable,MouseTable_len);

	pThis->_loadNVK(pebm,viIn_NVKTable,NrElements(viIn_NVKTable),NVKTable_P,NVKTable_P_len);

	//pThis->_loadChar(pebm,s_CharTable,NrElements(s_CharTable),s_CharPrefixTable,NrElements(s_CharPrefixTable));
	pThis->_loadChar(pebm,CharTable,CharTable_len,NULL,0);

	return UT_TRUE;
}
