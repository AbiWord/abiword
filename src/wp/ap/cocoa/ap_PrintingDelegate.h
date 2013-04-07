/* AbiWord
 * Copyright (C) 2003 Hubert Figuiere
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


#ifndef __AP_PRINTING_DELEGATE__
#define __AP_PRINTING_DELEGATE__

#include "xap_PrintingDelegate.h"

class FV_View;

class AP_PrintingDelegate
	: public XAP_PrintingDelegate
{
public:
	AP_PrintingDelegate(FV_View* pView);
	virtual int getPageCount(void);
	virtual void printPage(int pageNum);

private:
	FV_View* m_pView;
};

#endif


