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


// *********************************************************************************
// *********************************************************************************
// *** THIS FILE DEFINES VI Edit mode KEYBOARD AND MOUSE BINDINGS FOR AbiWord 1. ***
// *** To define bindings for other emulations, clone this file and change the   ***
// *** various settings.  See ap_LoadBindings.cpp for more information.          ***
// *********************************************************************************
// *********************************************************************************

#include "ut_assert.h"
#include "ut_types.h"
#include "ev_EditBits.h"
#include "ev_EditBinding.h"
#include "ev_EditMethod.h"
#include "ev_NamedVirtualKey.h"
#include "ap_LoadBindings.h"
#include "ap_LB_viEdit.h"

#define PREFIX_KEY	""

#define _S		| EV_EMS_SHIFT
#define _C		| EV_EMS_CONTROL
#define _A		| EV_EMS_ALT

extern ap_bs_Mouse MouseTable[];

/*****************************************************************
******************************************************************
** load top-level (non-prefixed) builtin bindings for the NamedVirtualKeys
******************************************************************
*****************************************************************/

extern ap_bs_NVK NVKTable[];

/*****************************************************************
******************************************************************
** load top-level prefixed builtin bindings for the NamedVirtualKeys
******************************************************************
*****************************************************************/

extern ap_bs_NVK_Prefix NVKTable_P[];

/*****************************************************************
******************************************************************
** load top-level (non-prefixed) builtin bindings for the non-nvk
** (character keys).  note that SHIFT-state is implicit in the
** character value and is not included in the table.  note that
** we do not include the ASCII control characters (\x00 -- \x1f
** and others) since we don't map keystrokes into them.
******************************************************************
*****************************************************************/

