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
	UT_Rect thisCallRect(x, y, width, height);
	const UT_Rect *clipRectFromGraphics = m_pView->getGraphics()->getClipRect();
	// record actual _draw arguments
	this->extendDrawArgsIfNeccessary(&thisCallRect, clipRectFromGraphics, bDirtyRunsOnly);
}

bool FV_ViewDoubleBuffering::getCallDrawOnlyAtTheEnd()
{
	return m_bCallDrawOnlyAtTheEnd;
}

void FV_ViewDoubleBuffering::callUnifiedDraw()
{
	if(noRecordedDrawCalls()) return;

	m_pView->getGraphics()->setClipRect(&mostExtArgs.clipRect);
	m_pView->_draw(
		mostExtArgs.fullRect.left, mostExtArgs.fullRect.top,
		mostExtArgs.fullRect.width, mostExtArgs.fullRect.height,
		mostExtArgs.bDirtyRunsOnly, false);
	m_pView->getGraphics()->setClipRect(NULL);

	UT_DEBUGMSG(("ASFRENT: unified _draw call for a total of %d previous calls.\n",  mostExtArgs.callCount));
}

bool FV_ViewDoubleBuffering::noRecordedDrawCalls()
{
	return mostExtArgs.callCount == 0;
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
	UT_Rect *thisCallRect,
	const UT_Rect *clipRectFromGraphics,
	bool bDirtyRunsOnly)
{
	if(clipRectFromGraphics == NULL)
		clipRectFromGraphics = thisCallRect;

	if(mostExtArgs.callCount == 0)
	{
		// then this is a first call, just record parameters
		mostExtArgs.bDirtyRunsOnly = bDirtyRunsOnly;
		mostExtArgs.fullRect = *thisCallRect;
		mostExtArgs.clipRect = *clipRectFromGraphics;
	}
	else
	{
		// extend those args
		
		// 1. dirty runs: false means more
		if(bDirtyRunsOnly == false) mostExtArgs.bDirtyRunsOnly = false;
		
		// 2. full rectangle
		mostExtArgs.fullRect.unionRect(thisCallRect);

		// 3. clip rectangle
		mostExtArgs.clipRect.unionRect(clipRectFromGraphics);
	}

	mostExtArgs.callCount++;
}

