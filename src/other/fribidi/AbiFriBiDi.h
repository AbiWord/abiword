/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
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

This is an interface between the FriBiDi library tables and AbiWord */

#ifndef ABIFRIBIDI_H

#include "ut_types.h"

#define FriBidiChar UT_UCSChar

/* Classifications of the various Bidi properties */
typedef enum
{
  FRIBIDI_TYPE_LTR = 0x01000000, /* Strong left to right */
  FRIBIDI_TYPE_RTL = 0x02000000, /* Strong right to left */
  FRIBIDI_TYPE_WL  = 0x01000001, /* Weak left to right */
  FRIBIDI_TYPE_WR  = 0x02000002, /* Weak right to left */
  FRIBIDI_TYPE_EN  = 0x03000000, /* European digit */
  FRIBIDI_TYPE_ES  = 0x04000000, /* European number separator */
  FRIBIDI_TYPE_ET  = 0x05000000, /* European number terminator */
  FRIBIDI_TYPE_AN  = 0x06000000, /* Arabic digit */
  FRIBIDI_TYPE_CS  = 0x07000000, /* Common Separator */
  FRIBIDI_TYPE_BS  = 0x08000000, /* Block separator */
  FRIBIDI_TYPE_SS  = 0x09000000, /* Segment separator */
  FRIBIDI_TYPE_WS  = 0x0A000000, /* Whitespace */
  FRIBIDI_TYPE_CTL = 0x10000090, /* Control units */
  FRIBIDI_TYPE_ON  = 0x80000009,  /* Other Neutral */

  /* The following are only used internally */
  FRIBIDI_TYPE_L   = 0x01000000,
  FRIBIDI_TYPE_R   = 0x02000000,
  FRIBIDI_TYPE_BN  = 0xF1000000,
  FRIBIDI_TYPE_CM  = 0xF2000000,
  FRIBIDI_TYPE_SOT = 0xF3000000,
  FRIBIDI_TYPE_EOT = 0xF4000000,
  FRIBIDI_TYPE_N   = 0xF5000000,
  FRIBIDI_TYPE_E   = 0xF6000000
} FriBidiCharType;

// the following is only defined for the sake of the file included below
// and is undef immediately afterwards.
#define gint UT_sint32

#include "fribidi_tables.i"

#undef gint


//if the following line is uncommented a-z will be treated as ltr, A-Z as rtl and
//space, tab and digits as directionally neutral.

//#define CAPS_TEST

//this function returns 1 if c is rtl, 0 if it is ltr and -1 if it is directionally neutral

UT_sint32 isUCharRTL(UT_UCSChar c)
{
    //UT_DEBUGMSG(("isUCharRTL: c=%d\n", c));
#ifdef CAPS_TEST
    if(((c >= '0') && (c <= '9')) || (c == 9) || (c == 32))
    	return (-1);
    else if ((c >= 'A') && (c <= 'Z'))
    	return (1);
    else
    	return (0);
#else
    for (UT_sint32 i = 0; i < nFriBidiPropertyList; i++)
    {
      if((c >= FriBidiPropertyList[i].first) && (c <= FriBidiPropertyList[i].last))
      {
         switch (FriBidiPropertyList[i].char_type)
         {
             case FRIBIDI_TYPE_LTR:
             case FRIBIDI_TYPE_WL:
             case FRIBIDI_TYPE_EN:
                 //UT_DEBUGMSG(("returning %d\n", 0));
                 return (0);

             case FRIBIDI_TYPE_RTL:
                  FRIBIDI_TYPE_WR:
                  FRIBIDI_TYPE_AN:
                 //UT_DEBUGMSG(("returning %d\n", 1));
                 return (1);

             default:
                 //UT_DEBUGMSG(("returning %d\n", -1));
                 return (-1);
         }
      }
    }
    UT_ASSERT((UT_SHOULD_NOT_HAPPEN));
    return (-1);
#endif
}

#endif
