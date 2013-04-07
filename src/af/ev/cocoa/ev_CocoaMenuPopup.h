/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef EV_COCOAMENUPOPUP_H
#define EV_COCOAMENUPOPUP_H

#import <Cocoa/Cocoa.h>

#include "ev_CocoaMenu.h"


class AV_View;
class AP_CocoaFrame;

/*****************************************************************/

class EV_CocoaMenuPopup : public EV_CocoaMenu
{
public:
	EV_CocoaMenuPopup(const char * szMenuLayoutName,
					 const char * szMenuLabelSetName);
	virtual ~EV_CocoaMenuPopup(void);

	virtual bool		synthesizeMenuPopup(void);
	virtual bool		refreshMenu(AV_View * pView);
	virtual NSMenu *	getMenuHandle(void) const;

protected:
	NSMenu *			m_wMenuPopup;
};

#endif /* EV_COCOAMENUPOPUP_H */
