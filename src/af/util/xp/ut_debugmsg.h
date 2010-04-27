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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef UT_DEBUGMSG_H
#define UT_DEBUGMSG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

ABI_EXPORT void _UT_OutputMessage(const char *s, ...) ABI_PRINTF_FORMAT(1,2);

#ifdef DEBUG
#define UT_DEBUGMSG(M) _UT_OutputMessage M
#else
#define UT_DEBUGMSG(M)
#endif

// define a quick way to no-op a debug message that
// you want to keep and without having to #if 0 it.

#define xxx_UT_DEBUGMSG(M)


/*
 * Similar to UT_DEBUGMSG, except exists even in production (non-debug) builds
 */
#ifdef _WIN32
ABI_EXPORT void _UT_WarningMessage(const char *s, ...);
#else
#define _UT_WarningMessage _UT_OutputMessage
#endif
#define UT_WARNINGMSG(M) _UT_WarningMessage M


#endif /* UT_DEBUGMSG_H */