const ap_bs_Char s_CharTable[] =
{
//	{char, /* desc   */ { none,					_C,					_A,				_A_C				}},
	{0x21, /* !      */ { "",					"",					"",				""					}},
	{0x22, /* "      */ { "",					"",					"",				""					}},
	{0x23, /* #      */ { "",					"",					"",				""					}},
	{0x24, /* $      */ { "warpInsPtEOL",		"",					"",				""					}},
	{0x25, /* %      */ { "",					"",					"",				""					}},
	{0x26, /* &      */ { "",					"",					"",				""					}},
	{0x27, /* '      */ { "",					"",					"",				""					}},
	{0x28, /* (      */ { "warpInsPtBOS",		"",					"",				""					}},
	{0x29, /* )      */ { "warpInsPtEOS",		"",					"",				""					}},
	{0x2a, /* *      */ { "",					"",					"",				""					}},
	{0x2b, /* +      */ { "",					"",					"",				""					}},
	{0x2c, /* ,      */ { "",					"",					"",				""					}},
	{0x2d, /* -      */ { "",					"",					"",				""					}},
	{0x2e, /* .      */ { "",					"",					"",				""					}},
	{0x2f, /* /      */ { "find",				"",					"",				""					}},
	{0x30, /* 0      */ { "warpInsPtBOL",		"",					"",				""					}},
	{0x31, /* 1      */ { "",					"",					"",				""					}},
	{0x32, /* 2      */ { "",					"",					"",				""					}},
	{0x33, /* 3      */ { "",					"",					"",				""					}},
	{0x34, /* 4      */ { "",					"",					"",				""					}},
	{0x35, /* 5      */ { "",					"",					"",				""					}},
	{0x36, /* 6      */ { "",					"",					"",				""					}},
	{0x37, /* 7      */ { "",					"",					"",				""					}},
	{0x38, /* 8      */ { "",					"",					"",				""					}},
	{0x39, /* 9      */ { "",					"",					"",				""					}},
	{0x3a, /* :      */ { PREFIX_KEY,			"",					"",				""					}},
	{0x3b, /* ;      */ { "",					"",					"",				""					}},
	{0x3c, /* <      */ { "",					"",					"warpInsPtBOD",	""					}},
	{0x3d, /* =      */ { "",					"",					"",				""					}},
	{0x3e, /* >      */ { "",					"",					"warpInsPtEOD",	""					}},
	{0x3f, /* ?      */ { "find",				"",					"",				""					}},
	{0x40, /* @      */ { "",					"",					"",				""					}},
	{0x41, /* A      */ { "viCmd_A",			"",					"",				""					}},
	{0x42, /* B      */ { "",					"",					"",				""					}},
	{0x43, /* C      */ { "",					"",					"",				""					}},
	{0x44, /* D      */ { "delEOL",				"",					"",				""					}},
	{0x45, /* E      */ { "",					"",					"",				""					}},
	{0x46, /* F      */ { "",					"",					"",				""					}},
	{0x47, /* G      */ { "warpInsPtEOD",		"",					"",				""					}},
	{0x48, /* H      */ { "",					"",					"",				""					}},
	//Move cursor to BOL and then switch to Insert mode
	{0x49, /* I      */ { "viCmd_I",			"",					"",				""					}},
	//Should be the join command which would translate to moving to the end of a sentence and then removing the para-break.
	{0x4a, /* J      */ { "viCmd_J",			"",					"",				""					}},
	{0x4b, /* K      */ { "",					"",		   			"",				""					}},
	{0x4c, /* L      */ { "",					"",					"",				""					}},
	{0x4d, /* M      */ { "",					"",					"",				""					}},
	{0x4e, /* N      */ { "",					"",					"",				""					}},
	//Should move cursor to begining of line,  insert a return and switch to Input mode
	{0x4f, /* O      */ { "viCmd_O",			"",					"",				""					}},
	{0x50, /* P      */ { "viCmd_P",			"",					"",				""					}},
	{0x51, /* Q      */ { "",					"",					"",				""					}},
	{0x52, /* R      */ { "",					"",					"",				""					}},
	{0x53, /* S      */ { "",					"",	   				"",				""					}},
	{0x54, /* T      */ { "",					"",					"",				""					}},
	{0x55, /* U      */ { "undo",				"",					"",				""					}},
	{0x56, /* V      */ { "",					"",					"",				""					}},
	{0x57, /* W      */ { "",					"",					"",				""					}},
	{0x58, /* X      */ { "delLeft",			"",					"",				""					}},
	{0x59, /* Y      */ { "viCmd_yy",					"",					"",				""					}},
	{0x5a, /* Z      */ { "",					"",					"",				""					}},
	{0x5b, /* [      */ { "warpInsPtBOB",		"",					"",				""					}},
	{0x5c, /* \      */ { "",					"",					"",				""					}},
	{0x5d, /* ]      */ { "warpInsPtEOB",		"",					"",				""					}},
	{0x5e, /* ^      */ { "warpInsPtBOL",		"",					"",				""					}},
	{0x5f, /* -      */ { "",					"",					"",				""					}},
	{0x60, /* `      */ { "",					"",					"",				""					}},
	{0x61, /* a      */ { "viCmd_a",			"",					"",				""					}},
	{0x62, /* b      */ { "warpInsPtBOW",		"",					"",				""					}},
	{0x63, /* c      */ { PREFIX_KEY,			"",					"",				""					}},
	{0x64, /* d      */ { PREFIX_KEY,			"",					"",				""					}},
	{0x65, /* e      */ { "warpInsPtEOW",		"",					"",				""					}},
	{0x66, /* f      */ { "",					"",					"",				""					}},
	{0x67, /* g      */ { "",					"",					"",				""					}},
	{0x68, /* h      */ { "warpInsPtLeft",		"",					"",				""					}},
	{0x69, /* i      */ { "setInputVI",			"",					"",				""					}},
	{0x6a, /* j      */ { "warpInsPtNextLine",	"",					"",				""					}},
	{0x6b, /* k      */ { "warpInsPtPrevLine",	"",					"",				""					}},
	{0x6c, /* l      */ { "warpInsPtRight",		"",					"",				""					}},
	{0x6d, /* m      */ { "",					"",					"",				""					}},
	{0x6e, /* n      */ { "findAgain",			"",					"",				""					}},
	// Should open a new line below current line
	{0x6f, /* o      */ { "viCmd_o",			"",					"",				""					}},
	{0x70, /* p      */ { "paste",				"",					"",				""					}},
	{0x71, /* q      */ { "",					"",					"",				""					}},
	{0x72, /* r      */ { "viCmd_r",			"",					"",				""					}},
	{0x73, /* s      */ { "",					"",					"",				""					}},
	{0x74, /* t      */ { "",					"",					"",				""					}},
	{0x75, /* u      */ { "undo",				"",					"",				""					}},
	// Could implement visual selection from vim but might add too much bloat
	{0x76, /* v      */ { "",					"",					"",				""					}},
	{0x77, /* w      */ { "warpInsPtEOW",		"",					"",				""					}},
	{0x78, /* x      */ { "delRight",			"",					"",				""					}},
	{0x79, /* y      */ { PREFIX_KEY,			"",					"",				""					}},
	{0x7a, /* z      */ { "",					"",					"",				""					}},
	{0x7b, /* {      */ { "",					"",					"",				""					}},
	{0x7c, /* |      */ { "warpInsPtBOL",		"",					"",				""					}},
	{0x7d, /* }      */ { "",					"",					"",				""					}},
	{0x7e, /* ~      */ { "",					"",					"",				""					}},

#if 0
//	Here are bindings for the portion of Latin-1 with the high-bit set.
//	I'm taking the list from /usr/include/X11/keysymdef.h, so I think
//	that the set is reasonably complete.
//	
//	I don't know how to test these on my en-US system with my en-US keyboard,
//	but I think they will work.
//
//	TODO For now I just bound them all to "insertData" which will cause
//	TODO the character to be inserted into the document.  I'm wondering
//	TODO if some of these (e.g., paragraph and section) should be bound
//	TODO to some other functions -- but rather than try guessing, let's
//	TODO wait and let some of our European friends comment.

//	{char, /* desc           */ { none,				_C,					_A,				_A_C				}},

	{0xa0, /* nbs            */ { "",	"",					"",				""					}},
	{0xa1, /* exclamdown     */ { "",		"",					"",				""					}},
	{0xa2, /* cent           */ { "",		"",					"",				""					}},
	{0xa3, /* sterling       */ { "",		"",					"",				""					}},
	{0xa4, /* currency       */ { "",		"",					"",				""					}},
	{0xa5, /* yen            */ { "",		"",					"",				""					}},
	{0xa6, /* brokenbar      */ { "",		"",					"",				""					}},
	{0xa7, /* section        */ { "",		"",					"",				""					}},
	{0xa8, /* diaeresis      */ { "",		"",					"",				""					}},
	{0xa9, /* copyright      */ { "",		"",					"",				""					}},
	{0xaa, /* ordfeminine    */ { "",		"",					"",				""					}},
	{0xab, /* guillemotleft  */ { "",		"",					"",				""					}},
	{0xac, /* notsign        */ { "",		"",					"",				""					}},
	{0xad, /* hyphen         */ { "",		"",					"",				""					}},
	{0xae, /* registered     */ { "",		"",					"",				""					}},
	{0xaf, /* macron         */ { "",		"",					"",				""					}},
	{0xb0, /* degree         */ { "",		"",					"",				""					}},
	{0xb1, /* plusminus      */ { "",		"",					"",				""					}},
	{0xb2, /* twosuperior    */ { "",		"",					"",				""					}},
	{0xb3, /* threesuperior  */ { "",		"",					"",				""					}},
	{0xb4, /* acute          */ { "",		"",					"",				""					}},
	{0xb5, /* mu             */ { "",		"",					"",				""					}},
	{0xb6, /* paragraph      */ { "",		"",					"",				""					}},
	{0xb7, /* periodcentered */ { "",		"",					"",				""					}},
	{0xb8, /* cedilla        */ { "",		"",					"",				""					}},
	{0xb9, /* onesuperior    */ { "",		"",					"",				""					}},
	{0xba, /* masculine      */ { "",		"",					"",				""					}},
	{0xbb, /* guillemotright */ { "",		"",					"",				""					}},
	{0xbc, /* onequarter     */ { "",		"",					"",				""					}},
	{0xbd, /* onehalf        */ { "",		"",					"",				""					}},
	{0xbe, /* threequarters  */ { "",		"",					"",				""					}},
	{0xbf, /* questiondown   */ { "",		"",					"",				""					}},
	{0xc0, /* Agrave         */ { "",		"",					"",				""					}},
	{0xc1, /* Aacute         */ { "",		"",					"",				""					}},
	{0xc2, /* Acircumflex    */ { "",		"",					"",				""					}},
	{0xc3, /* Atilde         */ { "",		"",					"",				""					}},
	{0xc4, /* Adiaeresis     */ { "",		"",					"",				""					}},
	{0xc5, /* Aring          */ { "",		"",					"",				""					}},
	{0xc6, /* AE             */ { "",		"",					"",				""					}},
	{0xc7, /* Ccedilla       */ { "",		"",					"",				""					}},
	{0xc8, /* Egrave         */ { "",		"",					"",				""					}},
	{0xc9, /* Eacute         */ { "",		"",					"",				""					}},
	{0xca, /* Ecircumflex    */ { "",		"",					"",				""					}},
	{0xcb, /* Ediaeresis     */ { "",		"",					"",				""					}},
	{0xcc, /* Igrave         */ { "",		"",					"",				""					}},
	{0xcd, /* Iacute         */ { "",		"",					"",				""					}},
	{0xce, /* Icircumflex    */ { "",		"",					"",				""					}},
	{0xcf, /* Idiaeresis     */ { "",		"",					"",				""					}},
	{0xd0, /* ETH            */ { "",		"",					"",				""					}},
	{0xd1, /* Ntilde         */ { "",		"",					"",				""					}},
	{0xd2, /* Ograve         */ { "",		"",					"",				""					}},
	{0xd3, /* Oacute         */ { "",		"",					"",				""					}},
	{0xd4, /* Ocircumflex    */ { "",		"",					"",				""					}},
	{0xd5, /* Otilde         */ { "",		"",					"",				""					}},
	{0xd6, /* Odiaeresis     */ { "",		"",					"",				""					}},
	{0xd7, /* multiply       */ { "",		"",					"",				""					}},
	{0xd8, /* Ooblique       */ { "",		"",					"",				""					}},
	{0xd9, /* Ugrave         */ { "",		"",					"",				""					}},
	{0xda, /* Uacute         */ { "",		"",					"",				""					}},
	{0xdb, /* Ucircumflex    */ { "",		"",					"",				""					}},
	{0xdc, /* Udiaeresis     */ { "",		"",					"",				""					}},
	{0xdd, /* Yacute         */ { "",		"",					"",				""					}},
	{0xde, /* THORN          */ { "",		"",					"",				""					}},
	{0xdf, /* ssharp         */ { "",		"",					"",				""					}},
	{0xe0, /* agrave         */ { "",		"",					"",				""					}},
	{0xe1, /* aacute         */ { "",		"",					"",				""					}},
	{0xe2, /* acircumflex    */ { "",		"",					"",				""					}},
	{0xe3, /* atilde         */ { "",		"",					"",				""					}},
	{0xe4, /* adiaeresis     */ { "",		"",					"",				""					}},
	{0xe5, /* aring          */ { "",		"",					"",				""					}},
	{0xe6, /* ae             */ { "",		"",					"",				""					}},
	{0xe7, /* ccedilla       */ { "",		"",					"",				""					}},
	{0xe8, /* egrave         */ { "",		"",					"",				""					}},
	{0xe9, /* eacute         */ { "",		"",					"",				""					}},
	{0xea, /* ecircumflex    */ { "",		"",					"",				""					}},
	{0xeb, /* ediaeresis     */ { "",		"",					"",				""					}},
	{0xec, /* igrave         */ { "",		"",					"",				""					}},
	{0xed, /* iacute         */ { "",		"",					"",				""					}},
	{0xee, /* icircumflex    */ { "",		"",					"",				""					}},
	{0xef, /* idiaeresis     */ { "",		"",					"",				""					}},
	{0xf0, /* eth            */ { "",		"",					"",				""					}},
	{0xf1, /* ntilde         */ { "",		"",					"",				""					}},
	{0xf2, /* ograve         */ { "",		"",					"",				""					}},
	{0xf3, /* oacute         */ { "",		"",					"",				""					}},
	{0xf4, /* ocircumflex    */ { "",		"",					"",				""					}},
	{0xf5, /* otilde         */ { "",		"",					"",				""					}},
	{0xf6, /* odiaeresis     */ { "",		"",					"",				""					}},
	{0xf7, /* division       */ { "",		"",					"",				""					}},
	{0xf8, /* oslash         */ { "",		"",					"",				""					}},
	{0xf9, /* ugrave         */ { "",		"",					"",				""					}},
	{0xfa, /* uacute         */ { "",		"",					"",				""					}},
	{0xfb, /* ucircumflex    */ { "",		"",					"",				""					}},
	{0xfc, /* udiaeresis     */ { "",		"",					"",				""					}},
	{0xfd, /* yacute         */ { "",		"",					"",				""					}},
	{0xfe, /* thorn          */ { "",		"",					"",				""					}},
	{0xff, /* ydiaeresis     */ { "",		"",					"",				""					}},
#endif	
};

