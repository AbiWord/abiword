/* AbiSource Application Framework
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

#ifndef XAP_MACTOOLBARICONS_H
#define XAP_MACTOOLBARICONS_H

#ifndef XP_MAC_TARGET_QUARTZ
# include <QuickDraw.h>
#else
# include <CoreGraphics/CGImage.h>
#endif

#include "ut_types.h"
#include "ut_misc.h"
#include "xap_Toolbar_Icons.h"

/*****************************************************************/

class AP_MacToolbar_Icons : public AP_Toolbar_Icons
{
public:
	AP_MacToolbar_Icons(void);
	~AP_MacToolbar_Icons(void);

	UT_Bool getBitmapForIcon(UT_uint32 maxWidth,
							 UT_uint32 maxHeight,
							 UT_RGBColor * pColor,
							 const char * szIconName,
#ifndef XP_MAC_TARGET_QUARTZ
							 PixMapHandle pBitmap);
#else
							 CGImageRef pBitmap);
#endif

protected:
};

#endif /* XAP_MACTOOLBARICONS_H */
