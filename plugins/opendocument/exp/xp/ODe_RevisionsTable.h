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

#ifndef ODE_REVISIONSTABLE_H_
#define ODE_REVISIONSTABLE_H_

// AbiWord includes
#include <ut_hash.h>

// External includes
#include <gsf/gsf-output.h>

// AbiWord classes
class UT_UTF8String;
class PD_Document;

/**
 * This class represents a <delta:tracked-changes> element and it's children.
 * The output from write() is loaded again by ODi_TrackedChanges_ListenerState.
 */
class ODe_RevisionsTable {
public:

    virtual ~ODe_RevisionsTable();

    // Write ourselves to the ODT
    UT_Error write(GsfOutput* pODT, PD_Document* pAbiDoc ) const;

    static std::string timetToODTChangeTrackingString( UT_uint64 timetval64 );
    
private:

};

#endif /*ODE_REVISIONSTABLE_H_*/
