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

#ifndef AP_BeOSDIALOG_PAGENUMBERS_H
#define AP_BeOSDIALOG_PAGENUMBERS_H

#include "ap_Dialog_PageNumbers.h"

class AP_BeOSDialog_PageNumbers : public AP_Dialog_PageNumbers
{
	friend class PageNumberWindow;
	
public: 

	AP_BeOSDialog_PageNumbers(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_BeOSDialog_PageNumbers (void);
	
	virtual void			setAnswer(AP_Dialog_PageNumbers::tAnswer a) { m_answer = a; };

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	virtual void	runModal(XAP_Frame * pFrame);

protected:

  // caches of the last known values for alignment and hdr/footer/both
  AP_Dialog_PageNumbers::tAlign m_recentAlign;
  AP_Dialog_PageNumbers::tControl m_recentControl;
};

#endif /* AP_BeOSDIALOG_PAGENUMBERS_H */
