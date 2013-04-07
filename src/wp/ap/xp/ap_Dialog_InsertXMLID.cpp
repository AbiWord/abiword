/* AbiWord
 * Copyright (C) 2011 Ben Martin
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

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "ap_Dialog_InsertXMLID.h"

AP_Dialog_InsertXMLID::AP_Dialog_InsertXMLID( XAP_DialogFactory * pDlgFactory,
                                              XAP_Dialog_Id id )
  : AP_Dialog_GetStringCommon( pDlgFactory,id, "interface/dialogxmlid" )
{
}

AP_Dialog_InsertXMLID::~AP_Dialog_InsertXMLID(void)
{
}
