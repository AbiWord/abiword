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

#ifndef AP_UNIXDIALOG_RDFQUERY_H
#define AP_UNIXDIALOG_RDFQUERY_H

#include "ap_Dialog_RDFQuery.h"
#include "fv_View.h"

class XAP_UnixFrame;



class AP_UnixDialog_RDFQuery: public AP_Dialog_RDFQuery
{
public:
	AP_UnixDialog_RDFQuery (XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_RDFQuery (void);

	static XAP_Dialog *static_constructor (XAP_DialogFactory *,
										   XAP_Dialog_Id id);

	virtual void runModeless 	   (XAP_Frame *pFrame);
	virtual void notifyActiveFrame (XAP_Frame *pFrame);
	virtual void activate	 	   (void);
	virtual void destroy 		   (void);

    void onExecuteClicked();
    void onShowAllClicked();


	const GtkWidget *getWindow (void) { return m_wDialog; }

    virtual void clear();
    virtual void addStatement( const PD_RDFStatement& st );
    virtual void addBinding( std::map< std::string, std::string >& b );
    virtual void setStatus( const std::string& msg );
    virtual void setQueryString( const std::string& sparql );
    virtual void setupBindingsView( std::map< std::string, std::string >& b );

protected:

	void _constructWindow 	  (XAP_Frame *pFrame);
	void _updateWindow		  (void);

private:

    enum
    {
        C_SUBJ_COLUMN = 0,
        C_PRED_COLUMN,
        C_OBJ_COLUMN,
		C_COLUMN_COUNT,
        C_COLUMN_ARRAY_SIZE = 1024
	};

    GtkTreeViewColumn* w_cols[C_COLUMN_ARRAY_SIZE];

	GtkWidget *m_wDialog;
	GtkWidget *m_btClose;
    GtkWidget *m_btExecute;
    GtkWidget *m_btShowAll;
    GtkWidget *m_query;
	GtkTreeView*  m_resultsView;
	GtkTreeStore* m_resultsModel;
    GtkWidget *m_status;

};

#endif /* AP_UNIXDIALOG_RDFQUERY_H */



