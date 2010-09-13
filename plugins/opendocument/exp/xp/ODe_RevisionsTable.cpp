/* AbiSource
 * 
 * Author: Ben Martin <monkeyiq@abisource.com>
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

#include "ODe_RevisionsTable.h"
#include "xad_Document.h"
#include "pd_Document.h"
#include "ODe_Common.h"

#include <sstream>
#include <string>

using std::endl;

ODe_RevisionsTable::~ODe_RevisionsTable()
{
}

// timet --> 2010-06-02T15:48:00
std::string
ODe_RevisionsTable::timetToODTChangeTrackingString( UT_uint64 timetval64 )
{
    time_t tt = (time_t)timetval64;
    std::string ret;
    struct tm *tmp = gmtime(&tt);
    if ( !tmp )
    {
        return "";
    }

    char outstr[200];
    strftime(outstr, sizeof(outstr), "%Y-%m-%dT%H:%M:%S", tmp);
    ret = outstr;
    return ret;
}

UT_Error
ODe_RevisionsTable::write( GsfOutput* pODT, PD_Document* pAbiDoc ) const
{
	const UT_GenericVector<AD_Revision*> & vRevisions = pAbiDoc->getRevisions();
    
    if( !vRevisions.getItemCount() )
        return UT_OK;

    std::stringstream ss;
    ss << " <delta:tracked-changes "
       << " delta:showing-revisions=\"" << pAbiDoc->isShowRevisions() << "\" "
       << " delta:mark-revisions=\""    << pAbiDoc->isMarkRevisions() << "\" "
       << " delta:current-revision=\""  << pAbiDoc->getShowRevisionId() << "\" "
       << " delta:auto-revisioning=\""  << pAbiDoc->isAutoRevisioning() << "\" "
       << " >" << endl;
         
	for (UT_sint32 k=0; k < vRevisions.getItemCount(); k++)
	{
		UT_String s;
        const AD_Revision * pRev = vRevisions.getNthItem(k);
		UT_continue_if_fail(pRev);
        
        UT_uint32   changeID = pRev->getId();
        std::string author   = pRev->getAuthor();
        UT_uint64   mtimet   = (UT_uint64)pRev->getStartTime();
        std::string mtimestr = timetToODTChangeTrackingString(mtimet);
        UT_UTF8String descutf8;
        descutf8.appendUCS4( pRev->getDescription() );
        
        ss << "  <delta:change-transaction delta:change-id=\"" << changeID << "\">" << endl
           << "     <delta:change-info>"                                 << endl
           << "       <dc:creator>" << author << "</dc:creator>"         << endl
           << "       <dc:date>" << mtimestr << "</dc:date>"             << endl
           << "       <delta:xvers>" << pRev->getVersion() << "</delta:xvers>" << endl
           << "       <delta:change-log>" << descutf8.utf8_str() << "</delta:change-log>" << endl
           << "     </delta:change-info>"                                << endl
           << "  </delta:change-transaction>"                            << endl;
	}

    ss << " </delta:tracked-changes>" << endl;
    ODe_writeUTF8String( pODT, ss.str().c_str() );
    
    return UT_OK;
}

