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
#include "xap_UnixFrame.h"
#include "gr_UnixGraphics.h"
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

	const UT_UCS4Char * buf = ((AP_StatusBarField_TextInfo *)m_pStatusBarField)->getBufUCS();
	UT_UCS4String * ucs = new UT_UCS4String(buf);
	
	UT_ASSERT(m_pLabel);
	// HACK: there's no decent way of giving some left padding on the gtklabel (that I know of)
	// which looks aesthetically pleasing, so we add an extra space to the label text
	char paddedLabel[AP_MAX_MESSAGE_FIELD];
	paddedLabel[0] = ' ';
	paddedLabel[1] = '\0';
	UT_ASSERT(strlen(ucs->utf8_str()) < (AP_MAX_MESSAGE_FIELD - 2));
	if (strlen(ucs->utf8_str()) < (AP_MAX_MESSAGE_FIELD - 2)) // buffer overflow check
		strcat(paddedLabel, ucs->utf8_str());

	gtk_label_set_label(GTK_LABEL(m_pLabel), paddedLabel);
	delete(ucs);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_UnixStatusBar::AP_UnixStatusBar(XAP_Frame * pFrame)
	: AP_StatusBar(pFrame)
{
	m_wStatusBar = NULL;

	GtkWidget * toplevel = (static_cast<XAP_UnixFrame *> (m_pFrame))->getTopLevelWindow();
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
// FIXME: we need to call a view update when this happens, to make sure the elements get
// their needed updates. this is why the page numbering doesn't appear on startup
GtkWidget * AP_UnixStatusBar::createWidget(void)
{
	UT_ASSERT(!m_wStatusBar);
	
	// probably should make this into an event box (if we want the user to be able to interact with the status bar)
	m_wStatusBar = gtk_hbox_new(FALSE, 0);
	gtk_object_set_user_data(GTK_OBJECT(m_wStatusBar),this);
	gtk_widget_show(m_wStatusBar);

	for (UT_uint32 k=0; k<getFields()->getItemCount(); k++) {
 		AP_StatusBarField * pf = (AP_StatusBarField *)m_vecFields.getNthItem(k);
		UT_ASSERT(pf); // we should NOT have null elements

		// set up a frame for status bar elements so they look like status bar elements, 
		// and not just normal widgets
		GtkWidget *pStatusBarElement = NULL;
		
		if (AP_StatusBarField_TextInfo *pf_TextInfo = dynamic_cast<AP_StatusBarField_TextInfo*>(pf)) {
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
