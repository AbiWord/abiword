/* AbiSource Program Utilities
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
 



#ifndef EV_EDITBITS_H
#define EV_EDITBITS_H

#include "ut_types.h"

/*****************************************************************
******************************************************************
** we compress virtual key code and mouse ops along with modifier
** keys into a single long word in order to facilitate binding lookups.
**
** TODO redo these definitions (consider keeping mouse and keyboard
** TODO separate).
**
******************************************************************
*****************************************************************/

typedef UT_uint32 EV_EditModifierState;								/* may be ORed */
#define EV_EMS__MASK__			((EV_EditModifierState) 0xff000000)
#define EV_EMS_SHIFT			((EV_EditModifierState) 0x01000000)
#define EV_EMS_CONTROL			((EV_EditModifierState) 0x02000000)
#define EV_EMS_ALT				((EV_EditModifierState) 0x04000000)
#define EV_EMS_ToNumber(ems)			(((ems)&EV_EMS__MASK__)>>24)
#define EV_EMS_ToNumberNoShift(ems)		(((ems)&EV_EMS__MASK__)>>25)	/* exclude shift */
#define EV_EMS_FromNumber(n)			(((n<<24)&EV_EMS__MASK__))
#define EV_EMS_FromNumberNoShift(n)		(((n<<25)&EV_EMS__MASK__))


typedef UT_uint32 EV_EditKeyPress;									/* may be ORed */
#define EV_EKP__MASK__			((EV_EditKeyPress)		0x00880000)
#define EV_EKP_PRESS			((EV_EditKeyPress)		0x00800000)
#define EV_EKP_NAMEDKEY			((EV_EditKeyPress)		0x00080000)
#define EV_NamedKey(xxxx)	(EV_EKP_NAMEDKEY | (xxxx))


typedef UT_uint32 EV_EditMouseButton;								/* may not be ORed */
#define EV_EMB__MASK__			((EV_EditMouseButton)	0x00700000)
#define EV_EMB_BUTTON1			((EV_EditMouseButton)	0x00100000)
#define EV_EMB_BUTTON2			((EV_EditMouseButton)	0x00200000)
#define EV_EMB_BUTTON3			((EV_EditMouseButton)	0x00300000)
#define EV_EMB_BUTTON4			((EV_EditMouseButton)	0x00400000)
#define EV_EMB_BUTTON5			((EV_EditMouseButton)	0x00500000)
#define EV_EMB_ToNumber(emb)	(((emb)&EV_EMB__MASK__)>>20)


typedef UT_uint32 EV_EditMouseOp;									/* may not be ORed */
#define EV_EMO__MASK__			((EV_EditMouseOp)		0x00070000)
#define EV_EMO_SINGLECLICK		((EV_EditMouseOp)		0x00010000)
#define EV_EMO_DOUBLECLICK		((EV_EditMouseOp)		0x00020000)
#define EV_EMO_DRAG				((EV_EditMouseOp)		0x00030000)
#define EV_EMO_RELEASE			((EV_EditMouseOp)		0x00040000)
#define EV_EMO_ToNumber(emb)	(((emb)&EV_EMO__MASK__)>>16)
#define EV_EMO_FromNumber(n)	(((n<<16)&EV_EMO__MASK__))


typedef UT_uint32 EV_EditVirtualKey;								/* may not be ORed */
#define EV_EVK__MASK__			((EV_EditVirtualKey)	0x0000ffff)
#define EV_EVK_ToNumber(evk)	(((evk)&EV_EVK__MASK__))
#define EV_NVK_ToNumber(nvk)	(((nvk)&EV_EVK__MASK__))


typedef UT_uint32 EV_EditBits;	/* union of all the above bits */

// the EV_COUNT_ give the number of unique combinations
// currently defined in the bits above.

#define EV_COUNT_EMS			8		// combinations under 'OR' (including 0)
#define EV_COUNT_EMS_NoShift	(EV_COUNT_EMS/2)

#define EV_COUNT_EMB			5		// simple count (not 'OR')
#define EV_COUNT_EMO			4		// simple count (not 'OR')

#endif /* EV_EDITBITS_H */
