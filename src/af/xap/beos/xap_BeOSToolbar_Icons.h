/* AbiSource Application Framework
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

#ifndef XAP_BEOSTOOLBARICONS_H
#define XAP_BEOSTOOLBARICONS_H

#include <Bitmap.h>
#include "ut_types.h"
#include "xap_Toolbar_Icons.h"

/*****************************************************************/

class AP_BeOSToolbar_Icons : public AP_Toolbar_Icons
{
public:
	AP_BeOSToolbar_Icons(void);
	~AP_BeOSToolbar_Icons(void);

	BBitmap *GetIconBitmap(const char * szIconName);
protected:
};

#endif /* XAP_BEOSTOOLBARICONS_H */
