#include <stdio.h>
#include "be_GRDrawView.h"

/*
 This is a generic drawing class ...
*/
be_GRDrawView::be_GRDrawView(AV_View *pView, BRect frame, const char *name, 
			     uint32 resizeMask, uint32 flags)
	:BView(frame, name, resizeMask, flags | B_FRAME_EVENTS) {

	m_pView = pView;
}

void be_GRDrawView::SetView(AV_View *pView) {
	m_pView = pView;
}

void be_GRDrawView::FrameResized(float new_width, float new_height) {
	//We should only do this after things are stableized
	if (m_pView) {
		//BRect r;
		printf("GRDRAWVIEW: Resize Redrawing \n");
		//m_pView->setWindowSize(new_width, new_height);
		//m_pView->draw(NULL);
	}
}

void be_GRDrawView::Draw(BRect updateRect) {
	printf("GRDRAWVIEW: Draw request entered \n");
	if (m_pView) {
		printf("GRDRAWVIEW: Draw request actually requested \n");
		//TODO: Make the update more succinct with a rect
		//BRect r = Bounds();
   		UT_Rect rect(updateRect.left,updateRect.top, updateRect.Width()+1, updateRect.Height()+1);
		m_pView->draw(&rect);
		//m_pView->draw(NULL);
	}
}

