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


#include "ut_assert.h"
#include "ut_types.h"
#include "ev_EditBits.h"
#include "ev_EditBinding.h"
#include "ev_EditMethod.h"
#include "ev_NamedVirtualKey.h"
#include "ap_LoadBindings.h"
#include "ap_LB_viEdit_r.h"

const ap_bs_Char s_CharTable[] =
{
//	{char, /* desc   */ { none,					_C,					_A,				_A_C				}},
	{0x21, /* !      */ { "replaceChar",			"",					"",				""					}},
	{0x22, /* "      */ { "replaceChar",			"",					"",				""					}},
	{0x23, /* #      */ { "replaceChar",			"",					"",				""					}},
	{0x24, /* $      */ { "replaceChar",			"",					"",				""					}},
	{0x25, /* %      */ { "replaceChar",			"",					"",				""					}},
	{0x26, /* &      */ { "replaceChar",			"",					"",				""					}},
	{0x27, /* '      */ { "replaceChar",			"",					"",				""					}},
	{0x28, /* (      */ { "replaceChar",			"",					"",				""					}},
	{0x29, /* )      */ { "replaceChar",			"",					"",				""					}},
	{0x2a, /* *      */ { "replaceChar",			"",					"",				""					}},
	{0x2b, /* +      */ { "replaceChar",			"",					"",				""					}},
	{0x2c, /* ,      */ { "replaceChar",			"",					"",				""					}},
	{0x2d, /* -      */ { "replaceChar",			"toggleSub",		"",				""					}},
	{0x2e, /* .      */ { "replaceChar",			"",					"",				""					}},
	{0x2f, /* /      */ { "replaceChar",			"",					"",				""					}},
	{0x30, /* 0      */ { "replaceChar",			"",					"",				""					}},
	{0x31, /* 1      */ { "replaceChar",			"singleSpace",		"setStyleHeading1",				""					}},
	{0x32, /* 2      */ { "replaceChar",			"doubleSpace",		"setStyleHeading2",				""					}},
	{0x33, /* 3      */ { "replaceChar",			"",					"setStyleHeading3",				""					}},
	{0x34, /* 4      */ { "replaceChar",			"",					"",				""					}},
	{0x35, /* 5      */ { "replaceChar",			"middleSpace",		"",				""					}},
	{0x36, /* 6      */ { "replaceChar",			"",					"",				""					}},
	{0x37, /* 7      */ { "replaceChar",			"",					"",				""					}},
	{0x38, /* 8      */ { "replaceChar",			"",					"",				""					}},
	{0x39, /* 9      */ { "replaceChar",			"",					"",				""					}},
	{0x3a, /* :      */ { "replaceChar",			"",					"",				""					}},
	{0x3b, /* ;      */ { "replaceChar",			"",					"",				""					}},
	{0x3c, /* <      */ { "replaceChar",			"",					"",				""					}},
	{0x3d, /* =      */ { "replaceChar",			"toggleSuper",		"",				""					}},
	{0x3e, /* >      */ { "replaceChar",			"",					"",				""					}},
	{0x3f, /* ?      */ { "replaceChar",			"",					"",				""					}},
	{0x40, /* @      */ { "replaceChar",			"",					"",				""					}},
	{0x41, /* A      */ { "replaceChar",			"selectAll",		"",				""					}},
	{0x42, /* B      */ { "replaceChar",			"toggleBold",		"",				""					}},
	{0x43, /* C      */ { "replaceChar",			"copy",				"",				""					}},
	{0x44, /* D      */ { "replaceChar",			"dlgFont",			"",				""					}},
	{0x45, /* E      */ { "replaceChar",			"alignCenter",		"",				""					}},
	{0x46, /* F      */ { "replaceChar",			"find",				"",				""					}},
	{0x47, /* G      */ { "replaceChar",			"go",				"",				""					}},
	{0x48, /* H      */ { "replaceChar",			"replace",			"",				""					}},
	{0x49, /* I      */ { "replaceChar",			"toggleItalic",		"",				""					}},
	{0x4a, /* J      */ { "replaceChar",			"alignJustify",		"",				""					}},
	{0x4b, /* K      */ { "replaceChar",			"toggleStrike",		"",				""					}},
	{0x4c, /* L      */ { "replaceChar",			"alignLeft",		"",				""					}},
	{0x4d, /* M      */ { "replaceChar",			"insSymbol",					"",				""					}},
	{0x4e, /* N      */ { "replaceChar",			"fileNew",			"",				""					}},
	{0x4f, /* O      */ { "replaceChar",			"fileOpen",			"",				""					}},
	{0x50, /* P      */ { "replaceChar",			"print",			"",				""					}},
	{0x51, /* Q      */ { "replaceChar",			"",					"",				""					}},
	{0x52, /* R      */ { "replaceChar",			"alignRight",		"",				""					}},
	{0x53, /* S      */ { "replaceChar",			"fileSave",			"",				""					}},
	{0x54, /* T      */ { "replaceChar",			"toggleOline",		"",				""					}},
	{0x55, /* U      */ { "replaceChar",			"toggleUline",		"",				""					}},
	{0x56, /* V      */ { "replaceChar",			"paste",			"",				""					}},
	{0x57, /* W      */ { "replaceChar",			"closeWindow",		"",				""					}},
	{0x58, /* X      */ { "replaceChar",			"cut",				"",				""					}},
	{0x59, /* Y      */ { "replaceChar",			"redo",				"",				""					}},
	{0x5a, /* Z      */ { "replaceChar",			"undo",				"",				""					}},
	{0x5b, /* [      */ { "replaceChar",			"",					"",				""					}},
	{0x5c, /* \      */ { "replaceChar",			"",					"",				""					}},
	{0x5d, /* ]      */ { "replaceChar",			"",					"",				""					}},
	{0x5e, /* ^      */ { "replaceChar",			"",					"",				""					}},
	{0x5f, /* -      */ { "replaceChar",			"",					"",				""					}},
	{0x60, /* `      */ { "replaceChar",			"",					"",				""					}},
	{0x61, /* a      */ { "replaceChar",			"selectAll",		"",				""					}},
	{0x62, /* b      */ { "replaceChar",			"toggleBold",		"",				""					}},
	{0x63, /* c      */ { "replaceChar",			"copy",				"",				""					}},
	{0x64, /* d      */ { "replaceChar",			"dlgFont",			"",				""					}},
	{0x65, /* e      */ { "replaceChar",			"alignCenter",		"",				""					}},
	{0x66, /* f      */ { "replaceChar",			"find",				"",				""					}},
	{0x67, /* g      */ { "replaceChar",			"go",				"",				""					}},
	{0x68, /* h      */ { "replaceChar",			"replace",			"",				""					}},
	{0x69, /* i      */ { "replaceChar",			"toggleItalic",		"",				""					}},
	{0x6a, /* j      */ { "replaceChar",			"alignJustify",		"",				""					}},
	{0x6b, /* k      */ { "replaceChar",			"toggleStrike",		"",				""					}},
	{0x6c, /* l      */ { "replaceChar",			"alignLeft",		"",				""					}},
	{0x6d, /* m      */ { "replaceChar",			"insSymbol",					"",				""					}},
	{0x6e, /* n      */ { "replaceChar",			"fileNew",			"",				""					}},
	{0x6f, /* o      */ { "replaceChar",			"fileOpen",			"",				""					}},
	{0x70, /* p      */ { "replaceChar",			"print",			"",				""					}},
	{0x71, /* q      */ { "replaceChar",			"",					"",				""					}},
	{0x72, /* r      */ { "replaceChar",			"alignRight",		"",				""					}},
	{0x73, /* s      */ { "replaceChar",			"fileSave",			"",				""					}},
	{0x74, /* t      */ { "replaceChar",			"toggleOline",		"",				""					}},
	{0x75, /* u      */ { "replaceChar",			"toggleUline",		"",				""					}},
	{0x76, /* v      */ { "replaceChar",			"paste",			"",				""					}},
	{0x77, /* w      */ { "replaceChar",			"closeWindow",		"",				""					}},
	{0x78, /* x      */ { "replaceChar",			"cut",				"",				""					}},
	{0x79, /* y      */ { "replaceChar",			"redo",				"",				""					}},
	{0x7a, /* z      */ { "replaceChar",			"undo",				"",				""					}},
	{0x7b, /* {      */ { "replaceChar",			"",					"",				""					}},
	{0x7c, /* |      */ { "replaceChar",			"",					"",				""					}},
	{0x7d, /* }      */ { "replaceChar",			"",					"",				""					}},
	{0x7e, /* ~      */ { "replaceChar",			"",					"",				""					}},

//	Here are bindings for the portion of Latin-1 with the high-bit set.
//	I'm taking the list from /usr/include/X11/keysymdef.h, so I think
//	that the set is reasonably complete.
//	
//	I don't know how to test these on my en-US system with my en-US keyboard,
//	but I think they will work.
//
//	TODO For now I just bound them all to "replaceChar" which will cause
//	TODO the character to be inserted into the document.  I'm wondering
//	TODO if some of these (e.g., paragraph and section) should be bound
//	TODO to some other functions -- but rather than try guessing, let's
//	TODO wait and let some of our European friends comment.

//	{char, /* desc           */ { none,				_C,					_A,				_A_C				}},

	{0xa0, /* nbs            */ { "insertNBSpace",	"",					"",				""					}},
	{0xa1, /* exclamdown     */ { "replaceChar",		"",					"",				""					}},
	{0xa2, /* cent           */ { "replaceChar",		"",					"",				""					}},
	{0xa3, /* sterling       */ { "replaceChar",		"",					"",				""					}},
	{0xa4, /* currency       */ { "replaceChar",		"",					"",				""					}},
	{0xa5, /* yen            */ { "replaceChar",		"",					"",				""					}},
	{0xa6, /* brokenbar      */ { "replaceChar",		"",					"",				""					}},
	{0xa7, /* section        */ { "replaceChar",		"",					"",				""					}},
	{0xa8, /* diaeresis      */ { "replaceChar",		"",					"",				""					}},
	{0xa9, /* copyright      */ { "replaceChar",		"",					"",				""					}},
	{0xaa, /* ordfeminine    */ { "replaceChar",		"",					"",				""					}},
	{0xab, /* guillemotleft  */ { "replaceChar",		"",					"",				""					}},
	{0xac, /* notsign        */ { "replaceChar",		"",					"",				""					}},
	{0xad, /* hyphen         */ { "replaceChar",		"",					"",				""					}},
	{0xae, /* registered     */ { "replaceChar",		"",					"",				""					}},
	{0xaf, /* macron         */ { "replaceChar",		"",					"",				""					}},
	{0xb0, /* degree         */ { "replaceChar",		"",					"",				""					}},
	{0xb1, /* plusminus      */ { "replaceChar",		"",					"",				""					}},
	{0xb2, /* twosuperior    */ { "replaceChar",		"",					"",				""					}},
	{0xb3, /* threesuperior  */ { "replaceChar",		"",					"",				""					}},
	{0xb4, /* acute          */ { "replaceChar",		"",					"",				""					}},
	{0xb5, /* mu             */ { "replaceChar",		"",					"",				""					}},
	{0xb6, /* paragraph      */ { "replaceChar",		"",					"",				""					}},
	{0xb7, /* periodcentered */ { "replaceChar",		"",					"",				""					}},
	{0xb8, /* cedilla        */ { "replaceChar",		"",					"",				""					}},
	{0xb9, /* onesuperior    */ { "replaceChar",		"",					"",				""					}},
	{0xba, /* masculine      */ { "replaceChar",		"",					"",				""					}},
	{0xbb, /* guillemotright */ { "replaceChar",		"",					"",				""					}},
	{0xbc, /* onequarter     */ { "replaceChar",		"",					"",				""					}},
	{0xbd, /* onehalf        */ { "replaceChar",		"",					"",				""					}},
	{0xbe, /* threequarters  */ { "replaceChar",		"",					"",				""					}},
	{0xbf, /* questiondown   */ { "replaceChar",		"",					"",				""					}},
	{0xc0, /* Agrave         */ { "replaceChar",		"",					"",				""					}},
	{0xc1, /* Aacute         */ { "replaceChar",		"",					"",				""					}},
	{0xc2, /* Acircumflex    */ { "replaceChar",		"",					"",				""					}},
	{0xc3, /* Atilde         */ { "replaceChar",		"",					"",				""					}},
	{0xc4, /* Adiaeresis     */ { "replaceChar",		"",					"",				""					}},
	{0xc5, /* Aring          */ { "replaceChar",		"",					"",				""					}},
	{0xc6, /* AE             */ { "replaceChar",		"",					"",				""					}},
	{0xc7, /* Ccedilla       */ { "replaceChar",		"",					"",				""					}},
	{0xc8, /* Egrave         */ { "replaceChar",		"",					"",				""					}},
	{0xc9, /* Eacute         */ { "replaceChar",		"",					"",				""					}},
	{0xca, /* Ecircumflex    */ { "replaceChar",		"",					"",				""					}},
	{0xcb, /* Ediaeresis     */ { "replaceChar",		"",					"",				""					}},
	{0xcc, /* Igrave         */ { "replaceChar",		"",					"",				""					}},
	{0xcd, /* Iacute         */ { "replaceChar",		"",					"",				""					}},
	{0xce, /* Icircumflex    */ { "replaceChar",		"",					"",				""					}},
	{0xcf, /* Idiaeresis     */ { "replaceChar",		"",					"",				""					}},
	{0xd0, /* ETH            */ { "replaceChar",		"",					"",				""					}},
	{0xd1, /* Ntilde         */ { "replaceChar",		"",					"",				""					}},
	{0xd2, /* Ograve         */ { "replaceChar",		"",					"",				""					}},
	{0xd3, /* Oacute         */ { "replaceChar",		"",					"",				""					}},
	{0xd4, /* Ocircumflex    */ { "replaceChar",		"",					"",				""					}},
	{0xd5, /* Otilde         */ { "replaceChar",		"",					"",				""					}},
	{0xd6, /* Odiaeresis     */ { "replaceChar",		"",					"",				""					}},
	{0xd7, /* multiply       */ { "replaceChar",		"",					"",				""					}},
	{0xd8, /* Ooblique       */ { "replaceChar",		"",					"",				""					}},
	{0xd9, /* Ugrave         */ { "replaceChar",		"",					"",				""					}},
	{0xda, /* Uacute         */ { "replaceChar",		"",					"",				""					}},
	{0xdb, /* Ucircumflex    */ { "replaceChar",		"",					"",				""					}},
	{0xdc, /* Udiaeresis     */ { "replaceChar",		"",					"",				""					}},
	{0xdd, /* Yacute         */ { "replaceChar",		"",					"",				""					}},
	{0xde, /* THORN          */ { "replaceChar",		"",					"",				""					}},
	{0xdf, /* ssharp         */ { "replaceChar",		"",					"",				""					}},
	{0xe0, /* agrave         */ { "replaceChar",		"",					"",				""					}},
	{0xe1, /* aacute         */ { "replaceChar",		"",					"",				""					}},
	{0xe2, /* acircumflex    */ { "replaceChar",		"",					"",				""					}},
	{0xe3, /* atilde         */ { "replaceChar",		"",					"",				""					}},
	{0xe4, /* adiaeresis     */ { "replaceChar",		"",					"",				""					}},
	{0xe5, /* aring          */ { "replaceChar",		"",					"",				""					}},
	{0xe6, /* ae             */ { "replaceChar",		"",					"",				""					}},
	{0xe7, /* ccedilla       */ { "replaceChar",		"",					"",				""					}},
	{0xe8, /* egrave         */ { "replaceChar",		"",					"",				""					}},
	{0xe9, /* eacute         */ { "replaceChar",		"",					"",				""					}},
	{0xea, /* ecircumflex    */ { "replaceChar",		"",					"",				""					}},
	{0xeb, /* ediaeresis     */ { "replaceChar",		"",					"",				""					}},
	{0xec, /* igrave         */ { "replaceChar",		"",					"",				""					}},
	{0xed, /* iacute         */ { "replaceChar",		"",					"",				""					}},
	{0xee, /* icircumflex    */ { "replaceChar",		"",					"",				""					}},
	{0xef, /* idiaeresis     */ { "replaceChar",		"",					"",				""					}},
	{0xf0, /* eth            */ { "replaceChar",		"",					"",				""					}},
	{0xf1, /* ntilde         */ { "replaceChar",		"",					"",				""					}},
	{0xf2, /* ograve         */ { "replaceChar",		"",					"",				""					}},
	{0xf3, /* oacute         */ { "replaceChar",		"",					"",				""					}},
	{0xf4, /* ocircumflex    */ { "replaceChar",		"",					"",				""					}},
	{0xf5, /* otilde         */ { "replaceChar",		"",					"",				""					}},
	{0xf6, /* odiaeresis     */ { "replaceChar",		"",					"",				""					}},
	{0xf7, /* division       */ { "replaceChar",		"",					"",				""					}},
	{0xf8, /* oslash         */ { "replaceChar",		"",					"",				""					}},
	{0xf9, /* ugrave         */ { "replaceChar",		"",					"",				""					}},
	{0xfa, /* uacute         */ { "replaceChar",		"",					"",				""					}},
	{0xfb, /* ucircumflex    */ { "replaceChar",		"",					"",				""					}},
	{0xfc, /* udiaeresis     */ { "replaceChar",		"",					"",				""					}},
	{0xfd, /* yacute         */ { "replaceChar",		"",					"",				""					}},
	{0xfe, /* thorn          */ { "replaceChar",		"",					"",				""					}},
	{0xff, /* ydiaeresis     */ { "replaceChar",		"",					"",				""					}},
};

bool ap_LoadBindings_viEdit_r(AP_BindingSet * pThis,
							 	EV_EditBindingMap * pebm)
{
	pThis->_loadChar(pebm,s_CharTable,NrElements(s_CharTable),NULL,0);
	
	return true;
}
