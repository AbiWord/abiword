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

#include <stdlib.h>
#include <ctype.h>

#include "ut_assert.h"
#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ev_Keyboard.h"
#include "ev_NamedVirtualKey.h"
#include "ev_MacKeyboard.h"
#include "ev_EditMethod.h"
#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"

#ifdef UT_DEBUG
#define MSG(keydata,args)	do { if ( ! (keyData & 0x40000000)) UT_DEBUGMSG args ; } while (0)
#else
#define MSG(keydata,args)	do { } while (0)
#endif

/*****************************************************************/
/*****************************************************************/

ev_MacKeyboard::ev_MacKeyboard(EV_EditEventMapper * pEEM)
	: EV_Keyboard(pEEM)
{
}

/*****************************************************************/
/*****************************************************************/

