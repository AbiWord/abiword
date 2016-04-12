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

#ifndef UT_WORKER_H
#define UT_WORKER_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

class UT_Worker;

class GR_Graphics;

typedef void (*UT_WorkerCallback)(UT_Worker* pWorker);

/*!
 * Class which is used to construct new instances of
 * UT_Workers based on a passed mode, whose choices
 * can be ORed together
 */
class ABI_EXPORT UT_WorkerFactory
{
 public:
  enum ConstructMode {
    NONE   = 0x00,
    IDLE   = 0x01,
    TIMER  = 0x02
  };

  //CAN_USE_THREAD   = 0x04

  static UT_Worker * static_constructor ( UT_WorkerCallback cb, void * data,
					  int wantMode,
					  UT_WorkerFactory::ConstructMode & outMode );

 private:
  UT_WorkerFactory ();
  ~UT_WorkerFactory ();
};

/*!
 * This class is a generic "worker" class which will
 * serve as a base-class for UT_Timers and UT_Idles
 */
class ABI_EXPORT UT_Worker
{

 public:
  virtual ~UT_Worker ();

  virtual void stop(void) = 0;		//! suspend events
  virtual void start(void) = 0;		//! resume events

  virtual void fire(void);              //! fire off an event

  UT_WorkerCallback getCallback() const;
  void* getInstanceData() const;

 protected:

  UT_Worker ();
  UT_Worker (UT_WorkerCallback cb, void * data);

  void _setCallback(UT_WorkerCallback cb);
  void _setInstanceData(void * data);

 private:
  UT_Worker (UT_Worker &); // no impl

  void * m_pInstanceData;
  UT_WorkerCallback m_pCallback;

};

#endif /* UT_WORKER_H */
