/* AbiWord
 * Copyright (C) 2005 Hubert Figuiere
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


#include <errno.h>
#include "tf_test.h"

#include "ut_types.h"

#define TFSUITE "core.af.util.types"

TFTEST_MAIN("UT_errnoToUTError")
{
	errno = EACCES;
	TFPASS(UT_errnoToUTError() == UT_IE_PROTECTED);

	errno = ENOENT;
	TFPASS(UT_errnoToUTError() == UT_IE_FILENOTFOUND);

	errno = ENOMEM;
	TFPASS(UT_errnoToUTError() == UT_OUTOFMEM);

	errno = EMFILE;
	TFPASS(UT_errnoToUTError() == UT_IE_COULDNOTOPEN);

	errno = EROFS;
	TFPASS(UT_errnoToUTError() == UT_IE_COULDNOTWRITE);

	errno = ENOSPC;
	TFPASS(UT_errnoToUTError() == UT_IE_COULDNOTWRITE);

	errno = 0;
	TFPASS(UT_errnoToUTError() == UT_OK);

	errno = EINVAL;
	TFPASS(UT_errnoToUTError() == UT_ERROR);
}
