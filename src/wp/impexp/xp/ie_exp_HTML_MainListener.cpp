/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiWord
* Copyright (C) 2007, 2009 Hubert Figuiere
* Copyright (C) 2003-2005 Mark Gilbert <mg_abimail@yahoo.com>
* Copyright (C) 2002, 2004 Francis James Franklin <fjf@alinameridon.com>
* Copyright (C) 2001-2002 AbiSource, Inc.
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
#include "ie_exp_HTML_MainListener.h"



IE_Exp_HTML_MainListener::IE_Exp_HTML_MainListener(IE_Exp_HTML_Writer* pWriter):
m_pWriter(pWriter),
        m_bIgnoreTillEnd(false),
        m_bIgnoreTillNextSection(false),
        m_iEmbedStartPos(0)
{
    
}

IE_Exp_HTML_MainListener::~IE_Exp_HTML_MainListener()
{
    
}

bool IE_Exp_HTML_MainListener::endOfDocument()
{
    m_bIgnoreTillNextSection = false;
    m_pWriter->popUnendedStructures(); // We need to clean up after ourselves lest we fsck up the footer and anything else that comes after this.
    /* Remaining endnotes, whether from the last of multiple sections or from all sections */
    m_pWriter->doEndnotes();

    m_pWriter->doFootnotes();

    m_pWriter->doAnnotations();

    return true;
}

bool IE_Exp_HTML_MainListener::change(PL_StruxFmtHandle /*sfh*/,
                                      const PX_ChangeRecord * /*pcr*/)
{
    UT_ASSERT_HARMLESS(0); // this function is not used.
    return false;
}

bool IE_Exp_HTML_MainListener::insertStrux(PL_StruxFmtHandle /*sfh*/,
                                           const PX_ChangeRecord * /*pcr*/,
                                           PL_StruxDocHandle /*sdh*/,
                                           PL_ListenerId /* lid */,
                                           void (* /*pfnBindHandles*/)(PL_StruxDocHandle /* sdhNew */,
                                           PL_ListenerId /* lid */,
                                           PL_StruxFmtHandle /* sfhNew */))
{
    UT_ASSERT_HARMLESS(0); // this function is not used.
    return false;
}

bool IE_Exp_HTML_MainListener::signal(UT_uint32 /* iSignal */)
{
    UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
    return false;
}

bool IE_Exp_HTML_MainListener::populate(PL_StruxFmtHandle /*sfh*/, const PX_ChangeRecord * pcr)
{
    if (!m_bSecondPass || (m_bSecondPass && m_bInAFENote))
    {
        if (m_pWriter->is_FirstWrite() && m_pWriter->is_ClipBoard())
        {
            m_pWriter->openSection(0, 0);
            m_pWriter->openTag(0, 0);
        }
        if (m_bIgnoreTillEnd || m_bIgnoreTillNextSection)
        {
            return true;
        }

        switch (pcr->getType())
        {
        case PX_ChangeRecord::PXT_InsertSpan:
        {
            const PX_ChangeRecord_Span * pcrs = 0;
            pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);

            PT_AttrPropIndex api = pcr->getIndexAP();

            m_pWriter->openSpan(api);

            PT_BufIndex bi = pcrs->getBufIndex();
            m_pWriter->outputData(m_pWriter->getDocument()->getPointer(bi), pcrs->getLength());

            // don't m_pWriter->closeSpan (); - leave open in case of identical sequences

            return true;
        }

        case PX_ChangeRecord::PXT_InsertObject:
        {
            if (m_pWriter->is_InSpan()) m_pWriter->closeSpan();

            m_pWriter->set_WroteText();

            const PX_ChangeRecord_Object * pcro = 0;
            pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);

            PT_AttrPropIndex api = pcr->getIndexAP();

            switch (pcro->getObjectType())
            {
            case PTO_Image:
                m_pWriter->handleImage(api);
                return true;

            case PTO_Field:
                m_pWriter->handleField(pcro, api);
                return true;

            case PTO_Hyperlink:
                m_pWriter->handleHyperlink(api);
                return true;

            case PTO_Annotation:
                m_pWriter->handleAnnotationMark(api);
                return true;

            case PTO_RDFAnchor:
                return true;

            case PTO_Bookmark:
                m_pWriter->handleBookmark(api);
                return true;

            case PTO_Math:
                m_pWriter->handleMath(api);
                return true;

            case PTO_Embed:
                m_pWriter->handleEmbedded(api);
                return true;

            default:
                UT_DEBUGMSG(("WARNING: ie_exp_HTML.cpp: unhandled object type: %d!\n", pcro->getObjectType()));
                UT_ASSERT_HARMLESS(UT_TODO);
                return true;
            }
        }

        case PX_ChangeRecord::PXT_InsertFmtMark:
            return true;

        default:
            UT_DEBUGMSG(("WARNING: ie_exp_HTML.cpp: unhandled record type!\n"));
            return true;
        }
    }
    else return true;
}

