/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001,2002 Hubert Figuiere
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

#import <Cocoa/Cocoa.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_Frame.h"
#include "xap_CocoaFrame.h"
#include "ap_CocoaFrame.h"
#include "gr_CocoaGraphics.h"
#include "ap_CocoaStatusBar.h"
#import "xap_CocoaFrameImpl.h"

class ap_csb_TextListener : public AP_StatusBarFieldListener
{
public:
	/* policy is as usual retain the object */
	ap_csb_TextListener(AP_StatusBarField *pStatusBarField, NSTextField *pLabel) 
		: AP_StatusBarFieldListener(pStatusBarField) 
	{
		m_pLabel = pLabel; 
		[m_pLabel retain];
	}
	virtual ~ap_csb_TextListener()
	{
		[m_pLabel release];
	}
	virtual void notify(); 

protected:
	NSTextField *m_pLabel;
};

void ap_csb_TextListener::notify()
{
	UT_ASSERT(m_pLabel);

	AP_StatusBarField_TextInfo * textInfo = ((AP_StatusBarField_TextInfo *)m_pStatusBarField);
	const UT_UCS4Char * buf = textInfo->getBufUCS();
	UT_UTF8String utf8 (buf);	

	[m_pLabel setStringValue:[NSString stringWithUTF8String:utf8.utf8_str()]];

	// we conditionally update the size request, if the representative string (or an earlier
	// size) wasn't large enough, if the element uses the representative string method
	// and is aligned with the center
	if (textInfo->getFillMethod() == REPRESENTATIVE_STRING && 
			textInfo->getAlignmentMethod() == CENTER) {
		// TODO FIXME
	/*
		GtkRequisition requisition;
		gint iOldWidthRequest, iOldHeightRequest;
		gtk_widget_get_size_request(m_pLabel, &iOldWidthRequest, &iOldHeightRequest);
		gtk_widget_set_size_request(m_pLabel, -1, -1);		
		gtk_widget_size_request(m_pLabel, &requisition);
		if (requisition.width > iOldWidthRequest)			
			gtk_widget_set_size_request(m_pLabel, requisition.width, -1);		
		else
			gtk_widget_set_size_request(m_pLabel, iOldWidthRequest, -1);	
	*/
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_CocoaStatusBar::AP_CocoaStatusBar(XAP_Frame * pFrame)
	: AP_StatusBar(pFrame)
{
	m_wStatusBar = nil;
	m_hidden = false;

	/* fetch the widget from the controller */
	m_wStatusBar = [(XAP_CocoaFrameController *)(static_cast<XAP_CocoaFrameImpl*>(m_pFrame->getFrameImpl()))->_getController() getStatusBar];
}

AP_CocoaStatusBar::~AP_CocoaStatusBar(void)
{
	if ((m_hidden) && ([m_wStatusBar superview] == nil))
	{
		[m_wStatusBar release];
	}
}


void AP_CocoaStatusBar::setView(AV_View * pView)
{
	AP_StatusBar::setView(pView);
}

XAP_CocoaNSStatusBar * AP_CocoaStatusBar::createWidget(void)
{
	UT_ASSERT(m_wStatusBar);
	float currentX = 0.0;
	float height = [m_wStatusBar frame].size.height;
	for (UT_uint32 k=0; k<getFields()->getItemCount(); k++) {
 		AP_StatusBarField * pf = (AP_StatusBarField *)m_vecFields.getNthItem(k);
		UT_ASSERT(pf); // we should NOT have null elements
		
		AP_StatusBarField_TextInfo *pf_TextInfo = static_cast<AP_StatusBarField_TextInfo*>(pf);
		NSRect frame = NSMakeRect (currentX, 0, 50, height);
		NSTextField *pStatusBarElementLabel = [[NSTextField alloc] initWithFrame:frame];
		pf->setListener((AP_StatusBarFieldListener *)(new ap_csb_TextListener
				(pf_TextInfo, pStatusBarElementLabel)));
		[pStatusBarElementLabel setEditable:NO];
		[pStatusBarElementLabel setBezeled:YES];
		[pStatusBarElementLabel setSelectable:NO];
		[pStatusBarElementLabel setStringValue:[NSString stringWithUTF8String:pf_TextInfo->getRepresentativeString()]];
		// align
		if (pf_TextInfo->getAlignmentMethod() == LEFT) {
			// TODO set the text alignement. Probaably thru attributed string
		//	gtk_misc_set_alignment(GTK_MISC(pStatusBarElementLabel), 0.0, 0.0);
		}
			
		// size and place
		if (pf_TextInfo->getFillMethod() == REPRESENTATIVE_STRING) {
		/*
			GtkRequisition requisition;
			gtk_widget_size_request(pStatusBarElementLabel, &requisition);				
			gtk_widget_set_size_request(pStatusBarElementLabel, requisition.width, -1);
			
			gtk_box_pack_start(GTK_BOX(m_wStatusBar), pStatusBarElement, FALSE, FALSE, 0);
			*/
		}
		[m_wStatusBar addSubview:pStatusBarElementLabel];
	}
	
	return m_wStatusBar;
}



void AP_CocoaStatusBar::show(void)
{
	if ([m_wStatusBar superview] == nil) {
		[m_superView addSubview:m_wStatusBar];
		UT_ASSERT ([m_wStatusBar retainCount] > 1);
		[m_wStatusBar autorelease];		// at this time it should have already been retained.
	}
	m_hidden = false;
}

void AP_CocoaStatusBar::hide(void)
{
	if ([m_wStatusBar superview] != nil) {
		m_superView = [m_wStatusBar superview];
		UT_ASSERT (m_superView);
		[m_wStatusBar retain];
		[m_wStatusBar removeFromSuperview];
	}
	// TODO Check about resizing / layout changes
	m_hidden = true;
}

@implementation XAP_CocoaNSStatusBar
- (NSControl *)addField:(AP_StatusBarField*)field
{
	UT_ASSERT (UT_NOT_IMPLEMENTED);;
	return nil;
}


@end