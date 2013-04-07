/* AbiWord
 * Copyright (C) 2011 AbiSource, Inc.
 * Copyright (C) Ben Martin
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

#ifndef AP_UNIXDIALOG_RDFEDITOR_H
#define AP_UNIXDIALOG_RDFEDITOR_H

#include "ap_Dialog_RDFEditor.h"
#include "fv_View.h"

class XAP_UnixFrame;



class AP_UnixDialog_RDFEditor: public AP_Dialog_RDFEditor
{
public:
	AP_UnixDialog_RDFEditor (XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_RDFEditor (void);

	static XAP_Dialog *static_constructor (XAP_DialogFactory *,
										   XAP_Dialog_Id id);

	virtual void runModeless 	   (XAP_Frame *pFrame);
	virtual void notifyActiveFrame (XAP_Frame *pFrame);
	virtual void activate	 	   (void);
	virtual void destroy 		   (void);

    void onExecuteClicked();
    void onShowAllClicked();
    void onDelClicked();
    void onCellEdited( GtkCellRendererText *cell,
                       gchar *path_string,
                       gchar *new_text,
                       int cidx );
    void onImportRDFXML();
    void onExportRDFXML();
    void onCursorChanged();

	const GtkWidget *getWindow (void) { return m_wDialog; }

    virtual void clear();
    virtual void addStatement( const PD_RDFStatement& st );
    virtual void setStatus( const std::string& msg );
    virtual void removeStatement( const PD_RDFStatement& st );
    virtual std::list< PD_RDFStatement > getSelection();
    virtual void setSelection( const std::list< PD_RDFStatement >& l );
    virtual void hideRestrictionXMLID( bool v );

    PD_RDFStatement next( const PD_RDFStatement& st );

protected:

	void _constructWindow 	  (XAP_Frame *pFrame);
	void _updateWindow		  (void);

private:

    enum
    {
        C_SUBJ_COLUMN = 0,
        C_PRED_COLUMN,
        C_OBJ_COLUMN,
		C_COLUMN_COUNT
    };

    GtkTreeViewColumn* w_cols[C_COLUMN_COUNT];

	GtkWidget *m_wDialog;
	GtkWidget *m_btClose;
    GtkWidget *m_btShowAll;
	GtkTreeView*  m_resultsView;
	GtkTreeStore* m_resultsModel;
    GtkWidget *m_status;
    GtkAction *m_anewtriple;
    GtkAction *m_acopytriple;
    GtkAction *m_adeletetriple;
    GtkAction *m_aimportrdfxml;
    GtkAction *m_aexportrdfxml;
    GtkComboBox *m_selectedxmlid;
    GtkWidget   *m_restrictxmlidhidew;

    GtkTreeIter getGIter( PD_RDFStatement st );
    PD_RDFStatement GIterToStatement( GtkTreeIter* giter );

};

#endif
