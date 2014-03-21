/* AbiWord - unix impl for selection handles
 * Copyright (c) 2012 One laptop per child
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
 *
 * Author: Carlos Garnacho <carlos@lanedo.com>
 */

#include "xap_Frame.h"
#include "xap_UnixFrameImpl.h"
#include "fv_UnixSelectionHandles.h"
#include "fv_View.h"
#include "gtktexthandleprivate.h"

static void handle_dragged_cb (FvTextHandle         *handle,
                               FvTextHandlePosition  pos,
                               gint                  x,
                               gint                  y,
                               gpointer              user_data)
{
	FvTextHandleMode mode;
	FV_UnixSelectionHandles *handles = static_cast<FV_UnixSelectionHandles *>(user_data);

	mode = _fv_text_handle_get_mode (handle);

	if (pos == FV_TEXT_HANDLE_POSITION_SELECTION_START) {
		handles->updateSelectionStart ((UT_sint32)x, (UT_sint32)y);
        }
	else {
		if (mode == FV_TEXT_HANDLE_MODE_SELECTION) {
			handles->updateSelectionEnd ((UT_sint32)x, (UT_sint32)y);
                }
                else {
			handles->updateCursor((UT_sint32)x, (UT_sint32)y);
                }
	}
}

FV_UnixSelectionHandles::FV_UnixSelectionHandles(FV_View *view, FV_Selection selection)
	: FV_SelectionHandles (view, selection)
	, m_text_handle(NULL)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame*>(m_pView->getParentData());
	// When saving to PDF (and printing) we don't have a frame
	// See bug 13586
	if (pFrame) {
		XAP_UnixFrameImpl * pFrameImpl = static_cast<XAP_UnixFrameImpl *>(pFrame->getFrameImpl());
		GtkWidget * pWidget = pFrameImpl->getViewWidget();

		if (pWidget)
		{
			m_text_handle = _fv_text_handle_new (pWidget);
			_fv_text_handle_set_relative_to (m_text_handle,
							 gtk_widget_get_window (pWidget));
			g_signal_connect (m_text_handle, "handle-dragged",
					  G_CALLBACK(handle_dragged_cb), this);
		}
	}
}

FV_UnixSelectionHandles::~FV_UnixSelectionHandles()
{
	if(!m_text_handle) {
		return;
	}
	g_object_unref (m_text_handle);
}

void FV_UnixSelectionHandles::hide()
{
	if(!m_text_handle) {
		return;
	}
	_fv_text_handle_set_mode (m_text_handle, FV_TEXT_HANDLE_MODE_NONE);
}

void FV_UnixSelectionHandles::setCursorCoords(UT_sint32 x, UT_sint32 y, UT_uint32 height, bool visible)
{
	if(!m_text_handle) {
		return;
	}

	GdkRectangle rect;

	_fv_text_handle_set_mode(m_text_handle, FV_TEXT_HANDLE_MODE_CURSOR);
	_fv_text_handle_set_visible (m_text_handle, FV_TEXT_HANDLE_POSITION_CURSOR, visible);

	if (visible)
	{
		rect.x = (int)x;
		rect.y = (int)y;
		rect.width = 1;
		rect.height = (int)height;
		_fv_text_handle_set_position(m_text_handle,
					     FV_TEXT_HANDLE_POSITION_CURSOR,
					     &rect);
	}
}

void FV_UnixSelectionHandles::setSelectionCoords(UT_sint32 start_x, UT_sint32 start_y, UT_uint32 start_height, bool start_visible,
                                                 UT_sint32 end_x, UT_sint32 end_y, UT_uint32 end_height, bool end_visible)
{
	if(!m_text_handle) {
		return;
	}

	GdkRectangle rect;

	_fv_text_handle_set_mode(m_text_handle, FV_TEXT_HANDLE_MODE_SELECTION);

	_fv_text_handle_set_visible (m_text_handle, FV_TEXT_HANDLE_POSITION_SELECTION_START, start_visible);
	_fv_text_handle_set_visible (m_text_handle, FV_TEXT_HANDLE_POSITION_SELECTION_END, end_visible);

	if (start_visible)
	{
		rect.x = (int)start_x;
		rect.y = (int)start_y;
		rect.width = 1;
		rect.height = (int)start_height;
		_fv_text_handle_set_position(m_text_handle,
					     FV_TEXT_HANDLE_POSITION_SELECTION_START,
					     &rect);
	}

	if (end_visible)
	{
		rect.x = (int)end_x;
		rect.y = (int)end_y;
		rect.width = 1;
		rect.height = (int)end_height;
		_fv_text_handle_set_position(m_text_handle,
					     FV_TEXT_HANDLE_POSITION_SELECTION_END,
					     &rect);
	}
}
