/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 1999 John Brewer DBA Jera Design
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

#ifndef AP_MACTOPRULER_H
#define AP_MACTOPRULER_H

// Class for dealing with the horizontal ruler at the top of
// a document window.

/*****************************************************************/

#include "ut_types.h"
#include "ap_TopRuler.h"
#ifndef XP_MAC_TARGET_QUARTZ
# include "gr_MacQDGraphics.h"
#else
# include "gr_MacGraphics.h"
#endif
class XAP_MacApp;
class XAP_Frame;

/*****************************************************************/

class AP_MacTopRuler : public AP_TopRuler
{
public:
	AP_MacTopRuler(XAP_Frame * pFrame);
	virtual ~AP_MacTopRuler(void);
};

#endif /* AP_MACTOPRULER_H */
