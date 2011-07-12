#include "fv_ViewDoubleBuffering.h"

#include "gr_Graphics.h"
#include "gr_Painter.h"
#include "fv_View.h"
#include "ut_types.h"
#include "ut_misc.h"
#include "ut_debugmsg.h"

// #define DEACTIVATE_FV_VIEW_DOUBLE_BUFFERING

FV_ViewDoubleBuffering::FV_ViewDoubleBuffering(FV_View *pView, bool suspendDirectDrawing, bool callDrawOnlyAtTheEnd)
	: m_pView(pView),
	  m_bSuspendDirectDrawing(suspendDirectDrawing),
	  m_bCallDrawOnlyAtTheEnd(callDrawOnlyAtTheEnd)
{
	this->initMostExtArgs();
}

FV_ViewDoubleBuffering::~FV_ViewDoubleBuffering()
{
	this->endDoubleBuffering();
}

void FV_ViewDoubleBuffering::beginDoubleBuffering()
{

#ifdef DEACTIVATE_FV_VIEW_DOUBLE_BUFFERING
	return;
#endif

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

	m_pPainter->beginDoubleBuffering();

	if(m_bSuspendDirectDrawing)
		m_pPainter->suspendDrawing();
}

void FV_ViewDoubleBuffering::endDoubleBuffering()
{

#ifdef DEACTIVATE_FV_VIEW_DOUBLE_BUFFERING
	return;
#endif

	if(!m_pView->unregisterDoubleBufferingObject(this))
		return;

	if(m_bSuspendDirectDrawing)
		m_pPainter->resumeDrawing();

	if(m_bCallDrawOnlyAtTheEnd)
		this->callUnifiedDraw();

	m_pPainter->endDoubleBuffering();

	delete m_pPainter;
}

void FV_ViewDoubleBuffering::recordViewDrawCall(
		UT_sint32 x, UT_sint32 y, 
		UT_sint32 width, UT_sint32 height, 
		bool bDirtyRunsOnly, bool bClip)
{
	this->extendDrawArgsIfNeccessary(x, y, width, height, bDirtyRunsOnly, bClip);
}

bool FV_ViewDoubleBuffering::getCallDrawOnlyAtTheEnd()
{
	return m_bCallDrawOnlyAtTheEnd;
}

void FV_ViewDoubleBuffering::callUnifiedDraw()
{
	UT_sint32 width = mostExtArgs.x2 - mostExtArgs.x1;
	UT_sint32 height = mostExtArgs.y2 - mostExtArgs.y1;

	if(mostExtArgs.callCount > 0)
	{
		this->m_pView->_draw(
			mostExtArgs.x1, mostExtArgs.y1,
			width, height,
			mostExtArgs.bDirtyRunsOnly, mostExtArgs.bClip);
	}

	UT_DEBUGMSG(("ASFRENT: unified _draw call for a total of %d previous calls.\n",  mostExtArgs.callCount));
}

void FV_ViewDoubleBuffering::redrawEntireScreen()
{
	this->m_pView->_draw(
		0, 0,
		m_pView->getWindowWidth(), m_pView->getWindowHeight(),
		false, false);
}

void FV_ViewDoubleBuffering::initMostExtArgs() 
{
	mostExtArgs.callCount = 0;
}

void FV_ViewDoubleBuffering::extendDrawArgsIfNeccessary(
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

