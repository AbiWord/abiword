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


#ifndef _XAP_GTKOBJECTHOLDER_H_
#define _XAP_GTKOBJECTHOLDER_H_


/** A simple class to hold a scoped ref on a GObject */
template <class T>
class XAP_GtkObjectHolder
{
public:
	XAP_GtkObjectHolder(const XAP_GtkObjectHolder<T> &_obj)
		: m_obj(_obj.m_obj)
		{
			_ref();
		}
	XAP_GtkObjectHolder(T * _obj = NULL)
		: m_obj(_obj)
		{
			_ref();
		}
	~XAP_GtkObjectHolder()
		{
			_unref();
		}
	XAP_GtkObjectHolder<T> & operator=(T * _obj)
		{
			_unref();
			m_obj = _obj;
			_ref();
			return *this;
		}
	XAP_GtkObjectHolder<T> & operator=(const XAP_GtkObjectHolder<T> & _obj)
		{
			_unref();
			m_obj = _obj.m_obj;
			_ref();
			return *this;
		}
	T * obj() const
		{
			return m_obj;
		}
private:
	void _unref()
		{
			if(m_obj)
				g_object_unref(m_obj);
		}
	void _ref()
		{
			if(m_obj)
				g_object_ref(m_obj);
		}
	T* m_obj;
};


#endif

