/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2001-2002 Hubert Figuiere
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

#ifndef EV_COCOAMENU_H
#define EV_COCOAMENU_H

#import <Cocoa/Cocoa.h>

#include "ut_types.h"
#include "ut_vector.h"
#include "xap_Types.h"
#include "ev_Menu.h"

class AV_View;
class XAP_CocoaApp;
class AP_CocoaFrame;

/*****************************************************************/
@interface EV_CocoaMenuTarget : NSObject
{
	
}
- (id)menuSelected:(id)sender;
- (BOOL)validateMenuItem:(id <NSMenuItem>)menuItem;
@end



class EV_CocoaMenu : public EV_Menu
{
public:
	EV_CocoaMenu(XAP_CocoaApp * pCocoaApp,
				AP_CocoaFrame * pCocoaFrame,
				const char * szMenuLayoutName,
				const char * szMenuLabelSetName);
	virtual ~EV_CocoaMenu();

	bool				synthesizeMenu(NSMenu * wMenuRoot);
	bool				menuEvent(XAP_Menu_Id menuid);
	virtual bool		refreshMenu(AV_View * pView) = 0;
	
	AP_CocoaFrame * 	getFrame();

	bool				_refreshMenu(AV_View * pView, NSMenu * wMenuRoot); // can't set this one protected
	                                     // as it is called from Obj-C
protected:
	bool				_isItemPresent(XAP_Menu_Id menuid) const;

	virtual bool		_doAddMenuItem(UT_uint32 layout_pos);

private:
	static NSString* _getItemCmd (const char * mnemonic, unsigned int & modifiers);
	XAP_CocoaApp *		m_pCocoaApp;
	AP_CocoaFrame *		m_pCocoaFrame;
	
	// actual GTK menu widgets
	UT_Vector			m_vecMenuWidgets;
	EV_CocoaMenuTarget	*m_menuTarget;
};

#endif /* EV_COCOAMENU_H */
