/* AbiWord
 * Copyright (C) 2009 Hubert Figuiere
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


#ifndef __PF_FRAG_META_H__
#define __PF_FRAG_META_H__

#include "pf_Frag.h"
#include "pm_MetaDataTypes.h"

class pt_PieceTable;

class pf_Frag_Meta
    : public pf_Frag
{
public:
    pf_Frag_Meta(pt_PieceTable * pPT,
                 PFType type,
                 PT_AttrPropIndex indexAP)
        : pf_Frag(pPT, type, indexAP)
        {
        }

    PMMetaDataId getMetaId() const;
    void setMetaId(PMMetaDataId id);

private:
    PMMetaDataId m_metaid;
};


#endif
