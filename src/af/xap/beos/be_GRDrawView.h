
#ifndef _GRDRAWVIEW_H_
#define _GRDRAWVIEW_H_

#include "xap_BeOSFrame.h"
#include "xav_View.h"
#include <View.h>

class be_GRDrawView: public BView {
	public: 
		be_GRDrawView(AV_View *pView, BRect frame, const char *name, 
			      uint32 resizeMask, uint32 flags);
		virtual void SetView(AV_View *pView);
		virtual	void Draw(BRect updateRect);
		virtual	void FrameResized(float new_width, float new_height);

	AV_View *	m_pView;
};

#endif