bool IE_Exp_HTML_MainListener::populateStrux(PL_StruxDocHandle sdh,
                                             const PX_ChangeRecord * pcr,
                                             PL_StruxFmtHandle * psfh)
{
    UT_return_val_if_fail(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux, false);

    *psfh = 0; // we don't need it.

    const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);

    PT_AttrPropIndex api = pcr->getIndexAP();

    switch (pcrx->getStruxType())
    {
    case PTX_Section:
    {
        m_bIgnoreTillNextSection = false;

        // TODO: It may be wise to look into the necessity of an _popUnendedStructures here.  However,
        // that may also not play nice with structures, if any, which span sections.  Unended structures
        // can theoretically do so, such as lists that attach to a extrastructural listID.  However, we
        // also (as of this writing) do not support incontiguous lists.  Again, there's an ambiguity
        // regarding how this should be handled because the behaviour of the piecetable is incompletely
        // defined.
        // UPDATE: We're going to put one in _closeSection for safety's sake.  If it makes some output
        // 	    less pretty, please do file bugs and we can fix that on a case by case basis, but
        // 	    that's preferable to spitting out corrupt html.

        if (m_bIgnoreTillEnd)
        {
            return true; // Nested sections could be the sign of a severe problem, even if caused by import
        }

        // This block prepares us for getting document-level properties (namely, the endnote-place-endsection one stored in doEndnotes)
        PT_AttrPropIndex docApi = m_pWriter->getDocument()->getAttrPropIndex();
        const gchar * doEndnotes = NULL;
        const PP_AttrProp * pDAP = NULL;
        m_pWriter->getDocument()->getAttrProp(docApi, &pDAP);

        // If the d-e-p-e.s. prop is defined	(getProp call succeeds and returns TRUE), and it is 1 (TRUE), we're supposed to spit out the endnotes every section.
        if (pDAP->getProperty("document-endnote-place-endsection", doEndnotes) && atoi(doEndnotes))
        {
            m_pWriter->doEndnotes(); // Spit out the endnotes that have accumulated for this past section.
        }

        if (m_pWriter->is_InBlock()) m_pWriter->closeTag(); // possible problem with lists??
        m_pWriter->openSection(api, 0); // Actually start the next section, which is why we're here.
        return true;
    }

    case PTX_Block:
    {
        if (m_bIgnoreTillEnd || m_bIgnoreTillNextSection)
        {
            return true;
        }
        if (m_pWriter->is_FirstWrite() && m_pWriter->is_ClipBoard()) m_pWriter->openSection(0, 0);
        m_pWriter->openTag(api, sdh);
        return true;
    }


    case PTX_SectionTable:
    {
        if (m_bIgnoreTillEnd || m_bIgnoreTillNextSection)
        {
            return true;
        }
        if (m_pWriter->is_FirstWrite() && m_pWriter->is_ClipBoard()) m_pWriter->openSection(0, 0);

        m_pWriter->getTableHelper().OpenTable(sdh, pcr->getIndexAP());
        m_pWriter->closeSpan();
        m_pWriter->closeTag();
        m_pWriter->openTable(pcr->getIndexAP());
        return true;
    }

    case PTX_SectionCell:
    {
        if (m_bIgnoreTillEnd || m_bIgnoreTillNextSection)
        {
            return true;
        }
        if (m_pWriter->getTableHelper().getNestDepth() < 1)
        {
            m_pWriter->getTableHelper().OpenTable(sdh, pcr->getIndexAP());
            m_pWriter->closeSpan();
            m_pWriter->closeTag();
            m_pWriter->openTable(pcr->getIndexAP());
        }
        m_pWriter->getTableHelper().OpenCell(pcr->getIndexAP());
        m_pWriter->closeSpan();
        m_pWriter->closeTag();
        m_pWriter->openCell(pcr->getIndexAP());
        return true;
    }

    case PTX_EndTable:
    {
        if (m_bIgnoreTillEnd || m_bIgnoreTillNextSection)
        {
            return true;
        }
        m_pWriter->closeTag();
        m_pWriter->get_Buffer() = "tr";
        m_pWriter->tagClose(TT_TR, m_pWriter->get_Buffer());
        m_pWriter->getTableHelper().CloseTable();
        m_pWriter->closeTable();
        return true;
    }

    case PTX_EndCell:
    {
        if (m_bIgnoreTillEnd || m_bIgnoreTillNextSection)
        {
            return true;
        }
        m_pWriter->closeTag();
        m_pWriter->closeCell();
        if (m_pWriter->getTableHelper().getNestDepth() < 1)
        {
            return true;
        }

        m_pWriter->getTableHelper().CloseCell();
        return true;
    }


    case PTX_SectionFootnote:
    case PTX_SectionEndnote:
    case PTX_SectionAnnotation:
    {
        // We should use strux-specific position markers, as this sets a precarious
        // precedent for nested struxes.
        m_iEmbedStartPos = pcrx->getPosition() + 1;
        m_bIgnoreTillEnd = true;
        return true;
    }
    case PTX_EndFootnote:
    case PTX_EndEndnote:
    case PTX_EndAnnotation:
    {
        PD_DocumentRange * pDocRange = new PD_DocumentRange(m_pWriter->getDocument(), m_iEmbedStartPos, pcrx->getPosition());
        if (pcrx->getStruxType() == PTX_EndFootnote)
        {
            m_pWriter->addFootnote(pDocRange);
        }
        else if (pcrx->getStruxType() == PTX_EndEndnote)
        {
            m_pWriter->addEndnote(pDocRange);
        }
        else
        {
            m_pWriter->addAnnotation(pDocRange);
        }
        m_bIgnoreTillEnd = false;
        return true;
    }
    case PTX_SectionFrame:
    {
        // We do this individually for explicitly handled types of frame, because we don't know the consequences
        // of doing it generally.
        // m_bInFrame = true; // Fortunately for the html exporter, abi does not permit nested frames.

        if (m_pWriter->get_ListDepth())
            m_pWriter->listPopToDepth(0); // AbiWord does not support frames in LIs, neither do we.

        if (m_bIgnoreTillEnd || m_bIgnoreTillNextSection)
        {
            return true;
        }
        // Set up to get and get the type of frame (a property thereof)
        const PP_AttrProp * pAP = 0;
        bool bHaveProp = m_pWriter->getDocument()->getAttrProp(api, &pAP);
        if (!bHaveProp || (pAP == 0)) return true;
        const gchar * szType = 0;
        if ((pAP->getProperty("frame-type", szType)) && szType)
        {
            if (!strcmp(szType, "textbox"))
            {
                m_pWriter->openTextBox(pcr->getIndexAP()); // Open a new text box
                return true;
            }
            if (!strcmp(szType, "image"))
            {
                m_pWriter->openPosImage(pcr->getIndexAP()); // Output positioned image
            }
        }
        return true;
    }

    case PTX_EndFrame:
    {
        m_pWriter->closeTextBox();
        return true;
    }
#if 0
    case PTX_EndMarginnote:
    case PTX_SectionMarginnote:
#endif
        // Ignore HdrFtr for now
    case PTX_SectionHdrFtr:
    {
        /* We need to close unended structures (like lists, which are known only as paragraphs with listIDs)
           because the HdrFtr comes after all such things except for those which are contained within it. -MG */
        // This call may be unnecessary. -MG
        m_pWriter->popUnendedStructures();
        m_bIgnoreTillNextSection = true;
        return true;
    }
    case PTX_SectionTOC:
    {
        m_pWriter->emitTOC(pcr->getIndexAP());
        return true;
    }
    case PTX_EndTOC:
    {
        return true;
    }
    default:
        UT_DEBUGMSG(("WARNING: ie_exp_HTML.cpp: unhandled strux type: %d!\n", pcrx->getStruxType()));
        UT_ASSERT_HARMLESS(UT_TODO);
        return true;
    }


}