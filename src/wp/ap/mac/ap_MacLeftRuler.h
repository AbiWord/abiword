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

#ifndef AP_MACLEFTRULER_H
#define AP_MACLEFTRULER_H

// Class for dealing with the horizontal ruler at the top of
// a document window.

/*****************************************************************/

#include "ut_types.h"
#include "ap_LeftRuler.h"
#ifndef XP_MAC_TARGET_QUARTZ
# include "gr_MacQDGraphics.h"
#else
# include "gr_MacGraphics.h"
#endif
class XAP_MacApp;
class XAP_Frame;

/*****************************************************************/

class AP_MacLeftRuler : public AP_LeftRuler
{
public:
	AP_MacLeftRuler(XAP_Frame * pFrame);
	virtual ~AP_MacLeftRuler(void);

};

#endif /* AP_MACLEFTRULER_H */
