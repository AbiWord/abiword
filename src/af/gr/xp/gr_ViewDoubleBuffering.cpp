#include "gr_ViewDoubleBuffering.h"

#include "gr_Graphics.h"
#include "gr_Painter.h"
#include "fv_View.h"

GR_ViewDoubleBuffering::GR_ViewDoubleBuffering(FV_View *pView, bool suspendDirectDrawing, bool callDrawOnlyAtTheEnd)
	: m_pView(pView),
	  m_bSuspendDirectDrawing(suspendDirectDrawing),
	  m_bCallDrawOnlyAtTheEnd(callDrawOnlyAtTheEnd)
{
	// We will need a painter to call double buffering code in GR_Graphics	
	m_pPainter = new GR_Painter(pView->getGraphics());
}

GR_ViewDoubleBuffering::~GR_ViewDoubleBuffering()
{
	delete m_pPainter;
}
