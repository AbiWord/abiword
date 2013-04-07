/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
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



#ifndef UT_WIN32IDLE_H
#define UT_WIN32IDLE_H

#include "ut_idle.h"
#include "ut_vector.h"

class ABI_EXPORT UT_Win32Idle : public UT_Idle
{
public:
	UT_Win32Idle(UT_WorkerCallback pCallback, void* pData);
	~UT_Win32Idle();

	virtual void stop(void);
	virtual void start(void);

	static bool _isEmpty();
	static void _fireall();
protected:
	static void _register(UT_Idle *);
	static void _unregister(UT_Idle *);

private:
	static UT_Vector static_vecIdles;

	bool m_bRunning;
};

#endif /* UT_WIN32IDLE_H */

