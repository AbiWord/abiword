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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <gtk/gtk.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_Frame.h"
#include "ap_UnixStatusBar.h"
#include "xap_UnixDialogHelper.h"
#include "xap_Gtk2Compat.h"

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
		gtk_widget_get_preferred_size(m_pLabel, &requisition, NULL);
		if (requisition.width > iOldWidthRequest)
			gtk_widget_set_size_request(m_pLabel, requisition.width, -1);
		else
			gtk_widget_set_size_request(m_pLabel, iOldWidthRequest, -1);
	}
}


class ap_usb_ProgressListener : public AP_StatusBarFieldListener
{
public:
	ap_usb_ProgressListener(AP_StatusBarField *pStatusBarField, GtkWidget *wProgress) : AP_StatusBarFieldListener(pStatusBarField) 
        { 
	    m_wProgress = wProgress; 
	}
	virtual void notify(); 

protected:
	GtkWidget *m_wProgress;
};

void ap_usb_ProgressListener::notify()
{
	UT_ASSERT(m_wProgress);

	AP_StatusBarField_ProgressBar * pProgress = ((AP_StatusBarField_ProgressBar *)m_pStatusBarField);
	if(pProgress->isDefinate())
        {
	    double fraction = pProgress->getFraction();
	    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(m_wProgress),fraction);
	}
	else
	{
	    gtk_progress_bar_pulse(GTK_PROGRESS_BAR(m_wProgress));
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_UnixStatusBar::AP_UnixStatusBar(XAP_Frame * pFrame)
	: AP_StatusBar(pFrame)
{
	m_wStatusBar = NULL;
	m_wProgressFrame = NULL;
}

AP_UnixStatusBar::~AP_UnixStatusBar(void)
{
}

void AP_UnixStatusBar::setView(AV_View * pView)
{
	// let the base class do it's thing	
	AP_StatusBar::setView(pView);
}

void AP_UnixStatusBar::showProgressBar(void)
{
  gtk_widget_show(m_wProgressFrame);
}

void AP_UnixStatusBar::hideProgressBar(void)
{
  gtk_widget_hide(m_wProgressFrame);
}

// FIXME: we need more sanity checking here to make sure everything allocates correctly
GtkWidget * AP_UnixStatusBar::createWidget(void)
{
	UT_ASSERT(!m_wStatusBar);
	
	// probably should make this into an event box (if we want the user to be able to interact with the status bar)
	m_wStatusBar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

	gtk_widget_show(m_wStatusBar);

	for (UT_sint32 k=0; k<getFields()->getItemCount(); k++) {
 		AP_StatusBarField * pf = (AP_StatusBarField *)m_vecFields.getNthItem(k);
		UT_ASSERT(pf); // we should NOT have null elements

		// set up a frame for status bar elements so they look like status bar elements, 
		// and not just normal widgets
		GtkWidget *pStatusBarElement = NULL;
		UT_DEBUGMSG(("Fill method %d \n",pf->getFillMethod()));
		if (pf->getFillMethod() == REPRESENTATIVE_STRING || (pf->getFillMethod() == MAX_POSSIBLE)){ //AP_StatusBarField_TextInfo *pf_TextInfo = dynamic_cast<AP_StatusBarField_TextInfo*>(pf))
		  AP_StatusBarField_TextInfo *pf_TextInfo = static_cast<AP_StatusBarField_TextInfo*>(pf);
			pStatusBarElement = gtk_frame_new(NULL);
			gtk_frame_set_shadow_type(GTK_FRAME(pStatusBarElement), GTK_SHADOW_IN);
			
			GtkWidget *pStatusBarElementLabel = gtk_label_new(pf_TextInfo->getRepresentativeString());
			pf->setListener((AP_StatusBarFieldListener *)(new ap_usb_TextListener(pf_TextInfo, pStatusBarElementLabel)));
			gtk_container_add(GTK_CONTAINER(pStatusBarElement), pStatusBarElementLabel);

			// align
			if (pf_TextInfo->getAlignmentMethod() == LEFT) {
				GValue val = G_VALUE_INIT;
				g_value_init(&val, G_TYPE_FLOAT);
				g_value_set_float(&val, 0.0);
				g_object_set_property(G_OBJECT(pStatusBarElementLabel),
						"xalign", &val);
				g_object_set_property(G_OBJECT(pStatusBarElementLabel),
						"yalign", &val);
			}

			// size and place
			if (pf_TextInfo->getFillMethod() == REPRESENTATIVE_STRING) {
				GtkRequisition requisition;
				gtk_widget_get_preferred_size(pStatusBarElementLabel, &requisition, NULL);
				gtk_widget_set_size_request(pStatusBarElementLabel, requisition.width, -1);

				gtk_box_pack_start(GTK_BOX(m_wStatusBar), pStatusBarElement, FALSE, FALSE, 0);
			}
			else { // fill
				gtk_box_pack_start(GTK_BOX(m_wStatusBar), pStatusBarElement, TRUE, TRUE, 0);
			}

			gtk_label_set_label(GTK_LABEL(pStatusBarElementLabel), ""); 
			gtk_widget_show(pStatusBarElementLabel);
		}
		else if(pf->getFillMethod() == 	PROGRESS_BAR)
		{
			GtkRequisition requisition;
			pStatusBarElement = gtk_frame_new(NULL);
			gtk_widget_get_preferred_size(pStatusBarElement, &requisition, NULL);
			gtk_widget_set_size_request(pStatusBarElement, -1, requisition.height);
			gtk_frame_set_shadow_type(GTK_FRAME(pStatusBarElement), GTK_SHADOW_IN);

			gtk_box_pack_start(GTK_BOX(m_wStatusBar), pStatusBarElement, TRUE, TRUE, 0);
			GtkWidget *  pProgress= gtk_progress_bar_new();
			gtk_container_add(GTK_CONTAINER(pStatusBarElement),pProgress);
			gtk_progress_bar_set_pulse_step (GTK_PROGRESS_BAR(pProgress),0.01);

			gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(pProgress),0.0);
			gtk_widget_show(pProgress);
			pf->setListener((AP_StatusBarFieldListener *)(new ap_usb_ProgressListener(pf, pProgress)));
			m_wProgressFrame = pStatusBarElement;

		}
		else
		{
		        UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		}

		gtk_widget_show(pStatusBarElement);
	}
	gtk_widget_show_all(m_wStatusBar);
	hideProgressBar();
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
