#include "gr_ViewDoubleBuffering.h"

#include "gr_Graphics.h"
#include "gr_Painter.h"
#include "fv_View.h"

GR_ViewDoubleBuffering::GR_ViewDoubleBuffering(FV_View *pView, bool suspendDirectDrawing, bool callDrawOnlyAtTheEnd)
	: m_pView(pView),
	  m_bSuspendDirectDrawing(suspendDirectDrawing),
	  m_bCallDrawOnlyAtTheEnd(callDrawOnlyAtTheEnd)
{
}

GR_ViewDoubleBuffering::~GR_ViewDoubleBuffering()
{
	this->endDoubleBuffering();
}

void GR_ViewDoubleBuffering::beginDoubleBuffering()
{
	// We will need to direct calls through a painter since it may initialize
	// the device context on some platforms
	m_pPainter = new GR_Painter(m_pView->getGraphics());

	if(m_bSuspendDirectDrawing == true && m_bCallDrawOnlyAtTheEnd == false)
	{
		// You put me in a curious sitution: you want me to disable drawing between
		// beginDoubleBuffering and endDoubleBuffering, but you also want to draw
		// through _draw. At the moment, we cannot do that.
		UT_ASSERT(UT_NOT_IMPLEMENTED);
	}
	
	if(!m_pView->registerDoubleBufferingObject(this))
		return;

	if(m_bSuspendDirectDrawing)
		m_pView->getGraphics()->suspendDrawing();
	
	m_pPainter->beginDoubleBuffering();
}

void GR_ViewDoubleBuffering::endDoubleBuffering()
{
	if(!m_pView->unregisterDoubleBufferingObject(this))
		return;

	if(m_bSuspendDirectDrawing)
		m_pView->getGraphics()->resumeDrawing();

	if(m_bCallDrawOnlyAtTheEnd)
		this->callUnifiedDraw();

	m_pPainter->endDoubleBuffering();

	delete m_pPainter;
}

bool GR_ViewDoubleBuffering::getCallDrawOnlyAtTheEnd()
{
	return m_bCallDrawOnlyAtTheEnd;
}

void GR_ViewDoubleBuffering::callUnifiedDraw()
{
	this->m_pView->_draw(
		0, 0, 
		m_pView->getWindowWidth(), m_pView->getWindowHeight(), 
		false, false);
}
