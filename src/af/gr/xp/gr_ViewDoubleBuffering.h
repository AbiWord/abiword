#ifndef GR_VIEW_DOUBLE_BUFFERING_H
#define GR_VIEW_DOUBLE_BUFFERING_H

#include "gr_Graphics.h"
#include "gr_Painter.h"
#include "fv_View.h"

class GR_ViewDoubleBuffering
{

public:
	GR_ViewDoubleBuffering(FV_View *pView, bool suspendDirectDrawing, bool callDrawOnlyAtTheEnd);
	~GR_ViewDoubleBuffering();

private:
	GR_Painter *m_pPainter; // used for accessing double buffering code in GR_Graphics
	FV_View *m_pView; // used to handle calls to _draw

	bool m_bCallDrawOnlyAtTheEnd;
	bool m_bSuspendDirectDrawing;
};

#endif
