/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 *
 * Copyright (C) 2002 AbiSource, Inc.
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

#ifndef XAP_LOG
#define XAP_LOG

#include <stdio.h>

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

#include <string>

class AV_View;

class EV_EditMethodCallData;

class ABI_EXPORT XAP_Log
{
public:
	void log(const std::string &method_name, AV_View *pAV_View, EV_EditMethodCallData *pCallData);
	static XAP_Log *get_instance();
	virtual ~XAP_Log();

private:
	XAP_Log(const std::string &logfile);
	FILE *m_pOutput;
	static XAP_Log *m_pInstance;
};

#endif /* XAP_LOG */
