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

#ifndef AP_QNXDIALOG_PAGENUMBERS_H
#define AP_QNXDIALOG_PAGENUMBERS_H

#include "ut_types.h"
#include "ap_Dialog_PageNumbers.h"

class XAP_QNXFrame;
class GR_QNXGraphics;

class AP_QNXDialog_PageNumbers : public AP_Dialog_PageNumbers
{
 public:
  AP_QNXDialog_PageNumbers(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
  virtual ~AP_QNXDialog_PageNumbers(void);

  virtual void runModal(XAP_Frame * pFrame);

  static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

  void event_OK(void);
  void event_Cancel(void);
  void event_WindowDelete(void);
  void event_PreviewExposed(void);
  void event_AlignChanged(AP_Dialog_PageNumbers::tAlign);
  void event_HdrFtrChanged(AP_Dialog_PageNumbers::tControl);

 protected:

  // private construction functions
  virtual PtWidget_t * _constructWindow (void);

  // caches of the last known values for alignment and hdr/footer/both
  AP_Dialog_PageNumbers::tAlign m_recentAlign;
  AP_Dialog_PageNumbers::tControl m_recentControl;

  PtWidget_t * m_buttonOK;
  PtWidget_t * m_buttonCancel;
  PtWidget_t * m_window;

  PtWidget_t * m_previewArea;
  int		   done;

  UT_Vector	   m_vecalign;
  UT_Vector	   m_vecposition;

  GR_QNXGraphics * m_qnxGraphics;
};

#endif /* AP_QNXDIALOG_PAGENUBMERS_H */
