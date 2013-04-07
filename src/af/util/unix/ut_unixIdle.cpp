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

#include "ut_unixIdle.h"
#include "ut_assert.h"

#ifndef TOOLKIT_COCOA
//
// timer procedure callback
//
static gint _Timer_Proc(void *p)
{
  UT_UnixIdle * pIdle = static_cast<UT_UnixIdle*>(p);
  UT_ASSERT(pIdle);

  pIdle->fire();

  return true;
}
#endif

/*!
 * Returns a new UT_Idle
 */
UT_Idle * UT_Idle::static_constructor ( UT_WorkerCallback cb, void *data )
{
  return new UT_UnixIdle ( cb, data );
}

/*!
 * Constructs a new Unix idle
 */
UT_UnixIdle::UT_UnixIdle ( UT_WorkerCallback cb, void * data )
  : UT_Idle ( cb, data ), m_id(-1)
{
}

/*!
 * Destructor. Will stop() the idle
 */
UT_UnixIdle::~UT_UnixIdle ()
{
  if ( m_id > 0 )
    stop ();
}

/*!
 * Start this idle running
 */
void UT_UnixIdle::start ()
{
//
// Sevior: Need to ignore this if the idle is already running.
//
// UT_ASSERT(m_id == -1);
	if(m_id == -1)
	{  
#ifndef TOOLKIT_COCOA
	        m_id = g_idle_add_full(G_PRIORITY_LOW,_Timer_Proc, this,NULL);
#else
		m_id = -1;
#endif
	}
	UT_ASSERT(m_id > 0);
}

/*!
 * Stop this idle from running
 */
void UT_UnixIdle::stop ()
{
//
// Sevior: Once again we have to ignore this if the idle is already stopped.
//    UT_ASSERT(m_id > 0);
	if(m_id > 0)
	{
#ifndef TOOLKIT_COCOA
		gboolean b = g_idle_remove_by_data(this);
		UT_UNUSED(b);
		UT_ASSERT(TRUE == b);
#else
		UT_ASSERT (UT_NOT_IMPLEMENTED);
#endif
	}
	m_id = -1;
}
