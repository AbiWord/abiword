/* AbiWord
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


#ifndef AP_DIALOG_IDSET_H
#define AP_DIALOG_IDSET_H

/* the following Id's must start at zero and be contiguous */

typedef enum _Ap_Dialog_Id
{
	AP_DIALOG_ID__BOGUS1__ = 0,			/* must be first */

	AP_DIALOG_ID_MESSAGE_BOX,
	AP_DIALOG_ID_FILE_OPEN,
	AP_DIALOG_ID_FILE_SAVE,
	AP_DIALOG_ID_FILE_SAVEAS,
	AP_DIALOG_ID_FILE_PAGESETUP,
	AP_DIALOG_ID_FILE_PRINT,

	AP_DIALOG_ID_WINDOW_MORE,

	/* ... add others here ... */

	AP_DIALOG_ID__BOGUS2__				/* must be last */

};

#endif /* AP_DIALOG_IDSET_H */