/*****************************************************************
 ** non-nvk table of prefix keys
 ****************************************************************/

const ap_bs_Char_Prefix s_CharPrefixTable[] =
{
//  Warning: case is significant here Ctrl-x and Ctrl-X are different :-)	
//	{char, /* desc   */ { none,					_C,					_A,				_A_C				}},
	{0x3a, /* :      */ { "viEdit_colon",		"",					"",				""					}},
	{0x63, /* c      */ { "viEdit_c",			"",					"",				""					}},
	{0x64, /* d      */ { "viEdit_d",			"",					"",				""					}},
	{0x72, /* r      */ { "viEdit_r",			"",					"",				""					}},
	{0x79, /* y      */ { "viEdit_y",			"",					"",				""					}}
};

/*****************************************************************
******************************************************************
** put it all together and load the default bindings.
******************************************************************
*****************************************************************/

bool ap_LoadBindings_viEdit(AP_BindingSet * pThis, EV_EditBindingMap * pebm)
{
	extern UT_uint32 MouseTable_len, NVKTable_len, NVKTable_P_len; 
	pThis->_loadMouse(pebm,MouseTable,MouseTable_len);
	pThis->_loadNVK(pebm,NVKTable,NVKTable_len,NVKTable_P,NVKTable_P_len);
	pThis->_loadChar(pebm,s_CharTable,NrElements(s_CharTable),s_CharPrefixTable,NrElements(s_CharPrefixTable));

	return true;
}
