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


#ifndef FV_LISTENER_H
#define FV_LISTENER_H

#include "ut_types.h"

class FV_View;

typedef UT_uint32 FV_ListenerId;

typedef UT_Byte FV_ChangeMask;
#define FV_CHG_NONE			((FV_ChangeMask) 0x00) 
#define FV_CHG_DO			((FV_ChangeMask) 0x01)		// canDo
#define FV_CHG_DIRTY		((FV_ChangeMask) 0x02)		// isDirty 
#define FV_CHG_EMPTYSEL		((FV_ChangeMask) 0x04)		// isSelectionEmpty 
#define FV_CHG_FILENAME		((FV_ChangeMask) 0x08)		// getFilename
#define FV_CHG_FMTBLOCK		((FV_ChangeMask) 0x10)		// getBlockFormat
#define FV_CHG_FMTCHAR		((FV_ChangeMask) 0x20)		// getCharFormat
#define FV_CHG_ALL			((FV_ChangeMask) 0xFF) 

#define FV_CHG_SAVE			((FV_ChangeMask) (FV_CHG_DO | FV_CHG_DIRTY | FV_CHG_FILENAME))
#define FV_CHG_TYPING		((FV_ChangeMask) (FV_CHG_DO | FV_CHG_DIRTY | FV_CHG_EMPTYSEL))
#define FV_CHG_MOTION		((FV_ChangeMask) (FV_CHG_EMPTYSEL | FV_CHG_FMTBLOCK | FV_CHG_FMTCHAR))

/* 
	Various UI elements (title, toolbar, etc.) need to stay in sync with 
	the current state of an FV_View.  They can do so by registering
	an FV_Listener with the FV_View in order to be notified of the existence 
	of certain changes to the document as they occur.  

	Note that these notifications do *not* pass any document state, they 
	simply note the existence of a certain category of changes to that state. 
	
	The view will notify each registered listener (in an undefined order).  
	When the listener registers, it is provided an ID which may be used
	later to refer to it.
*/

class FV_Listener
{
public:
	virtual UT_Bool		notify(FV_View * pView, const FV_ChangeMask mask) = 0;
};

	
#endif /* FV_LISTENER_H */
