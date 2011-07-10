#ifndef GR_VIEW_DOUBLE_BUFFERING_H
#define GR_VIEW_DOUBLE_BUFFERING_H

#include "gr_Graphics.h"
#include "gr_Painter.h"
#include "fv_View.h"
#include "ut_types.h"

class GR_ViewDoubleBuffering
{

public:
	GR_ViewDoubleBuffering(FV_View *pView, bool suspendDirectDrawing, bool callDrawOnlyAtTheEnd);
	~GR_ViewDoubleBuffering();

	void beginDoubleBuffering();
	void endDoubleBuffering();
	bool getCallDrawOnlyAtTheEnd();
	void recordViewDrawCall(
		UT_sint32 x, UT_sint32 y, 
		UT_sint32 width, UT_sint32 height, 
		bool bDirtyRunsOnly, bool bClip);

private:
	GR_Painter *m_pPainter; // used for accessing double buffering code in GR_Graphics
	FV_View *m_pView; // used to handle calls to _draw

	bool m_bCallDrawOnlyAtTheEnd;
	bool m_bSuspendDirectDrawing;

	void callUnifiedDraw();
	void redrawEntireScreen();

	struct ViewDrawFunctionArguments
	{
		UT_sint32 x1;
		UT_sint32 y1;
		UT_sint32 x2;
		UT_sint32 y2;
		bool bDirtyRunsOnly;
		bool bClip;
		UT_sint32 callCount;
	};

	ViewDrawFunctionArguments mostExtArgs;
	void initMostExtArgs();
	void GR_ViewDoubleBuffering::extendDrawArgsIfNeccessary(
		UT_sint32 x, UT_sint32 y, 
		UT_sint32 width, UT_sint32 height, 
		bool bDirtyRunsOnly, bool bClip);
};

#endif
