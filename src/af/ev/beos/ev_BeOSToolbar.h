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

#ifndef EV_BEOSTOOLBAR_H
#define EV_BEOSTOOLBAR_H

#include "ut_types.h"
#include "ut_vector.h"
#include "xap_Types.h"
#include "ev_Toolbar.h"
#include "xav_Listener.h"

#include <InterfaceKit.h>

class XAP_BeOSApp;
class XAP_BeOSFrame;
class AP_BeOSToolbar_Icons;
class EV_BeOSToolbar_ViewListener;

/*********************************************************/
enum _tb_types {
	NONE			= 0, 	//0x0
	SEPERATOR 		= 1,	//0x1
	BITMAP_BUTTON 	= 2,	//0x2
	BITMAP_TOGGLE 	= 6,	//0x2 | 0x4
	DROP_DOWN		= 8		//0x8
};
#define BITMAP_MASK 	0x2
#define ENABLED_MASK	0x1
#define PRESSED_MASK	0x2

typedef struct {
	int 			type, state;
	AP_Toolbar_Id	id;
	BBitmap 		*bitmap; 
	BMenuField		*menu;
	BRect			rect;
} tb_item_t;

#define ITEM_WIDTH 		23
#define ITEM_HEIGHT		23
#define ITEM_SEPERATE	5
#define ITEM_MAX 		20

class EV_BeOSToolbar;

class ToolbarView: public BView {
	public:
		ToolbarView(EV_BeOSToolbar *tb, BRect frame, const char *name, 
					 uint32 resizeMask, uint32 flags);
		~ToolbarView();
				
		bool AddSeperator();
		bool AddItem(BBitmap *onbitmap, BBitmap *offbitmap, AP_Toolbar_Id id);
		bool AddItem(BPopUpMenu * menu, int width, AP_Toolbar_Id id);
		
		virtual void Draw(BRect clip);
		virtual void FrameResized(float width, float height);
		virtual void MessageReceived(BMessage *msg);
		virtual	void MouseMoved(BPoint where, uint32 code, const BMessage *msg);
		tb_item_t *  FindItemByID(AP_Toolbar_Id id);
				
		void 		HighLightItem(int index, int up);

		int				item_count, last_highlight;
		tb_item_t		items[ITEM_MAX];
		EV_BeOSToolbar	*m_pBeOSToolbar;
};


/*********************************************************/

class EV_BeOSToolbar : public EV_Toolbar
{
public:
	EV_BeOSToolbar(XAP_BeOSApp * pBeOSApp, XAP_BeOSFrame * pBeOSFrame,
				   const char * szToolbarLayoutName,
				   const char * szToolbarLabelSetName);
	~EV_BeOSToolbar(void);

	//This is called in the frame code
	UT_Bool bindListenerToView(AV_View * pView);
	//This is called in the frame code
	UT_Bool synthesize(void);
	//This is called locally and by the toolbar listener
	UT_Bool refreshToolbar(AV_View * pView, AV_ChangeMask mask);

	//This is called by the local BeOS View
	UT_Bool toolbarEvent(AP_Toolbar_Id id,
			 UT_UCSChar * pData,
			 UT_uint32 dataLength);

protected:
	void 	_releaseListener(void);


	XAP_BeOSApp *					m_pBeOSApp;
	XAP_BeOSFrame *					m_pBeOSFrame;
	EV_BeOSToolbar_ViewListener *	m_pViewListener;
	AP_BeOSToolbar_Icons *			m_pBeOSToolbarIcons;
	ToolbarView						*m_pTBView;
};

#endif /* EV_BEOSTOOLBAR_H */
