#include "gr_ViewDoubleBuffering.h"

#include "gr_Graphics.h"
#include "gr_Painter.h"
#include "fv_View.h"
#include "ut_types.h"
#include "ut_misc.h"
#include "ut_debugmsg.h"

GR_ViewDoubleBuffering::GR_ViewDoubleBuffering(FV_View *pView, bool suspendDirectDrawing, bool callDrawOnlyAtTheEnd)
	: m_pView(pView),
	  m_bSuspendDirectDrawing(suspendDirectDrawing),
	  m_bCallDrawOnlyAtTheEnd(callDrawOnlyAtTheEnd)
{
	this->initMostExtArgs();
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

void GR_ViewDoubleBuffering::recordViewDrawCall(
		UT_sint32 x, UT_sint32 y, 
		UT_sint32 width, UT_sint32 height, 
		bool bDirtyRunsOnly, bool bClip)
{
	this->extendDrawArgsIfNeccessary(x, y, width, height, bDirtyRunsOnly, bClip);
}

bool GR_ViewDoubleBuffering::getCallDrawOnlyAtTheEnd()
{
	return m_bCallDrawOnlyAtTheEnd;
}

void GR_ViewDoubleBuffering::callUnifiedDraw()
{
	UT_sint32 width = mostExtArgs.x2 - mostExtArgs.x1;
	UT_sint32 height = mostExtArgs.y2 - mostExtArgs.y1;

//	this->m_pView->_draw(
//		mostExtArgs.x1, mostExtArgs.y1,
//		width, height,
//		mostExtArgs.bDirtyRunsOnly, mostExtArgs.bClip);

	this->redrawEntireScreen();

	UT_DEBUGMSG(("ASFRENT: unified _draw call for a total of %d previous calls.\n",  mostExtArgs.callCount));
}

void GR_ViewDoubleBuffering::redrawEntireScreen()
{
	this->m_pView->_draw(
		0, 0,
		m_pView->getWindowWidth(), m_pView->getWindowHeight(),
		false, false);
}

void GR_ViewDoubleBuffering::initMostExtArgs() 
{
	mostExtArgs.callCount = 0;
}

void GR_ViewDoubleBuffering::extendDrawArgsIfNeccessary(
	UT_sint32 x, UT_sint32 y, 
	UT_sint32 width, UT_sint32 height, 
	bool bDirtyRunsOnly, bool bClip)
{
	if(mostExtArgs.callCount == 0)
	{
		// then this is a first call, just record parameters
		mostExtArgs.x1 = x;
		mostExtArgs.y1 = y;
		mostExtArgs.x2 = x + width;
		mostExtArgs.y2 = y + height;
		mostExtArgs.bDirtyRunsOnly = bDirtyRunsOnly;
		mostExtArgs.bClip = bClip;
	}
	else
	{
		// extend those args
		
		// 1. dirty runs: false means more
		if(bDirtyRunsOnly == false) mostExtArgs.bDirtyRunsOnly = false;

		// 2. clipping region: false means entire view  [I hope so]
		if(bClip == false) mostExtArgs.bClip = false;

		// 3. rectangle
		mostExtArgs.x1 = UT_MIN(mostExtArgs.x1, x);
		mostExtArgs.y1 = UT_MIN(mostExtArgs.y1, y);
		mostExtArgs.x2 = UT_MAX(mostExtArgs.x2, x + width);
		mostExtArgs.y2 = UT_MAX(mostExtArgs.y2, y + height);
	}
	
	mostExtArgs.callCount++;
}
