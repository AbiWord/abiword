/* AbiSource Application Framework
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

#ifndef XAP_ARGS_H
#define XAP_ARGS_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

class ABI_EXPORT XAP_Args
{
	friend class AP_Args;

public:
	XAP_Args(int argc, char ** argv);	/* for systems which cut up the command line for us */
	XAP_Args(const char * szCmdLine);	/* for systems which give one big arg */
	~XAP_Args(void);

private:
	int	m_argc;
	char **	m_argv;

	bool m_bAllocated;
	char *	m_szBuf;
};

#endif /* XAP_ARGS_H */
