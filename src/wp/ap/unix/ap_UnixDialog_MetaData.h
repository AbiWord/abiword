/* AbiWord
 * Copyright (C) 2002 Dom Lachowicz
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

#ifndef AP_UNIXDIALOG_METADATA_H
#define AP_UNIXDIALOG_METADATA_H

#include "xap_UnixDialogHelper.h"
#include "ap_Dialog_MetaData.h"

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_MetaData: public AP_Dialog_MetaData
{
 public:
  AP_UnixDialog_MetaData(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
  virtual ~AP_UnixDialog_MetaData(void);
  
  virtual void			runModal(XAP_Frame * pFrame);
  
  static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

 protected:

  void eventCancel () ;
  void eventOK () ;

  virtual GtkWidget * _constructWindow () ;

  void _constructWindowContents ( GtkWidget * container ) ;

 private:  

	typedef enum
		{
			BUTTON_OK,
			BUTTON_CANCEL
		} ResponseId ;
	
  GtkWidget * m_entryTitle ;
  GtkWidget * m_entrySubject ;
  GtkWidget * m_entryAuthor ;
  GtkWidget * m_entryPublisher ;
  GtkWidget * m_entryCoAuthor ;
  GtkWidget * m_entryCategory ;
  GtkWidget * m_entryKeywords ;
  GtkWidget * m_entryLanguages ;
  GtkWidget * m_entrySource ;
  GtkWidget * m_entryRelation ;
  GtkWidget * m_entryCoverage ;
  GtkWidget * m_entryRights ;

  GtkWidget * m_textDescription ;
};

#endif /* AP_UNIXDIALOG_METADATA_H */
