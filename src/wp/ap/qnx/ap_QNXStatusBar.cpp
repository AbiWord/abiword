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

#include <Pt.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_Frame.h"
#include "xap_QNXFrame.h"
#include "gr_QNXGraphics.h"
#include "ap_QNXStatusBar.h"
#include "ap_StatusBar.h"

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class ap_usb_TextListener : public AP_StatusBarFieldListener
{
public:
	ap_usb_TextListener(AP_StatusBarField *pStatusBarField, PtWidget_t *pLabel) : AP_StatusBarFieldListener(pStatusBarField) { m_pLabel = pLabel; }
	virtual void notify(); 

protected:
	PtWidget_t *m_pLabel;
};

void ap_usb_TextListener::notify()
{

	const UT_UCS4Char * buf = ((AP_StatusBarField_TextInfo *)m_pStatusBarField)->getBufUCS();
	UT_UCS4String * ucs = new UT_UCS4String(buf);
	
	UT_ASSERT(m_pLabel);
#if 0
	// HACK: there's no decent way of giving some left padding on the gtklabel (that I know of)
	// which looks aesthetically pleasing, so we add an extra space to the label text
	char paddedLabel[AP_MAX_MESSAGE_FIELD];
	paddedLabel[0] = ' ';
	paddedLabel[1] = '\0';
	UT_ASSERT(strlen(ucs->utf8_str()) < (AP_MAX_MESSAGE_FIELD - 2));
	if (strlen(ucs->utf8_str()) < (AP_MAX_MESSAGE_FIELD - 2)) // buffer overflow check
		strcat(paddedLabel, ucs->utf8_str());
#endif

	PtSetResource(m_pLabel,Pt_ARG_TEXT_STRING,ucs->utf8_str(),0);
	delete(ucs);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_QNXStatusBar::AP_QNXStatusBar(XAP_Frame * pFrame)
	: AP_StatusBar(pFrame)
{
	m_wStatusBar = NULL;

}

AP_QNXStatusBar::~AP_QNXStatusBar(void)
{
}

void AP_QNXStatusBar::setView(AV_View * pView)
{
	// let the base class do it's thing	
	AP_StatusBar::setView(pView);
}

// FIXME: we need more sanity checking here to make sure everything allocates correctly
// FIXME: we need to call a view update when this happens, to make sure the elements get
// their needed updates. this is why the page numbering doesn't appear on startup
PtWidget_t * AP_QNXStatusBar::createWidget(void)
{
	PtArg_t args[10];
	int n=0;
	PhPoint_t ptr;
	unsigned short *h,*w;
	UT_ASSERT(!m_wStatusBar);


	PtWidget_t * toplevel = (static_cast<XAP_QNXFrame *> (m_pFrame))->getTopLevelWindow();
	PtGetResource(toplevel,Pt_ARG_HEIGHT,&h,0);	
	PtGetResource(toplevel,Pt_ARG_WIDTH,&w,0);
	ptr.y = *h -24;
	ptr.x = 0;
	// probably should make this into an event box (if we want the user to be able to interact with the status bar)
	PtSetArg(&args[n++],Pt_ARG_TOOLBAR_FLAGS,Pt_TOOLBAR_ITEM_SEPARATORS ,Pt_TOOLBAR_END_SEPARATOR|Pt_TOOLBAR_DRAGGABLE|Pt_TOOLBAR_ITEM_SEPARATORS);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT,24,0);
	PtSetArg(&args[n++], Pt_ARG_ANCHOR_FLAGS,Pt_TRUE,Pt_LEFT_ANCHORED_LEFT|Pt_RIGHT_ANCHORED_RIGHT|Pt_BOTTOM_ANCHORED_BOTTOM);
	PtSetArg(&args[n++], Pt_ARG_WIDTH,*w,0);
	PtSetArg(&args[n++], Pt_ARG_POS,&ptr,0);
	m_wStatusBar = PtCreateWidget(PtToolbar,toplevel,n,args); 
	n=0;

	for (UT_uint32 k=0; k<getFields()->getItemCount(); k++) {
 		AP_StatusBarField * pf = (AP_StatusBarField *)m_vecFields.getNthItem(k);
		UT_ASSERT(pf); // we should NOT have null elements

		// set up a frame for status bar elements so they look like status bar elements, 
		// and not just normal widgets
		
		if (AP_StatusBarField_TextInfo *pf_TextInfo = dynamic_cast<AP_StatusBarField_TextInfo*>(pf)) {

			PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,pf_TextInfo->getRepresentativeString(),0);			
			PtWidget_t *pStatusBarElementLabel = PtCreateWidget(PtLabel,Pt_DEFAULT_PARENT,n,args);

			pf->setListener((AP_StatusBarFieldListener *)(new ap_usb_TextListener(pf_TextInfo, pStatusBarElementLabel)));
			// align
			if (pf_TextInfo->getAlignmentMethod() == LEFT) {
				PtSetResource(pStatusBarElementLabel,Pt_ARG_HORIZONTAL_ALIGNMENT,Pt_LEFT,0);				
			}
			
			// size and place
			if (pf_TextInfo->getFillMethod() == REPRESENTATIVE_STRING) {
/*				GtkRequisition requisition;
				gtk_widget_size_request(pStatusBarElementLabel, &requisition);				
				gtk_widget_set_size_request(pStatusBarElementLabel, requisition.width, -1);
				
				gtk_box_pack_start(GTK_BOX(m_wStatusBar), pStatusBarElement, FALSE, FALSE, 0);*/
			}
			else { // fill
		//		gtk_box_pack_start(GTK_BOX(m_wStatusBar), pStatusBarElement, TRUE, TRUE, 0);
			}
		}
		else {
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN); // there are no other kinds of elements
		}
	}
	PtRealizeWidget(m_wStatusBar);
			
	return m_wStatusBar;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
	
void AP_QNXStatusBar::show(void)
{
	PtRealizeWidget (m_wStatusBar);
}

void AP_QNXStatusBar::hide(void)
{
	PtRealizeWidget (m_wStatusBar);
			
	m_pFrame->queue_resize();
}
