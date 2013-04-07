/* AbiWord
 * Copyright (C) 2009 Hubert Figuiere
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


#ifndef _XAP_GTKSIGNALBLOCK_H_
#define _XAP_GTKSIGNALBLOCK_H_


/** A simple signal blocker that is scoped.
 */
class XAP_GtkSignalBlocker
{
public:
	XAP_GtkSignalBlocker(GObject * _obj, int _signal_id)
		: m_id(_signal_id)
		, m_obj(_obj)
		{
			g_signal_handler_block(m_obj, m_id);
		}

	~XAP_GtkSignalBlocker()
		{
			g_signal_handler_unblock(m_obj, m_id);
		}
private:
	int m_id;
	GObject * m_obj;
	// make sure this is NEVER called.
	XAP_GtkSignalBlocker(const XAP_GtkSignalBlocker &);
};


#endif

