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

#include <gtk/gtk.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_Frame.h"
#include "ap_UnixStatusBar.h"
#include "xap_UnixDialogHelper.h"

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class ap_usb_TextListener : public AP_StatusBarFieldListener
{
public:
	ap_usb_TextListener(AP_StatusBarField *pStatusBarField, GtkWidget *pLabel) : AP_StatusBarFieldListener(pStatusBarField) { m_pLabel = pLabel; }
	virtual void notify(); 

protected:
	GtkWidget *m_pLabel;
};

void ap_usb_TextListener::notify()
{
	UT_ASSERT(m_pLabel);

	AP_StatusBarField_TextInfo * textInfo = ((AP_StatusBarField_TextInfo *)m_pStatusBarField);

	gtk_label_set_label(GTK_LABEL(m_pLabel), textInfo->getBuf().utf8_str());

	// we conditionally update the size request, if the representative string (or an earlier
	// size) wasn't large enough, if the element uses the representative string method
	// and is aligned with the center
	if (textInfo->getFillMethod() == REPRESENTATIVE_STRING && 
	    textInfo->getAlignmentMethod() == CENTER) {
		GtkRequisition requisition;
		gint iOldWidthRequest, iOldHeightRequest;
		gtk_widget_get_size_request(m_pLabel, &iOldWidthRequest, &iOldHeightRequest);
		gtk_widget_set_size_request(m_pLabel, -1, -1);		
		gtk_widget_size_request(m_pLabel, &requisition);
		if (requisition.width > iOldWidthRequest)			
			gtk_widget_set_size_request(m_pLabel, requisition.width, -1);		
		else
			gtk_widget_set_size_request(m_pLabel, iOldWidthRequest, -1);					
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_UnixStatusBar::AP_UnixStatusBar(XAP_Frame * pFrame)
	: AP_StatusBar(pFrame)
{
	m_wStatusBar = NULL;
}

AP_UnixStatusBar::~AP_UnixStatusBar(void)
{
}

void AP_UnixStatusBar::setView(AV_View * pView)
{
	// let the base class do it's thing	
	AP_StatusBar::setView(pView);
}

// FIXME: we need more sanity checking here to make sure everything allocates correctly
GtkWidget * AP_UnixStatusBar::createWidget(void)
{
	UT_ASSERT(!m_wStatusBar);
	
	// probably should make this into an event box (if we want the user to be able to interact with the status bar)
	m_wStatusBar = gtk_hbox_new(FALSE, 0);

	gtk_widget_show(m_wStatusBar);

	for (UT_sint32 k=0; k<getFields()->getItemCount(); k++) {
 		AP_StatusBarField * pf = (AP_StatusBarField *)m_vecFields.getNthItem(k);
		UT_ASSERT(pf); // we should NOT have null elements

		// set up a frame for status bar elements so they look like status bar elements, 
		// and not just normal widgets
		GtkWidget *pStatusBarElement = NULL;
		
		if (true){ //AP_StatusBarField_TextInfo *pf_TextInfo = dynamic_cast<AP_StatusBarField_TextInfo*>(pf))
		  AP_StatusBarField_TextInfo *pf_TextInfo = static_cast<AP_StatusBarField_TextInfo*>(pf);
			pStatusBarElement = gtk_frame_new(NULL);
			gtk_frame_set_shadow_type(GTK_FRAME(pStatusBarElement), GTK_SHADOW_IN);
			
			GtkWidget *pStatusBarElementLabel = gtk_label_new(pf_TextInfo->getRepresentativeString());
			pf->setListener((AP_StatusBarFieldListener *)(new ap_usb_TextListener(pf_TextInfo, pStatusBarElementLabel)));
			gtk_container_add(GTK_CONTAINER(pStatusBarElement), pStatusBarElementLabel);

			// align
			if (pf_TextInfo->getAlignmentMethod() == LEFT) {
				gtk_misc_set_alignment(GTK_MISC(pStatusBarElementLabel), 0.0, 0.0);
			}
			
			// size and place
			if (pf_TextInfo->getFillMethod() == REPRESENTATIVE_STRING) {
				GtkRequisition requisition;
				gtk_widget_size_request(pStatusBarElementLabel, &requisition);				
				gtk_widget_set_size_request(pStatusBarElementLabel, requisition.width, -1);
				
				gtk_box_pack_start(GTK_BOX(m_wStatusBar), pStatusBarElement, FALSE, FALSE, 0);
			}
			else { // fill
				gtk_box_pack_start(GTK_BOX(m_wStatusBar), pStatusBarElement, TRUE, TRUE, 0);
			}
			
			gtk_label_set_label(GTK_LABEL(pStatusBarElementLabel), ""); 
			gtk_widget_show(pStatusBarElementLabel);
		}
		else {
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN); // there are no other kinds of elements
		}

		gtk_widget_show(pStatusBarElement);
	}
			
	return m_wStatusBar;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
	
void AP_UnixStatusBar::show(void)
{
	gtk_widget_show (m_wStatusBar);
}

void AP_UnixStatusBar::hide(void)
{
	gtk_widget_hide (m_wStatusBar);
	m_pFrame->queue_resize();
}
