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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef PL_LISTENERCOUPLECLOSER_H
#define PL_LISTENERCOUPLECLOSER_H

#include "ut_types.h"
#include "pt_Types.h"
class PX_ChangeRecord;
#include "pl_Listener.h"

#include <list>
#include <string>


class ABI_EXPORT PL_ListenerCoupleCloser : public PL_Listener
{
  protected:
    PD_Document* m_pDocument;
    PL_Listener* m_delegate;
    typedef std::list< std::string > stringlist_t;
    stringlist_t m_rdfAnchorStack;
    
  public:
    PL_ListenerCoupleCloser();
    virtual ~PL_ListenerCoupleCloser();
    void setDelegate( PL_Listener* delegate );
    virtual bool isFinished();
	PD_Document*  getDocument(void);
	void          setDocument(PD_Document * pDoc);

    virtual bool populate( PL_StruxFmtHandle sfh,
                           const PX_ChangeRecord * pcr );
    virtual bool populateStrux( PL_StruxDocHandle sdh,
                                const PX_ChangeRecord * pcr,
                                PL_StruxFmtHandle * psfh);
    
    virtual bool populateClose( PL_StruxFmtHandle sfh,
                                const PX_ChangeRecord * pcr );
    virtual bool populateStruxClose( PL_StruxDocHandle sdh,
                                     const PX_ChangeRecord * pcr,
                                     PL_StruxFmtHandle * psfh);

    virtual bool		change(PL_StruxFmtHandle /*sfh*/,
                               const PX_ChangeRecord * /*pcr*/)
    {return true;}
    
    virtual bool		insertStrux(PL_StruxFmtHandle /*sfh*/,
                                    const PX_ChangeRecord * /*pcr*/,
                                    PL_StruxDocHandle /*sdhNew*/,
                                    PL_ListenerId /*lid*/,
                                    void (* /*pfnBindHandles*/)(PL_StruxDocHandle sdhNew,
                                                                PL_ListenerId lid,
                                                                PL_StruxFmtHandle sfhNew))
    { return true;}
    
	virtual bool		signal(UT_uint32 /*iSignal*/)
    { return true;}    
};

#endif
