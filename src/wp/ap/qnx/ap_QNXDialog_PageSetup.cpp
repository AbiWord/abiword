/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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

#include "ap_QNXDialog_PageSetup.h"

AP_QNXDialog_PageSetup::AP_QNXDialog_PageSetup(XAP_DialogFactory * pDlgFactory, 
			       XAP_Dialog_Id id) :
  AP_Dialog_PageSetup (pDlgFactory, id)
{
}

AP_QNXDialog_PageSetup::~AP_QNXDialog_PageSetup(void)
{
}

XAP_Dialog * AP_QNXDialog_PageSetup::static_constructor(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
{
  AP_QNXDialog_PageSetup * dlg = new AP_QNXDialog_PageSetup (pDlgFactory, id);
  return dlg;
}

void AP_QNXDialog_PageSetup::runModal (XAP_Frame *pFrame)
{
  return;
}
