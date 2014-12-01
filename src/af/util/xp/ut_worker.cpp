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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ut_worker.h"
#include "ut_assert.h"

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

/*!
 * Fires off the event with the proper callback data
 */
void UT_Worker::fire()
{
  UT_ASSERT(m_pCallback);
  if (m_pCallback) {
    m_pCallback(this);
  }
}

/****************************************************************************/
/****************************************************************************/

#include "ut_timer.h"
#include "ut_idle.h"
#include "ut_debugmsg.h"

/*!
 * Private c'tor
 */
UT_WorkerFactory::UT_WorkerFactory()
{
}

/*!
 * Private d'tor
 */
UT_WorkerFactory::~UT_WorkerFactory()
{
}

/*!
 * Constructs a new UT_Worker object with the callback \param cb
 * and \param data. It will try to create the type of UT_Worker
 * specified by \param mode. The mode values can be ORed together.
 * IDLE is given preference over TIMER. \return a valid UT_Worker
 * on success, NULL on failure. \param outMode will be set to the
 * type of object constructed, so that you can static_cast<> it to
 * the proper type for further manipulation, should it be needed
 */
UT_Worker * UT_WorkerFactory::static_constructor ( UT_WorkerCallback cb, 
						   void * data, int mode,
						   UT_WorkerFactory::ConstructMode & outMode )
{

  UT_Worker * tmp = 0;
  
  // give preference to CAN_USE_IDLE

#if defined(SUPPORTS_UT_IDLE) || defined(TOOLKIT_GTK_ALL)
  if ( mode & IDLE )
  {
      tmp = UT_Idle::static_constructor ( cb, data );
      outMode = IDLE;
  }
  else
#endif
  if ( mode & TIMER )
  {
    tmp = UT_Timer::static_constructor ( cb, data );
    outMode = TIMER;
  }
  else
  {
      UT_DEBUGMSG(("UNKNOWN MODE: %d\n", mode));
      outMode = NONE;
  }


  UT_ASSERT(tmp != 0);
  return tmp;
}
