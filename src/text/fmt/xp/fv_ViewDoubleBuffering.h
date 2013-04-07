/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef FV_VIEW_DOUBLE_BUFFERING_H
#define FV_VIEW_DOUBLE_BUFFERING_H

#include "gr_Graphics.h"
#include "gr_Painter.h"
#include "fv_View.h"
#include "ut_types.h"

class FV_ViewDoubleBuffering
{

public:
	FV_ViewDoubleBuffering(FV_View *pView, bool suspendDirectDrawing, bool callDrawOnlyAtTheEnd);
	~FV_ViewDoubleBuffering();

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
		UT_Rect clipRect;
		UT_Rect fullRect;
		bool bDirtyRunsOnly;
		bool bClip;
		UT_sint32 callCount;
	};

	ViewDrawFunctionArguments mostExtArgs;
	bool noRecordedDrawCalls();
	void initMostExtArgs();
	void extendDrawArgsIfNeccessary(
		UT_Rect *thisCallRect,
		const UT_Rect *clipRectFromGraphics,
		bool bDirtyRunsOnly);
};

#endif
