/* AbiWord
 * Copyright (C) 2011 AbiSource, Inc.
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

#ifndef AP_DIALOG_RDFQUERY_H
#define AP_DIALOG_RDFQUERY_H

#include <string>

#include "ap_Dialog_Modeless.h"
#include "fv_View.h"
#include "pd_DocumentRDF.h"

class XAP_Frame;

class ABI_EXPORT AP_Dialog_RDFQuery : public AP_Dialog_Modeless
{
  protected:
    virtual XAP_String_Id getWindowTitleStringId();

public:
	AP_Dialog_RDFQuery(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_RDFQuery();

    /**
     * Execute the given SPARQL query, using clear() and addStatement()
     * to update the GUI.
     */
    void executeQuery( const std::string& sparql );
    void showAllRDF();
    std::string uriToPrefixed( const std::string& uri );
    PD_DocumentRDFHandle getRDF();

    /**
     * methods for GUI subclasses to override.
     */
    virtual void clear();
    virtual void addStatement( const PD_RDFStatement& st );
    virtual void setupBindingsView( std::map< std::string, std::string >& b );
    virtual void addBinding( std::map< std::string, std::string >& b );
    virtual void setStatus( const std::string& msg );
    virtual void setQueryString( const std::string& sparql );


protected:
    int m_count;
};

#endif
