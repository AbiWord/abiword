/* AbiSource Program Utilities
 * Copyright (C) 2001 Dom Lachowicz <dominicl@seas.upenn.edu>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include "ut_worker.h"

/*!
 * Public virtual destructor
 */
UT_Worker::~UT_Worker ()
{
}

/*!
 * Protected constructor for this base-class
 */
UT_Worker::UT_Worker ()
  : m_pInstanceData(0), m_pCallback(0)
{
}

/*!
 * Protected constructor for this base-class, accepts
 * instance data and callback
 */
UT_Worker::UT_Worker (UT_WorkerCallback cb, void * data)
  : m_pInstanceData(data), m_pCallback(cb)
{
}

/*!
 * Sets the worker's callback function to \param cb
 */
void UT_Worker::_setCallback(UT_WorkerCallback cb)
{
  m_pCallback = cb;
}

/*!
 * Gets the worker's callback function
 */
UT_WorkerCallback UT_Worker::getCallback() const
{
  return m_pCallback;
}

/*!
 * Sets the worker's instance data to \param data
 */
void UT_Worker::_setInstanceData(void * data)
{
  m_pInstanceData = data;
}

/*!
 * Gets the worker's instance data
 */
void* UT_Worker::getInstanceData() const
{
  return m_pInstanceData;
}
