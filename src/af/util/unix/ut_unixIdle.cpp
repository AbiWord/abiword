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

#include <glib.h>

#include "ut_unixIdle.h"
#include "ut_assert.h"

//
// timer procedure callback
//
static gint _Timer_Proc(void *p)
{
  UT_UnixIdle * pIdle = static_cast<UT_UnixIdle*>(p);
  UT_ASSERT(pIdle);

  pIdle->fire();

  return TRUE;
}

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
  UT_ASSERT(m_id == -1);
  m_id = g_idle_add(_Timer_Proc, this);
  UT_ASSERT(m_id > 0);
}

/*!
 * Stop this idle from running
 */
void UT_UnixIdle::stop ()
{
  UT_ASSERT(m_id > 0);

  gboolean b = g_idle_remove_by_data(this);
  UT_ASSERT(TRUE == b);
  m_id = -1;
}
