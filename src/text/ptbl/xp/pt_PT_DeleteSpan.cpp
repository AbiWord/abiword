/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001,2002 Tomas Frydrych
 * Some change tracking code was added in 2011 by Ben Martin
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


// deleteSpan-related routines for class pt_PieceTable

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
#include "ut_stack.h"
#include "pt_PieceTable.h"
#include "pf_Frag.h"
#include "pf_Frag_FmtMark.h"
#include "pf_Frag_Object.h"
#include "pf_Frag_Strux.h"
#include "pf_Frag_Strux_Block.h"
#include "pf_Frag_Strux_Section.h"
#include "pf_Frag_Text.h"
#include "pf_Fragments.h"
#include "px_ChangeRecord.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"
#include "pp_Revision.h"

#include "ut_conversion.h"

#define SETP(p,v)	do { if (p) (*(p)) = (v); } while (0)

/****************************************************************/
/****************************************************************/
bool pt_PieceTable::deleteSpan(PT_DocPosition dpos1,
							   PT_DocPosition dpos2,
							   PP_AttrProp *p_AttrProp_Before,
							   UT_uint32 &iRealDeleteCount,
							   bool bDontGlob)
{
	return deleteSpan(dpos1,
					  dpos2,
					  p_AttrProp_Before,
					  iRealDeleteCount,
					  true,
					  bDontGlob);
}

/****************************************************************/
bool pt_PieceTable::deleteSpanWithTable(PT_DocPosition dpos1,
										PT_DocPosition dpos2,
										PP_AttrProp *p_AttrProp_Before,
										UT_uint32 &iRealDeleteCount,
										bool bDeleteTableStruxes)
{
  //        getFragments().verifyDoc();
	return deleteSpan(dpos1,
					  dpos2,
					  p_AttrProp_Before,
					  iRealDeleteCount,
					  bDeleteTableStruxes,
					  false);
}



/****************************************************************/

bool pt_PieceTable::dumpDoc(
    const char* msg,
    PT_DocPosition currentpos,
    PT_DocPosition endpos )
{
    UT_DEBUG_ONLY_ARG(msg);

    if( !endpos )
    {
        m_pDocument->getBounds( true, endpos );
    }
    
    pf_Frag *pf;
    PT_BlockOffset Offset;

    UT_DEBUGMSG(("=========================================\n" ));
    UT_DEBUGMSG(("dumpDoc(%s) showing range:%d to %d\n", msg, currentpos, endpos ));
    for( PT_DocPosition pos = currentpos; pos < endpos; )
    {
        if(!getFragFromPosition( pos, &pf, &Offset ))
        {
            UT_DEBUGMSG(("dumpDoc() NO FRAG AT pos:%d\n", pos ));
            break;
        }
        std::string fragTypeStr = "            ";
        switch( pf->getType() )
        {
            case pf_Frag::PFT_Text:     fragTypeStr = "PFT_Text    "; break;
            case pf_Frag::PFT_Object:   fragTypeStr = "PFT_Object  "; break;
            case pf_Frag::PFT_Strux:    fragTypeStr = "PFT_Strux   "; break;
            case pf_Frag::PFT_EndOfDoc: fragTypeStr = "PFT_EndOfDoc"; break;
            case pf_Frag::PFT_FmtMark:  fragTypeStr = "PFT_FmtMark "; break;
        }
        std::string extra = "";
        if( pf->getType() == pf_Frag::PFT_Text )
        {
            if( pf_Frag_Text* pft = static_cast<pf_Frag_Text*>(pf))
            {
                extra = pft->toString().substr( 0, 20 );
            }
        }

        if( pf->getType() == pf_Frag::PFT_Strux && tryDownCastStrux( pf, PTX_Block ) ) {
            UT_DEBUGMSG(("dumpDoc()\n"));
	}
        
        UT_DEBUGMSG(("dumpDoc() %s pos:%d frag:%p len:%d frag type:%d extra:%s\n",
                     fragTypeStr.c_str(), pos, pf, pf->getLength(), pf->getType(), extra.c_str() ));
            
        if( pf->getType() == pf_Frag::PFT_Object )
        {
			pf_Frag_Object * po = (pf_Frag_Object*) pf;
            std::string typeStr = "";
            switch( po->getObjectType() )
            {
                case PTO_Image: typeStr = "PTO_Image    "; break;
                case PTO_Field: typeStr = "PTO_Field    "; break;
                case PTO_Bookmark: typeStr = "PTO_Bookmark    "; break;
                case PTO_Hyperlink: typeStr = "PTO_Hyperlink    "; break;
                case PTO_Math: typeStr = "PTO_Math    "; break;
                case PTO_Embed: typeStr = "PTO_Embed    "; break;
                case PTO_Annotation: typeStr = "PTO_Annotation    "; break;
                case PTO_RDFAnchor: typeStr = "PTO_RDFAnchor    "; break;
            }
            UT_DEBUGMSG(("          %s objectType:%d\n", typeStr.c_str(), po->getObjectType() ));
        }
        
        if( pf->getType() == pf_Frag::PFT_Strux )
        {
            PTStruxType eStruxType = static_cast<pf_Frag_Strux*>(pf)->getStruxType();
            std::string eStruxTypeStr;
            switch(eStruxType)
            {
                case PTX_Section:           eStruxTypeStr = "PTX_Section          "; break;
                case PTX_Block:             eStruxTypeStr = "PTX_Block            "; break;
                case PTX_SectionHdrFtr:     eStruxTypeStr = "PTX_SectionHdrFtr    "; break;
                case PTX_SectionEndnote:    eStruxTypeStr = "PTX_SectionEndnote   "; break;
                case PTX_SectionTable:      eStruxTypeStr = "PTX_SectionTable     "; break;
                case PTX_SectionCell:       eStruxTypeStr = "PTX_SectionCell      "; break;
                case PTX_SectionFootnote:   eStruxTypeStr = "PTX_SectionFootnote  "; break;
                case PTX_SectionMarginnote: eStruxTypeStr = "PTX_SectionMarginnote"; break;
                case PTX_SectionAnnotation: eStruxTypeStr = "PTX_SectionAnnotation"; break;
                case PTX_SectionFrame:      eStruxTypeStr = "PTX_SectionFrame     "; break;
                case PTX_SectionTOC:        eStruxTypeStr = "PTX_SectionTOC       "; break;
                case PTX_EndCell:           eStruxTypeStr = "PTX_EndCell          "; break;
                case PTX_EndTable:          eStruxTypeStr = "PTX_EndTable         "; break;
                case PTX_EndFootnote:       eStruxTypeStr = "PTX_EndFootnote      "; break;
                case PTX_EndMarginnote:     eStruxTypeStr = "PTX_EndMarginnote    "; break;
                case PTX_EndEndnote:        eStruxTypeStr = "PTX_EndEndnote       "; break;
                case PTX_EndAnnotation:     eStruxTypeStr = "PTX_EndAnnotation    "; break;
                case PTX_EndFrame:          eStruxTypeStr = "PTX_EndFrame         "; break;
                case PTX_EndTOC:            eStruxTypeStr = "PTX_EndTOC           "; break;
                case PTX_StruxDummy:        eStruxTypeStr = "PTX_StruxDummy       "; break;
            }
            
            UT_DEBUGMSG(("          %s eStruxType:%d\n", eStruxTypeStr.c_str(), eStruxType ));
        }
        pos += pf->getLength();
    }

    return true;
}

 



#ifdef BUILD_ODT_GCT
/**
 * Given a pointer to the start of a block, find the last PFT_Text
 * contained in that block or null.
 *
 */
static pf_Frag_Text* findLastTextFragOfBlock( pf_Frag_Strux* pfblock )
{
    UT_DEBUGMSG(("ODTCT: findLastTxt() start:%p\n", pfblock ));
    if( !pfblock )
        return 0;

    pf_Frag* pf = pfblock->getNext();
    pf_Frag_Text* ret = 0;
    while( pf )
    {
        UT_DEBUGMSG(("ODTCT: findLastTxt() pftype:%d offset:%d len:%d\n", pf->getType(), pf->getPos(), pf->getLength() ));

        if( pf->getType() == pf_Frag::PFT_Text )
        {
			ret = static_cast<pf_Frag_Text*>(pf);
        }
        if( tryDownCastStrux( pf, PTX_Block ))
        {
            //
            // we have hit another block so return the
            // "ret" which is a cache of the last PFT_Text
            return ret;
        }
        
        pf = pf->getNext();
    }
    
    // we might have a ret!=0 if we started in the last block...
    return ret;
}
#endif

/**
 * Return the strux PTX_Block if both startpos and endpos are
 * contained in the same block. This way the method can be used both
 * as a test to see if this is the case and the return value has added
 * value if you actually want to inspect the block containing these
 * positions too.
 */
pf_Frag_Strux* pt_PieceTable::inSameBlock( PT_DocPosition startpos, PT_DocPosition endpos )
{    
    pf_Frag_Strux* ret = 0;
    pf_Frag_Strux* startBlock = _getBlockFromPosition( startpos );
    pf_Frag_Strux* endBlock   = 0;
    if(!_getStruxOfTypeFromPosition( endpos, PTX_Block, &endBlock ))
    {
        return ret;
    }

    // pf_Frag *startFrag;
    // PT_BlockOffset os;
    
    // if(!getFragFromPosition( startpos, &startFrag, &os ))
    // {
    //     UT_DEBUGMSG(("ODTCT: inSameBlock() can not get start, startpos:%d endpos:%d\n", startpos, endpos ));
    //     return ret;
    // }

    // pf_Frag_Strux *startBlock, *endBlock;
    // if( pf_Frag_Strux* pfs = tryDownCastStrux( startFrag, PTX_Block ))
    // {
    //     startBlock = pfs;
    // }
    // else if(!_getStruxOfTypeFromPosition( startpos, PTX_Block, &startBlock ))
    // {
    //     return ret;
    // }
    // if(!_getStruxOfTypeFromPosition( endpos, PTX_Block, &endBlock ))
    // {
    //     return ret;
    // }

    if( startBlock == endBlock )
        ret = startBlock;

    return ret;
}


/**
 *
 * MIQ11: Work out if we are deleting from one paragraph to another in
 * a way that we want to emit a delta:merge to have the two paragraphs
 * merged in ODT. Appologies for the complexity of this method, there
 * seem to be an interesting range of "what if..." questions that come
 * up when you are deciding if something is a delta:merge or a
 * delta:removed-content. Having a single method to trap all of those
 * allows the caller to just mark things appropriately using the
 * "revision" attribute.
 *
 * For .abw format the explicit marking of start/end/para deleted
 * matters less. But the marking needs to be made so that ODT can work
 * properly. 
 *
 * The delta:merge and delta:removed-content allow two or more XML
 * elements to be joined or removed respectively in ODT+GCT [1].
 * Although I talk about these markup constructs from a serialized ODT
 * file, it is best to work out what has happened at the time of
 * document editing rather than try to work backwards at save time.
 * These pieces of markup are put in place again when a ODT+GCT file
 * is read so that they can again be used during a save.
 *
 * Basically delta:merge lets to join two paragraphs together into a
 * single one. If you are just removing content from the start or end
 * of a paragraph then you are better off just using
 * delta:removed-content. If the deletion is for one or more complete
 * paragraphs then it is cleaner to use delta:removed-content to get
 * rid of those paragraphs. Likewise, if deleting from mid way through
 * a paragraph to the end of a subsequent one then you are best off
 * using delta:removed-content on the first paragraph content and
 * delta:removed-content on the subsequent paragraphs. It is just
 * these edge cases that this method contains and allows the caller to
 * simply react if we are performing a delta:merge by marking things
 * as needed.
 * 
 *
 * There are three attributes which should be set to the revision
 * number in which they occur. The start and end deleted attributes
 * should only be added when we are performing a delta:merge.
 * 
 * ABIATTR_PARA_DELETED_REVISION
 *     The whole paragraph is deleted
 * ABIATTR_PARA_START_DELETED_REVISION
 *     The start of the paragraph was deleted, causing it to join with
 *     the previous paragraph
 * ABIATTR_PARA_END_DELETED_REVISION
 *     The end of this paragraph is deleted, causing a subsequent paragraph
 *     to join it's content with this one.
 * 
 * Consider first ABIATTR_PARA_END_DELETED_REVISION which should be
 * set to the current revision if the deletion of the selection can
 * join the next object at the same document scope.
 *
 * Note that this should only be done when the deletion ends in the
 * middle of a paragraph. Otherwise, we should delete the end content
 * of the paragraph (its pcdata or PFT_Text) and contain the other
 * paragraph(s) in delta:removed-content.
 * 
 * This is the case when:
 *
 ** deleting from a para to another para.
 *
 ** deleting from a para through an entire table to
 *  another para
 *
 * This is NOT the case when:
 *
 ** deleting from a para into an image (draw:frame). In this case the
 *  para content to its end is deleted and the image is wrapped in
 *  delta:removed-content.
 *
 ** deleting from a para to a para in a table or to a para in another
 *  cell of this or another table. In this case starting with para1,
 *  para2...paran, tableA, ... celly If the selection extends from ra1
 *  through into celly then The content of para1 "ra1" is deleted with
 *  delta:removed-content. para2...paran are enclosed in
 *  delta:removed-content to be deleted. Each cell in the selection
 *  which from tableA is handled as a subdocument.
 *
 ***********************
 *
 * Firstly, this is not a delta:merge if the start and end pos are
 * contained within the same paragraph (and startpos+1 != endpos, see
 * below).
 *
 * __Document Level Considerations__
 * 
 * Failing that, we want to get the paragraph (PTX_Block) for both the
 * first and second position, if both exist then we need to make sure
 * they are in the same table cell or no cell at all. Finding the cell
 * is done by seeking backward from the Block.
 *
 * If we hit a table end or cell end marker (PTX_EndTable |
 * PTX_EndCell) before we find a cell start (PTX_SectionCell) then we
 * are in a paragraph that is at the top document scope. ie, a
 * paragraph with "no cell".
 *
 * __Delete To and From the exact Start/End of Paragraph__
 * 
 * Another two special cases; if we are deleting right through to the
 * end of the last paragraph then we do not mark it as a delta:merge
 * because we perfer to delete the end content of the first paragraph
 * and delta:removed-content the other paragraphs as that is a simpler
 * document. Similarly, if we are deleting from the start of a para
 * through it's end then we mark that para with delta:removed-content
 * and remove the starting content from the last para. If there are
 * one or more paragraphs between these start/end cases they should be
 * wrapped in delta:removed-content too.
 *
 * __Single Character Delete/Backspace To Merge__
 * __Delete from starting PTX_Block into the para__
 * 
 * When marging two paragraphs using the backspace or delete key, the
 * range is only a single character: startpos+1 == endpos. The
 * startpos will be the PTX_Block and the endpos likely the first char
 * in the PFT_Text. This case is a delta:merge because we are merging the
 * second paragraph into the content of the first. Note that we handle
 * this case when the start and end blocks are the same even when
 * endpos > startpos+1 because the user might have a selection
 * starting at the PTX_Block and into the second paragraph and delete
 * the lot in one operation.
 *
 * 
 ***********************
 *
 * NOTES:
 *
 * __Selection to Delete Para__
 *
 * Note that in the GUI you have to select from the start of the
 * previous line in order to delete a paragraph or to merge it with
 * the previous one. eg, if the document text is as below, to merge
 * the second paragraph you either place the carrot at the start of
 * the second line and backspace or the end of the first line and
 * delete.
 * 
 * This is para1
 * And the second one.
 *
 * This seems natural enough, but to delete the whole second paragraph
 * you have to start the text selection from the carrot position just
 * after the "para1" string, ie, the end of the first line instead of
 * the start of the second line.
 * 
 * __Variable Names__
 *
 * MIQ11: I have omitted the pf prefix to pointers to fragments
 * because most of the local variables are of that type.
 *
 *
 ***********************
 *
 * [1]  http://monkeyiq.blogspot.com/2011/04/change-tracking-why.html
 */
#ifdef BUILD_ODT_GCT
bool pt_PieceTable::deleteSpanChangeTrackingAreWeMarkingDeltaMerge( PT_DocPosition startpos,
                                                                    PT_DocPosition endpos )
{
    bool ret = false;

#if DEBUG
    dumpDoc( "areWeMarkingDM(top)", 0, 0 );
    UT_DEBUGMSG(("ODTCT: areWeMarkingDM() startpos:%d endpos:%d\n", startpos, endpos ));
#endif
    
    pf_Frag *startFrag, *endFrag;
    PT_BlockOffset os,oe;
    
    if(!getFragFromPosition( startpos, &startFrag, &os ))
    {
        UT_DEBUGMSG(("ODTCT: areWeMarkingDM() can not get start, startpos:%d endpos:%d\n", startpos, endpos ));
        return ret;
    }

    if(!getFragFromPosition( endpos, &endFrag, &oe ))
    {
        UT_DEBUGMSG(("ODTCT: areWeMarkingDM() can not get end, startpos:%d endpos:%d\n", startpos, endpos ));
        return ret;
    }
    pf_Frag_Strux *startBlock, *endBlock;
    if( pf_Frag_Strux* pfs = tryDownCastStrux( startFrag, PTX_Block ))
    {
        UT_DEBUGMSG(("ODTCT: areWeMarkingDM() deleting right from start of para block...\n" ));
        startBlock = pfs;
    }
    else if(!_getStruxOfTypeFromPosition( startpos, PTX_Block, &startBlock ))
    {
        UT_DEBUGMSG(("ODTCT: areWeMarkingDM() can not get start block, startpos:%d endpos:%d\n", startpos, endpos ));
        return ret;
    }

    
    if(!_getStruxOfTypeFromPosition( endpos, PTX_Block, &endBlock ))
    {
        UT_DEBUGMSG(("ODTCT: areWeMarkingDM() can not get end block, startpos:%d endpos:%d\n", startpos, endpos ));
        return ret;
    }

#if DEBUG
    UT_DEBUGMSG(("ODTCT: areWeMarkingDM(starting work...) startpos:%d endpos:%d\n", startpos, endpos ));
    UT_DEBUGMSG(("ODTCT: areWeMarkingDM(soffset) pos:%d len:%d\n", startFrag->getPos(), startFrag->getLength() ));
    UT_DEBUGMSG(("ODTCT: areWeMarkingDM(eoffset) pos:%d len:%d\n", endFrag->getPos(), endFrag->getLength() ));
    UT_DEBUGMSG(("ODTCT: areWeMarkingDM(sblock ) pos:%d len:%d\n", startBlock->getPos(), startBlock->getLength() ));
    UT_DEBUGMSG(("ODTCT: areWeMarkingDM(eblock ) pos:%d len:%d\n", endBlock->getPos(), endBlock->getLength() ));
#endif
    
    /*
     * Very likely not a delta:merge if we are in the same block.
     */
    if( startBlock && startBlock == endBlock )
    {
        /*
         * The special case here is merging of two paragraphs using
         * the delete or backspace key or a selection which causes
         * merge. In this case the startBlock is right on the
         * PTX_Block and the endBlock should be the start of the
         * PFT_Text (for delete or backspace). However, if the
         * selection extends into the para more than 1 char we are
         * still wanting to merge.
         */
        if( tryDownCastStrux( startFrag, PTX_Block ) )
        {
            pf_Frag* lastFragOfEnd = findLastTextFragOfBlock( endBlock );
            if( lastFragOfEnd->getPos() + lastFragOfEnd->getLength() == endpos )
            {
                UT_DEBUGMSG(("ODTCT: areWeMarkingDM() start and end are the same block and you are deleting the whole para.\n" ));
                return false;
            }
            
            UT_DEBUGMSG(("ODTCT: areWeMarkingDM() start and end are the same block BUT we are merging two paras with delete or backspace\n" ));
            return true;
        }
        
        UT_DEBUGMSG(("ODTCT: areWeMarkingDM() start and end are the same block... not a delta:merge\n" ));
        return false;
    }
    
    
    pf_Frag* lastFragOfEnd = findLastTextFragOfBlock( endBlock );
    if( lastFragOfEnd )
        UT_DEBUGMSG(("ODTCT: areWeMarkingDM(elastB ) pos:%d len:%d\n", lastFragOfEnd->getPos(), lastFragOfEnd->getLength() ));
    
    PTStruxType stopCondition[] = { PTX_EndCell, PTX_EndTable, PTX_StruxDummy };
    bool bSkipEmbededSections = true;
    pf_Frag_Strux *startCell = _findLastStruxOfType( startBlock, PTX_SectionCell, stopCondition, bSkipEmbededSections );
    pf_Frag_Strux *endCell   = _findLastStruxOfType( endBlock,   PTX_SectionCell, stopCondition, bSkipEmbededSections );

#if DEBUG
    UT_DEBUGMSG(("ODTCT: areWeMarkingDM(cells ) start:%p end:%p\n", startCell, endCell ));
    if( startCell )
        UT_DEBUGMSG(("ODTCT: areWeMarkingDM(scell ) pos:%d len:%d\n", startCell->getPos(), startCell->getLength() ));
    if( endCell )
        UT_DEBUGMSG(("ODTCT: areWeMarkingDM(ecell ) pos:%d len:%d\n", endCell->getPos(), endCell->getLength() ));
    if( lastFragOfEnd )
        UT_DEBUGMSG(("ODTCT: areWeMarkingDM(elast ) pos:%d len:%d\n", lastFragOfEnd->getPos(), lastFragOfEnd->getLength() ));
    if( endBlock )
        UT_DEBUGMSG(("ODTCT: areWeMarkingDM(block ) pos:%d len:%d\n", endBlock->getPos(), endBlock->getLength() ));
#endif
    
    // no cell or same cell.
    if( (!startCell && !endCell) || (startCell == endCell) )
    {
        /*
        * If we are deleting through to the middle of another
        * paragraph then we need to use delta:merge to join them.
        */
        ret = true;
        UT_DEBUGMSG(("ODTCT: areWeMarkingDM(ret) in same or no cell, startBlock:%d startpos:%d\n",
                     startBlock->getPos(), startpos ));

        /*
        * On the other hand, if we are deleting from a paragraph
        * through to the end of another paragraph then we prefer
        * instead to mark the end content of the first paragraph as
        * deleted and use delta:removed-content to mark the other
        * paragraph(s) as deleted. Simpler markup this way.
        *
        * lastFragOfEnd is the last fragment contained in the last block.
        * if it has an ending that is exactly the deletion endpos
        * then we are not deleting *into* the paragraph but to the end of it.
        */
        if( lastFragOfEnd &&
            ( lastFragOfEnd->getPos() + lastFragOfEnd->getLength() == endpos ))
        {
            UT_DEBUGMSG(("ODTCT: areWeMarkingDM(ret) deletion is right to end of last paragraph!\n" ));
            ret = false;
        }

        
        /*
         * Likewise, if we are deleting from the very start of a paragraph through
         * to another para then we mark the entire first para as delta:removed-content
         * and just remove the leading content in the last para as deleted.
         */
        if( startBlock && startBlock->getPos() == startpos )
        {
            UT_DEBUGMSG(("ODTCT: areWeMarkingDM(ret) deletion from right at start of first paragraph!\n" ));
            ret = false;
        }
        
    }
    
    return ret;
}
#endif

/**
 * Get the first strux that marks the end of the block containing
 * currentpos or null. The end of block strux has to be positioned
 * before endpos or null is returned.
 *
 * Note that if currentpos is itself a block this method will move
 * over that PTX_Block before searching for the end of block.
 */
pf_Frag* pt_PieceTable::getEndOfBlock( PT_DocPosition currentpos, PT_DocPosition endpos )
{
    pf_Frag *pf;
    PT_BlockOffset Offset;
    bool bFoundEndBlockBeforeEndpos = false;

    UT_DEBUGMSG(("ODTCT: getEndOfBlock() cpos:%d e:%d\n", currentpos, endpos ));
    
    // If the frag at currentpos is a block move past it.
    if(getFragFromPosition( currentpos, &pf, &Offset ))
    {
        if( tryDownCastStrux( pf, PTX_Block ))
        {
            ++currentpos;
        }
    }
    
    for( PT_DocPosition pos = currentpos; !bFoundEndBlockBeforeEndpos && pos <= endpos; )
    {
        if(!getFragFromPosition( pos, &pf, &Offset ))
        {
            UT_DEBUGMSG(("ODTCT: getEndOfBlock() NO FRAG AT pos:%d\n", pos ));
            break;
        }

        UT_DEBUGMSG(("ODTCT: getEndOfBlock() pos:%d frag:%p len:%d frag type:%d\n", pos, pf, pf->getLength(), pf->getType() ));

        if( pf->getType() == pf_Frag::PFT_EndOfDoc )
        {
            return 0;
        }
        
        if( pf->getType() == pf_Frag::PFT_Strux )
        {
            PTStruxType eStruxType = static_cast<pf_Frag_Strux*>(pf)->getStruxType();
            switch (eStruxType)
            {
                case PTX_SectionFootnote: 
                case PTX_SectionEndnote: 
                case PTX_SectionAnnotation:
                    break;
                default:
                    return pf;
            }
        }
                
        pos = pf->getPos() + pf->getLength();
    }
    return 0;
}

/**
 * Add a change tracking attribute/value to the given fragment. If the
 * attribute already exists on the fragment then it is not modified.
 */
#ifdef BUILD_ODT_GCT
bool pt_PieceTable::changeTrackingAddParaAttribute( pf_Frag_Strux* pfs,
                                                    const char* attr,
                                                    std::string v )
{
    const PP_AttrProp * pAP2;
    if(!getAttrProp(pfs->getIndexAP(),&pAP2))
    {
        UT_DEBUGMSG(("Can not get the attrProp for a fragment when adding change tracking information!\n" ));
        return false;
    }
    else
    {
        const gchar name[] = "revision";
        const gchar * pRevision = NULL;
                    
        if(!pAP2->getAttribute(name, pRevision))
            pRevision = NULL;
        PP_RevisionAttr Revisions(pRevision);
        if( pRevision && strstr(pRevision, attr ))
        {
            // already have that attribute..
            return true;
        }
        else
        {
            Revisions.mergeAttr( 1, PP_REVISION_ADDITION_AND_FMT,
                                 attr, v.c_str() );
            
            const gchar * ppRevAttrib[3];
            ppRevAttrib[0] = name;
            ppRevAttrib[1] = Revisions.getXMLstring();
            ppRevAttrib[2] = NULL;

            UT_DEBUGMSG(("ODTCT: changeTrackingAddParaAttribute() adding attr:%s v:%s for block at:%d\n",
                         attr, v.c_str(), pfs->getPos() ));
                    
            int iLen = pf_FRAG_STRUX_BLOCK_LENGTH;
            PTStruxType eStruxType = pfs->getStruxType();

            if(! _realChangeStruxFmt(PTC_AddFmt, pfs->getPos() + iLen, pfs->getPos() + iLen,
                                     ppRevAttrib, NULL,
                                     eStruxType, true))
            {
                return false;
            }
        }
    }
    
    return true;
}
#endif
                                                    
/*
 * If we are deleting a selection for which a delta:merge is in
 * progress, this method adds ABIATTR_PARA_END_DELETED_REVISION to the
 * block only if the block ends before endpos
 */
#ifdef BUILD_ODT_GCT
bool pt_PieceTable::deleteSpanChangeTrackingMaybeMarkParagraphEndDeletion( PT_DocPosition currentpos,
                                                                           PT_DocPosition endpos )
{
#if DEBUG
    dumpDoc( "deleteSpanChangeTrackingMaybeMarkParagraphEndDeletion(top)", 0, 0 );
    UT_DEBUGMSG(("ODTCT: deleteSpanChangeTrackingMaybeMarkParagraphEndDeletion() cpos:%d endpos:%d\n", currentpos, endpos ));
#endif
    
    //
    // MIQ11: If we are deleting from the middle through the end of
    // a paragraph then we want to record when the end of the
    // paragraph was deleted. We need to get the PTX_Block, say
    // (a), containing the start marker currentpos and make sure that
    // (a) is closed before endpos is reached.
    //
    // First, walk forwards to see if the block will end before
    // endpos is reached. If so, find the strux (a) that contains
    // currentpos and mark it as having it's end of block at this
    // revision.
    {
        //
        // Find the first end-of-block condition that is yonder of
        // currentpos
        //
        UT_DEBUGMSG(("ODTCT: deleteSpan(revisionsEP) searching for eob from:%d\n", currentpos ));
        pf_Frag *pf = getEndOfBlock( currentpos, endpos );
        if( !pf )
        {
            UT_DEBUGMSG(("ODTCT: deleteSpan(revisionsEP) no end of block!\n" ));
            return false;
        }
        
        UT_DEBUGMSG(("ODTCT: deleteSpan(revisionsEP) block that ends the currentpos starter is %p at offset:%d len:%d\n",
                     pf, pf->getPos(), pf->getLength() ));
            
            
        //
        // find and mark the block containing currentpos as having its
        // ending deleted in this revision.
        //
        pf_Frag_Strux * pfs;
        PTStruxType eStruxType = PTX_Block;
        if(!_getStruxOfTypeFromPosition( currentpos, eStruxType, &pfs ))
        {
            // failed
            UT_DEBUGMSG(("ODTCT: deleteSpan(revisionsEP) delete started not inside a ptx_block! currentpos:%d\n", currentpos ));
            return false;
        }
        else
        {
            UT_DEBUGMSG(("ODTCT: deleteSpan(revisionsEP) TOP currentpos:%d\n", currentpos ));
            UT_DEBUGMSG(("ODTCT: deleteSpan(revisionsEP) TOP text strux:%p\n", pfs ));

            changeTrackingAddParaAttribute( pfs,
                                            ABIATTR_PARA_END_DELETED_REVISION,
                                            tostr(m_pDocument->getRevisionId()));
        }
    }

    return true;
}
#endif



/****************************************************************/
bool pt_PieceTable::deleteSpan(PT_DocPosition dpos1,
							   PT_DocPosition dpos2,
							   PP_AttrProp *p_AttrProp_Before,
							   UT_uint32 &iRealDeleteCount,
							   bool bDeleteTableStruxes,
							   bool bDontGlob)
{
#ifdef BUILD_ODT_GCT
    PT_DocPosition startOfRange = dpos1;
#endif
    
  //        getFragments().verifyDoc();
	if(m_pDocument->isMarkRevisions())
	{

#if DEBUG
        UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) TOP dpos1:%d dpos2:%d\n", dpos1, dpos2 ));
        dumpDoc( "deleteSpan(top)", 0, 0 );
#endif
        
		bool bHdrFtr = false;
		// if the user selected the whole document for deletion, we will not delete the
		// first block (we need always a visible block in any document); we make an
		// exception to this in VDND, because in that case the original block is
		// guaranteed to come back
		// 
		// NB: it is possible that the user might delete all contents in several separate
		// steps; there is no easy way to protect against that
		bool bWholeDoc = false;
		if(!m_pDocument->isVDNDinProgress())
		{
			pf_Frag * pLast = getFragments().getLast();
			bWholeDoc = (dpos1 <= 2 && pLast->getPos() == dpos2);
		}
		
		iRealDeleteCount = 0;

		const gchar name[] = "revision";
		const gchar * pRevision = NULL;


#ifdef BUILD_ODT_GCT
        bool MarkingDeltaMerge = deleteSpanChangeTrackingAreWeMarkingDeltaMerge( dpos1, dpos2 );
        UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) TOP2 dpos1:%d dpos2:%d\n", dpos1, dpos2 ));
        UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) TOP3 MarkingDeltaMerge:%d\n", MarkingDeltaMerge ));

        if( MarkingDeltaMerge )
        {
            deleteSpanChangeTrackingMaybeMarkParagraphEndDeletion( dpos1, dpos2 );

            /*
             * For merging paragraphs by pressing delete when the carrot is on
             * the end of the last line of a paragraph. So dpos1 is the PTX_Block itself
             * and we need to grab the previous block to dpos1 and mark it's end deleted.
             */
            if(pf_Frag_Strux* pfs = inSameBlock( dpos1, dpos2 ))
            {
                UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) isSameBlock at:%d\n", pfs->getPos() ));
                bool bSkipEmbededSections = true;
                pf_Frag_Strux* prevBlock = _findLastStruxOfType( pfs->getPrev(), PTX_Block, bSkipEmbededSections );
                if( prevBlock )
                {
                    UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) prevBlock at:%d\n", prevBlock->getPos() ));

                    changeTrackingAddParaAttribute( prevBlock,
                                                    ABIATTR_PARA_END_DELETED_REVISION,
                                                    tostr(m_pDocument->getRevisionId()));
                }
            }
        }
#endif
    
        
		// we cannot retrieve the start and end fragments here and
		// then work between them in a loop using getNext() because
		// processing might result in merging of fragments. so we have
		// to use the doc position to keep track of where we are and
		// retrieve the fragments afresh in each step of the loop
		// Tomas, Oct 28, 2003

		bool bRet = false;
		while(dpos1 < dpos2)
		{
			// first retrive the starting and ending fragments
			pf_Frag * pf1, * pf2;
			PT_BlockOffset Offset1, Offset2;
			bool bTableStrux = false;
			bool bHasEndStrux = false;
			UT_sint32 iTableDepth = 0;

            // UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) while.top dpos1:%d dpos2:%d\n", dpos1, dpos2 ));
            
			if(!getFragsFromPositions(dpos1,dpos2, &pf1, &Offset1, &pf2, &Offset2))
				return bRet;
			else
				bRet = true;

            // UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) while.2 dpos1:%d dpos2:%d\n", dpos1, dpos2 ));
            // UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) while.2 types pf1:%d pf2:%d pf1.len:%d\n",
            //              pf1->getType(), pf2->getType(), pf1->getLength() ));
            
			// get attributes for this fragement
			const PP_AttrProp * pAP2;
			pf_Frag::PFType eType = pf1->getType();
			UT_uint32 iLen = 1;
			PTStruxType eStruxType = PTX_StruxDummy;

			if(eType == pf_Frag::PFT_Text)
			{
				if(!getAttrProp(static_cast<pf_Frag_Text*>(pf1)->getIndexAP(),&pAP2))
					return false;
			}
			else if(eType == pf_Frag::PFT_Strux)
			{
				if(!getAttrProp(static_cast<pf_Frag_Strux*>(pf1)->getIndexAP(),&pAP2))
					return false;

				eStruxType = static_cast<pf_Frag_Strux*>(pf1)->getStruxType();

#ifdef BUILD_ODT_GCT
                UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) while... eStruxType:%d\n", eStruxType ));

                if( dpos1 > startOfRange )
                {
                    // Mark close of block
                    switch (eStruxType)
                    {
                        case PTX_SectionFootnote: 
                        case PTX_SectionEndnote: 
                        case PTX_SectionAnnotation:
                            break;
                        default:
                            // If this block ends before dpos2,
                            // we should also explicitly mark that it's end is deleted.
                            if( MarkingDeltaMerge )
                            {
                                deleteSpanChangeTrackingMaybeMarkParagraphEndDeletion( dpos1-1, dpos2 );
                            }
                            break;
                    }
                }
#endif

                
				switch (eStruxType)
				{
					case PTX_Block:
						iLen = pf_FRAG_STRUX_BLOCK_LENGTH;
						if(bWholeDoc && dpos1 == 2)
						{
							dpos1 += iLen;
							continue;
						}
						
                        // UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) block... dpos1:%d dpos2:%d len:%d\n", dpos1, dpos2, iLen ));
						break;
						
					case PTX_SectionTable:
						iTableDepth = 1;
						// fall through
					case PTX_SectionCell:
						bTableStrux = true;
						bHasEndStrux = true;
						iLen = pf_FRAG_STRUX_SECTION_LENGTH;
						break;
						
					case PTX_EndCell:
					case PTX_EndTable:
						bTableStrux = true;
						iLen = pf_FRAG_STRUX_SECTION_LENGTH;
						break;
						
					case PTX_SectionEndnote:
					case PTX_SectionFootnote:
					case PTX_SectionAnnotation:
					case PTX_SectionFrame:
					case PTX_SectionTOC:
						bHasEndStrux = true;
						// fall through ...
					case PTX_SectionHdrFtr:
						bHdrFtr = true;
						// fall through
					case PTX_Section:
				    case PTX_EndFootnote:
				    case PTX_EndEndnote:
				    case PTX_EndAnnotation:
				    case PTX_EndFrame:
					case PTX_EndTOC:
						iLen = pf_FRAG_STRUX_SECTION_LENGTH;
						break;

					default:
						UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
						iLen = 1;
						break;
				}

			}
			else if(eType == pf_Frag::PFT_Object)
			{
				if(!getAttrProp(static_cast<pf_Frag_Object*>(pf1)->getIndexAP(),&pAP2))
					return false;
			}
			else if(eType == pf_Frag:: PFT_FmtMark)
			{
				iLen = 0;
				if(!getAttrProp(static_cast<pf_Frag_Object*>(pf1)->getIndexAP(),&pAP2))
					return false;
				// something that does not carry AP
			}
			else if (eType == pf_Frag:: PFT_EndOfDoc)
			{
				break;
			}
			else
			{
				UT_ASSERT(0); // Dunno what this could be
				break;
			}

            // UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) while bTableStrux:%d bDeleteTableStruxes:%d\n",
            //              bTableStrux, bDeleteTableStruxes ));
			if(bTableStrux && !bDeleteTableStruxes)
			{
				// skip over this frag
				dpos1 += iLen;
				continue;
			}
			
			if(!pAP2->getAttribute(name, pRevision))
				pRevision = NULL;

            // UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) while.3 dpos1:%d dpos2:%d pRevision:%p\n", dpos1, dpos2, pRevision ));
            // if( pRevision )
                // UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) while.3 pRevision.str:%s\n", pRevision ));

            
			PP_RevisionAttr Revisions(pRevision);

			// now we need to see if revision with this id is already
			// present, and if it is, whether it might not be addition
			UT_uint32 iId = m_pDocument->getRevisionId();
			const PP_Revision * pS;
			const PP_Revision * pRev = Revisions.getGreatestLesserOrEqualRevision(iId, &pS);

			PT_DocPosition dposEnd = UT_MIN(dpos2,dpos1 + pf1->getLength());

#ifdef BUILD_ODT_GCT
#if DEBUG
            {
                UT_uint32 gid = 100;
                if( pRev )
                {
                    UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) while.5 have pRev, iId:%d pRev->id::%d\n", iId, pRev->getId() ));
                    gid = pRev->getId();
                }
                UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) while.4 pRev:%d iId:%d getId:%d dposEnd:%d\n", pRev!=0, iId, gid, dposEnd ));
            }
#endif
#endif            
            
			if(pRev && iId == pRev->getId())
			{
				// OK, we already have a revision with this id here,
				// which means that the user made a change earlier
				// (insertion or format change) but now wants this deleted
				//
				// so if the previous revision is an addition, we just
				// remove this fragment as if this was regular delete
				if(   (pRev->getType() == PP_REVISION_ADDITION)
				   || (pRev->getType() == PP_REVISION_ADDITION_AND_FMT ))
				{
					// if this fragment is one of the struxes that is paired with an end-strux, we
					// need to delete both struxes and everything in between
					if(bHasEndStrux || bHdrFtr)
					{
						PT_DocPosition posEnd = dposEnd;
						for(pf_Frag * pf = pf1->getNext(); pf != NULL; pf = pf->getNext())
						{
							posEnd += pf->getLength();
							
							if(pf_Frag::PFT_Strux != pf->getType())
								continue;

							pf_Frag_Strux * pfs = (pf_Frag_Strux*) pf;
							PTStruxType eStrux2Type = pfs->getStruxType();

							if(eStrux2Type == PTX_SectionTable)
								iTableDepth++;
							else if (eStrux2Type == PTX_EndTable)
								iTableDepth--;
								
							
							switch(eStruxType )
							{
								case PTX_SectionEndnote:
									if(eStrux2Type != PTX_EndEndnote)
										continue;
									break;
								case PTX_SectionTable:
									if(iTableDepth > 0 || eStrux2Type != PTX_EndTable)
										continue;
									break;
								case PTX_SectionCell:
									if(iTableDepth > 0 || eStrux2Type != PTX_EndCell)
										continue;
									break;
								case PTX_SectionFootnote:
									if(eStrux2Type != PTX_EndFootnote)
										continue;
									break;
								case PTX_SectionAnnotation:
									if(eStrux2Type != PTX_EndAnnotation)
										continue;
									break;
								case PTX_SectionFrame:
									if(eStrux2Type != PTX_EndFrame)
										continue;
									break;
								case PTX_SectionTOC:
									if(eStrux2Type != PTX_EndTOC)
										continue;
									break;
								case PTX_SectionHdrFtr:
									if(eStrux2Type != PTX_SectionHdrFtr)
										continue;
									break;
							    default:
									break;
							}

							// if we got this far, we found what we are looking for and we have the
							// correct end position
							break;
						}
						
						dposEnd = posEnd;
					}

					if(bHdrFtr)
					{
						bHdrFtr = false; // only do this once
						pf_Frag_Strux_SectionHdrFtr * pfHdr = static_cast<pf_Frag_Strux_SectionHdrFtr *>(pf1);

						const PP_AttrProp * pAP = NULL;

						if(!getAttrProp(pfHdr->getIndexAP(),&pAP) || !pAP)
							return false;

						const gchar * pszHdrId;
						if(!pAP->getAttribute("id", pszHdrId) || !pszHdrId)
							return false;

						const gchar * pszHdrType;
						if(!pAP->getAttribute("type", pszHdrType) || !pszHdrType)
							return false;

						// needs to be in this order because of undo
						_realDeleteHdrFtrStrux(static_cast<pf_Frag_Strux*>(pf1));
						_fixHdrFtrReferences(pszHdrType, pszHdrId);
					}
					else
					{
                        // UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) calling realDeleteSpan %d %d\n", dpos1, dposEnd ));
                        
						if(!_realDeleteSpan(dpos1, dposEnd, p_AttrProp_Before,bDeleteTableStruxes,
											bDontGlob))
							return false;
					}
					
					UT_uint32 iDeleteThisStep = dposEnd - dpos1;
					
					iRealDeleteCount += iDeleteThisStep;

					// because we removed stuff, the position dpos1 remains the same and dpos2 needs
					// to be adjusted
					if(dpos2 > iDeleteThisStep)
						dpos2 -= iDeleteThisStep;
					else
						dpos2 = 0;
					
					continue;
				}
			}

#ifdef BUILD_ODT_GCT
            //
            // handle the marking of DELETED and START_DELETED
            //
            if( eStruxType == PTX_Block )
            {
                std::string idstr = tostr(iId);
                UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) old:%s\n", Revisions.getXMLstring() ));
                UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) finding eob...\n" ));

                if( pf_Frag *endOfblock = getEndOfBlock( dpos1, dpos2 ) )
                {
                    UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) eob pos:%d len:%d dpos1:%d dpos2:%d\n",
                                 endOfblock->getPos(), endOfblock->getLength(), dpos1, dpos2 ));

                    Revisions.mergeAttrIfNotAlreadyThere( 1, PP_REVISION_ADDITION_AND_FMT,
                                                          ABIATTR_PARA_DELETED_REVISION,
                                                          idstr.c_str() );
                }
    
                if( MarkingDeltaMerge )
                {
                    UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) marking delta:merge...\n" ));

                    Revisions.mergeAttrIfNotAlreadyThere( 1, PP_REVISION_ADDITION_AND_FMT,
                                                          ABIATTR_PARA_START_DELETED_REVISION,
                                                          idstr.c_str() );
                }

                UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) DONE adding, revs:%s\n", Revisions.getXMLstring() ));
            }
#endif
            
			Revisions.addRevision(iId,PP_REVISION_DELETION,NULL,NULL);
            // UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) 2...\n" ));
			const gchar * ppRevAttrib[3];
			ppRevAttrib[0] = name;
			ppRevAttrib[1] = Revisions.getXMLstring();
			ppRevAttrib[2] = NULL;

            // UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) while.6 dpos1:%d dpos2:%d eType:%d RX:%s\n",
            //             dpos1, dpos2, eType, Revisions.getXMLstring() ));
            
			switch (eType)
			{
				case pf_Frag::PFT_Object:
				case pf_Frag::PFT_Text:
                    // UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) addfmt dpos1:%d dposEnd:%d\n", dpos1, dposEnd ));
					if(! _realChangeSpanFmt(PTC_AddFmt, dpos1, dposEnd, ppRevAttrib,NULL,true))
						return false;


                    //
                    // MIQ11: The above _realChangeSpanFmt() may have split the fragment,
                    //        So we should get the pf_Frag again and adjust dposEnd if nessesary
                    //
                    {
                        pf_Frag * pft;
                        PT_BlockOffset toffset;
                        getFragFromPosition(dpos1, &pft, &toffset);
                        UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) addfmt dpos1:%d dposEnd:%d toffset:%d pftpos:%d pftlen:%d\n",
                                     dpos1, dposEnd, toffset, pft->getPos(), pft->getLength() ));

                        dposEnd = pft->getPos() + pft->getLength();
                    }
                    
					break;

				case pf_Frag::PFT_Strux:
                    UT_DEBUGMSG(("ODTCT: deleteSpan(revisions) strux dpos1:%d iLen:%d\n", dpos1, iLen ));
					// _realChangeStruxFmt() changes the strux
					// *containing* the given position, hence we pass
					// it the position immediately after the strux; we
					// only want the one strux changed, so we pass
					// identical position in both parameters
					if(! _realChangeStruxFmt(PTC_AddFmt, dpos1 + iLen, dpos1 + iLen /*2*iLen*/, ppRevAttrib,NULL,
											 eStruxType,true))
						return false;

					if(bHdrFtr)
					{
						// even though this is just a notional removal, we still have to
						// fix the references
						bHdrFtr = false; // only do this once
						pf_Frag_Strux_SectionHdrFtr * pfHdr = static_cast<pf_Frag_Strux_SectionHdrFtr *>(pf1);

						const PP_AttrProp * pAP = NULL;

						if(!getAttrProp(pfHdr->getIndexAP(),&pAP) || !pAP)
							return false;

						const gchar * pszHdrId;
						if(!pAP->getAttribute("id", pszHdrId) || !pszHdrId)
							return false;

						const gchar * pszHdrType;
						if(!pAP->getAttribute("type", pszHdrType) || !pszHdrType)
							return false;

						_fixHdrFtrReferences(pszHdrType, pszHdrId, true);
                        // empty the strux listener chache since the pointers are now
                        // invalid                                              
                        pfHdr->clearAllFmtHandles();                            
					}
					
					break;

				default:;
			}

			dpos1 = dposEnd;
		}

		return true;
	}
	else
		return _realDeleteSpan(dpos1, dpos2, p_AttrProp_Before, bDeleteTableStruxes,
							   bDontGlob);
}

/*!
    scan piecetable and remove any references to the hdr/ftr located at pos dpos
    bNotional indicates that the header has been marked deleted in revison mode, but not
    physically removed from the document
*/
bool pt_PieceTable::_fixHdrFtrReferences(const gchar * pszHdrType, const gchar * pszHdrId,
										 bool bNotional /* = false */)
{
	UT_return_val_if_fail( pszHdrType && pszHdrId, false );
	
	bool bRet = true;
	const PP_AttrProp * pAP = NULL;

	// look for any doc sections that referrence this header type and id
	const pf_Frag * pFrag = m_fragments.getFirst();
	while(pFrag)
	{
		if(pFrag->getType() == pf_Frag::PFT_Strux &&
		   static_cast<const pf_Frag_Strux*>(pFrag)->getStruxType()==PTX_Section)
		{
			if(!getAttrProp(pFrag->getIndexAP(),&pAP) || !pAP)
				continue;

			// check for normal attribute
			const gchar * pszMyHdrId2 = NULL;
			if(pAP->getAttribute(pszHdrType, pszMyHdrId2) && pszMyHdrId2)
			{
				if(0 == strcmp(pszMyHdrId2, pszHdrId))
				{
					const gchar* pAttrs [3];
					pAttrs[0] = pszHdrType;
					pAttrs[1] = pszMyHdrId2;
					pAttrs[2] = NULL;

					bRet &= _fmtChangeStruxWithNotify(PTC_RemoveFmt, (pf_Frag_Strux*)pFrag,
													  pAttrs, NULL, false);
				}
			}

			// now check for revision attribute ...
			const gchar * pszRevision;
			if(pAP->getAttribute("revision", pszRevision) && pszRevision)
			{
				bool bFound = false;
				PP_RevisionAttr Revisions(pszRevision);

				for(UT_uint32 i = 0; i < Revisions.getRevisionsCount(); ++i)
				{
					const PP_Revision * pRev2 = Revisions.getNthRevision(i);
					UT_return_val_if_fail( pRev2, false );

					const gchar * pszMyHdrId = NULL;
					if(pRev2->getAttribute(pszHdrType, pszMyHdrId) && pszMyHdrId)
					{
						if(0 != strcmp(pszHdrId, pszMyHdrId))
							continue;

						if(!bNotional)
						{
							// NB: this is safe, since we own the PP_RevisionAttr object
							// of local scope which in turn owns this revisions
							const_cast<PP_Revision*>(pRev2)->setAttribute(pszHdrType, "");
						}
						else
						{
							UT_uint32 iId = m_pDocument->getRevisionId();
							UT_uint32 iMinId;
							const PP_Revision * pRev = Revisions.getRevisionWithId(iId, iMinId);
							if(pRev)
							{
								// NB: this is safe, since we own the PP_RevisionAttr object
								// of local scope which in turn owns this revisions
								const_cast<PP_Revision*>(pRev)->setAttribute(pszHdrType, "");
							}
							else
							{
								// we have a section that references this header in
								// previous revision and has no changes in the current
								// revision, so we need to add a new revisions in which
								// the header is not referenced
								const gchar * pAttrs [3] = {pszHdrType, pszHdrId, NULL};
								Revisions.addRevision(iId, PP_REVISION_FMT_CHANGE, pAttrs, NULL);
							}
						}

						Revisions.forceDirty();
						bFound = true;
					}
				}
				
				if(bFound)
				{
					const gchar* pAttrs [3];
					pAttrs[0] = "revision";
					pAttrs[1] = Revisions.getXMLstring();
					pAttrs[2] = NULL;

					bRet &= _fmtChangeStruxWithNotify(PTC_SetFmt, (pf_Frag_Strux*)pFrag,
													  pAttrs, NULL, false);
				}
			}
								
								
		}
							
		pFrag = pFrag->getNext();
	}


	return bRet;
}

bool pt_PieceTable::_deleteSpan(pf_Frag_Text * pft, UT_uint32 fragOffset,
								PT_BufIndex bi, UT_uint32 length,
								pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	// perform simple delete of a span of text.
	// we assume that it is completely contained within this fragment.

	UT_return_val_if_fail (fragOffset+length <= pft->getLength(),false);

	SETP(ppfEnd, pft);
	SETP(pfragOffsetEnd, fragOffset);

	if (fragOffset == 0)
	{
		// the change is at the beginning of the fragment,

		if (length == pft->getLength())
		{
			// the change exactly matches the fragment, just delete the fragment.
			// as we delete it, see if the fragments around it can be coalesced.

			_unlinkFrag(pft,ppfEnd,pfragOffsetEnd);
			delete pft;
			return true;
		}

		// the change is a proper prefix within the fragment,
		// do a left-truncate on it.

		pft->adjustOffsetLength(m_varset.getBufIndex(bi,length),pft->getLength()-length);
		return true;
	}

	if (fragOffset+length == pft->getLength())
	{
		// the change is a proper suffix within the fragment,
		// do a right-truncate on it.

		pft->changeLength(fragOffset);

		SETP(ppfEnd, pft->getNext());
		SETP(pfragOffsetEnd, 0);

		return true;
	}

	// otherwise, the change is in the middle of the fragment.
	// we right-truncate the current fragment at the deletion
	// point and create a new fragment for the tail piece
	// beyond the end of the deletion.

	UT_uint32 startTail = fragOffset + length;
	UT_uint32 lenTail = pft->getLength() - startTail;
	PT_BufIndex biTail = m_varset.getBufIndex(pft->getBufIndex(),startTail);
	pf_Frag_Text * pftTail = new pf_Frag_Text(this,biTail,lenTail,pft->getIndexAP(),pft->getField());
	UT_return_val_if_fail (pftTail, false);
	pft->changeLength(fragOffset);
	m_fragments.insertFrag(pft,pftTail);

	SETP(ppfEnd, pftTail);
	SETP(pfragOffsetEnd, 0);

	return true;
}

bool pt_PieceTable::_deleteSpanWithNotify(PT_DocPosition dpos,
										  pf_Frag_Text * pft, UT_uint32 fragOffset,
										  UT_uint32 length,
										  pf_Frag_Strux * pfs,
										  pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd,
										  bool bAddChangeRec)
{
	// create a change record for this change and put it in the history.

	UT_return_val_if_fail (pfs, false);

	if (length == 0)					// TODO decide if this is an error.
	{
		xxx_UT_DEBUGMSG(("_deleteSpanWithNotify: length==0\n"));
		SETP(ppfEnd, pft->getNext());
		SETP(pfragOffsetEnd, 0);
		return true;
	}

	// we do this before the actual change because various fields that
	// we need are blown away during the delete.  we then notify all
	// listeners of the change.

	PT_BlockOffset blockOffset = _computeBlockOffset(pfs,pft) + fragOffset;

	PX_ChangeRecord_Span * pcr
		= new PX_ChangeRecord_Span(PX_ChangeRecord::PXT_DeleteSpan,
								   dpos, pft->getIndexAP(),
								   m_varset.getBufIndex(pft->getBufIndex(),fragOffset),
								   length,blockOffset,pft->getField());
	UT_return_val_if_fail (pcr, false);
	pcr->setDocument(m_pDocument);
	bool bResult = _deleteSpan(pft,fragOffset,pft->getBufIndex(),length,ppfEnd,pfragOffsetEnd);

	bool canCoalesce = _canCoalesceDeleteSpan(pcr);
	if (!bAddChangeRec || (canCoalesce && !m_pDocument->isCoalescingMasked()))
	{
		if (canCoalesce)
			m_history.coalesceHistory(pcr);

		m_pDocument->notifyListeners(pfs,pcr);
		delete pcr;
	}
	else
	{
		m_history.addChangeRecord(pcr);
		m_pDocument->notifyListeners(pfs,pcr);
	}

	return bResult;
}

bool pt_PieceTable::_canCoalesceDeleteSpan(PX_ChangeRecord_Span * pcrSpan) const
{
	// see if this record can be coalesced with the most recent undo record.

	UT_return_val_if_fail (pcrSpan->getType() == PX_ChangeRecord::PXT_DeleteSpan, false);

	PX_ChangeRecord * pcrUndo;
	if (!m_history.getUndo(&pcrUndo,true))
		return false;
	if (pcrSpan->getType() != pcrUndo->getType())
		return false;
	if (pcrSpan->getIndexAP() != pcrUndo->getIndexAP())
		return false;
	if((pcrUndo->isFromThisDoc() != pcrSpan->isFromThisDoc()))
	   return false;

	PX_ChangeRecord_Span * pcrUndoSpan = static_cast<PX_ChangeRecord_Span *>(pcrUndo);
	UT_uint32 lengthUndo = pcrUndoSpan->getLength();
	PT_BufIndex biUndo = pcrUndoSpan->getBufIndex();

	UT_uint32 lengthSpan = pcrSpan->getLength();
	PT_BufIndex biSpan = pcrSpan->getBufIndex();

	if (pcrSpan->getPosition() == pcrUndo->getPosition())
	{
		if (m_varset.getBufIndex(biUndo,lengthUndo) == biSpan)
			return true;				// a forward delete

		return false;
	}
	else if ((pcrSpan->getPosition() + lengthSpan) == pcrUndo->getPosition())
	{
		if (m_varset.getBufIndex(biSpan,lengthSpan) == biUndo)
			return true;				// a backward delete

		return false;
	}
	else
	{
		return false;
	}
}

bool pt_PieceTable::_isSimpleDeleteSpan(PT_DocPosition dpos1,
										PT_DocPosition dpos2) const
{
	// see if the amount of text to be deleted is completely
	// contained within the fragment found.  if so, we have
	// a simple delete.  otherwise, we need to set up a multi-step
	// delete -- it may not actually take more than one step, but
	// it is too complicated to tell at this point, so we assume
	// it will and don't worry about it.
	//
	// we are in a simple change if the beginning and end are
	// within the same fragment.

	pf_Frag * pf_First;
	pf_Frag * pf_End;
	PT_BlockOffset fragOffset_First;
	PT_BlockOffset fragOffset_End;

	bool bFound = getFragsFromPositions(dpos1,dpos2,&pf_First,&fragOffset_First,&pf_End,&fragOffset_End);
	UT_return_val_if_fail (bFound, false);

	if ((fragOffset_End==0) && pf_End->getPrev() && (pf_End->getPrev()->getType() == pf_Frag::PFT_Text))
	{
		pf_End = pf_End->getPrev();
		fragOffset_End = pf_End->getLength();
	}

	return (pf_First == pf_End);
}

bool pt_PieceTable::_tweakDeleteSpanOnce(PT_DocPosition & dpos1,
										 PT_DocPosition & dpos2,
										 UT_Stack * pstDelayStruxDelete) const
{
	if(m_bDoNotTweakPosition)
		return true;
	
	//  Our job is to adjust the end positions of the delete
	//  operating to delete those structural object that the
	//  user will expect to have deleted, even if the dpositions
	//  aren't quite right to encompass those.

	pf_Frag * pf_First;
	pf_Frag * pf_End;
	pf_Frag * pf_Other;
	PT_BlockOffset fragOffset_First;
	PT_BlockOffset fragOffset_End;

	bool bFound = getFragsFromPositions(dpos1,dpos2,&pf_First,&fragOffset_First,&pf_End,&fragOffset_End);
	UT_return_val_if_fail (bFound, false);

	pf_Frag_Strux * pfsContainer = NULL;
	bool bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
	UT_return_val_if_fail (bFoundStrux,false);

    _tweakFieldSpan(dpos1,dpos2);

	switch (pfsContainer->getStruxType())
	{
	default:
		UT_ASSERT_HARMLESS(0);
		return false;

	case PTX_Section:
		// if the previous container is a section, then pf_First
		// must be the first block in the section.
		UT_return_val_if_fail ((pf_First->getPrev() == pfsContainer),false);
		UT_return_val_if_fail ((pf_First->getType() == pf_Frag::PFT_Strux),false);
		UT_return_val_if_fail (((static_cast<pf_Frag_Strux *>(pf_First))->getStruxType() == PTX_Block),false);
		// We can delete the first block in a section only if the section then start
		// with either a new block or the table. We will allow it here.
		return true;
	case PTX_SectionHdrFtr:
		// if the previous container is a Header/Footersection, then pf_First
		// must be the first block or the first Table in the section.
		UT_return_val_if_fail ((pf_First->getPrev() == pfsContainer),false);
		UT_return_val_if_fail ((pf_First->getType() == pf_Frag::PFT_Strux),false);
		UT_return_val_if_fail ((((static_cast<pf_Frag_Strux *>(pf_First))->getStruxType() == PTX_Block) || (static_cast<pf_Frag_Strux *>(pf_First))->getStruxType() == PTX_SectionTable),false);

		//
		// This allows us to delete the first Table in a section
		//
		if(static_cast<pf_Frag_Strux *>(pf_First)->getStruxType() == PTX_SectionTable)
		{
		     return true;
		}
		// since, we cannot delete the first block in a section, we
		// secretly translate this into a request to delete the section;
		// the block we have will then be slurped into the previous
		// section.
		dpos1 -= pfsContainer->getLength();
		
		return true;

	case PTX_SectionTable:
	case PTX_SectionCell:
	case PTX_SectionFrame:
	case PTX_EndTable:
	case PTX_EndCell:
	case PTX_EndFrame:
	case PTX_SectionTOC:
	case PTX_EndTOC:
//
// We've set things up so that deleting table struxes is done very deliberately.//  Don't mess with the end points here
//
		return true;
	case PTX_SectionFootnote:
	case PTX_SectionAnnotation:
	case PTX_SectionEndnote:
	{
//
// Get the actual block strux container for the endnote. 
//
 		xxx_UT_DEBUGMSG(("_deleteSpan 1: orig pfsContainer %x type %d \n",pfsContainer,pfsContainer->getStruxType()));
 		_getStruxFromFragSkip(pfsContainer,&pfsContainer);
 		xxx_UT_DEBUGMSG(("_deleteSpan 2: After skip  pfsContainer %x type %d \n",pfsContainer,pfsContainer->getStruxType()));
 		break;
	}
 	case PTX_EndFootnote:	
 	case PTX_EndEndnote:	
 	case PTX_EndAnnotation:	
 	{
//
// Get the actual block strux container for the endnote. 
//
 		xxx_UT_DEBUGMSG(("_deleteSpan 1: orig pfsContainer %x type %d \n",pfsContainer,pfsContainer->getStruxType()));
 		_getStruxFromFragSkip(pfsContainer,&pfsContainer);
 		xxx_UT_DEBUGMSG(("_deleteSpan 2: After skip  pfsContainer %x type %d \n",pfsContainer,pfsContainer->getStruxType()));
 		break;
	}
	case PTX_Block:
		// if the previous container is a block, we're ok.
		// the loop below will take care of everything.
		break;
	}

	if (pf_First->getType() == pf_Frag::PFT_Strux)
	{
		switch(static_cast<pf_Frag_Strux *>(pf_First)->getStruxType())
		{
		default:
			break;

		case PTX_Section:
			UT_return_val_if_fail (fragOffset_First == 0,false);
			if (dpos2 == dpos1 + pf_First->getLength())
			{
				//  If we are just deleting a section break, then
				//  we should delete the first block marker in the
				//  next section, combining the blocks before and
				//  after the section break.
				pf_Other = pf_First->getNext();
				UT_return_val_if_fail (pf_Other, false);
				UT_return_val_if_fail (pf_Other->getType() == pf_Frag::PFT_Strux,false);
				UT_return_val_if_fail (((static_cast<pf_Frag_Strux *>(pf_Other))->getStruxType() == PTX_Block),false);
				dpos2 += pf_Other->getLength();
				return true;
			}
		case PTX_SectionHdrFtr:
			UT_return_val_if_fail (fragOffset_First == 0, false);
			if (dpos2 == dpos1 + pf_First->getLength())
			{
				//  If we are just deleting a section break, then
				//  we should delete the first block marker in the
				//  next section, combining the blocks before and
				//  after the section break.
				pf_Other = pf_First->getNext();
				UT_return_val_if_fail (pf_Other,false);
				UT_return_val_if_fail (pf_Other->getType() == pf_Frag::PFT_Strux,false);
				UT_return_val_if_fail (((static_cast<pf_Frag_Strux *>(pf_Other))->getStruxType() == PTX_Block),false);
				dpos2 += pf_Other->getLength();
				return true;
			}

			break;
		}
	}

	if(pf_End->getType() ==  pf_Frag::PFT_Strux)
	{
	    if(static_cast<pf_Frag_Strux *>(pf_End)->getStruxType() == PTX_EndTOC)
		{
			dpos2++;
		}
	}

	if (fragOffset_First == 0 && fragOffset_End == 0 && pf_First != pf_End)
	{
		pf_Frag * pf_Before = pf_First->getPrev();
		while (pf_Before && pf_Before->getType() == pf_Frag::PFT_FmtMark)
			pf_Before = pf_Before->getPrev();
		pf_Frag * pf_Last = pf_End->getPrev();
		while (pf_Last && pf_Last->getType() == pf_Frag::PFT_FmtMark)
			pf_Last = pf_Last->getPrev();

		if (pf_Before && pf_Before->getType() == pf_Frag::PFT_Strux &&
			pf_Last && pf_Last->getType() == pf_Frag::PFT_Strux)
		{
			PTStruxType pt_BeforeType = static_cast<pf_Frag_Strux *>(pf_Before)->getStruxType();
			PTStruxType pt_LastType = static_cast<pf_Frag_Strux *>(pf_Last)->getStruxType();

			if (pt_BeforeType == PTX_Block && pt_LastType == PTX_Block)
			{
				//  Check that there is something between the pf_Before and pf_Last, otherwise
                //  This leads to a segfault from continually pushing pf_Before onto the stack
                //  if we delete a whole lot of blank lines. These get popped off then deleted
                //  only to find the same pointer waiting to come off the stack.
				pf_Frag * pScan = pf_Before->getNext();
				while(pScan && pScan != pf_Last && (pScan->getType() != pf_Frag::PFT_Strux))
				{
					pScan = pScan->getNext();
				}
				if(pScan == pf_Last)
				{

				//  If we are the structure of the document is
				//  '[Block] ... [Block]' and we are deleting the
				//  '... [Block]' part, then the user is probably expecting
				//  us to delete '[Block] ... ' instead, so that any text
				//  following the second block marker retains its properties.
				//  The problem is that it might not be safe to delete the
				//  first block marker until the '...' is deleted because
				//  it might be the first block of the section.  So, we
				//  want to delete the '...' first, and then get around
				//  to deleting the block later.

					pf_Frag_Strux * pfs_BeforeSection, * pfs_LastSection;
					_getStruxOfTypeFromPosition(dpos1 - 1,
												PTX_Section, &pfs_BeforeSection);
					_getStruxOfTypeFromPosition(dpos2 - 1,
												PTX_Section, &pfs_LastSection);

					if ((pfs_BeforeSection == pfs_LastSection) && (dpos2 > dpos1 +1))
					{
						dpos2 -= pf_Last->getLength();
						pstDelayStruxDelete->push(pf_Before);
						return true;
					}
				}
			}
		}
	}

	return true;
}

bool pt_PieceTable::_tweakDeleteSpan(PT_DocPosition & dpos1,
									 PT_DocPosition & dpos2,
									 UT_Stack * pstDelayStruxDelete) const
{
	if(m_bDoNotTweakPosition)
		return true;
	
	//
// First we want to handle hyperlinks. If we delete all the text within
// a hyperlink or annotation, then we should also delete the start 
// and end point of the
// hyperlink or annotation.
//
	pf_Frag * pf_First;
	pf_Frag * pf_End;
	PT_BlockOffset fragOffset_First;
	PT_BlockOffset fragOffset_End;

	bool bFound = getFragsFromPositions(dpos1,dpos2,&pf_First,&fragOffset_First,&pf_End,&fragOffset_End);
	UT_return_val_if_fail (bFound,false);
	while(pf_First && (pf_First->getLength() == 0))
	{
		pf_First = pf_First->getNext();
	}
	if(pf_First)
	{
		while(pf_End && (pf_End->getLength() == 0))
		{
			pf_End = pf_End->getPrev();
		}
		bool bDoit = false;
		if(pf_End && ((pf_End->getPos() + pf_End->getLength() - pf_First->getPos())  == (dpos2 - dpos1 +1)))
		{
			if((pf_First->getType() == pf_Frag::PFT_Text) && (pf_First->getLength() == 2))
			{
				bDoit = false;
			}
			else if((pf_First->getType() == pf_Frag::PFT_Text) && (pf_End->getType() == pf_Frag::PFT_Text) && (pf_First != pf_End))
			{
				bDoit = false;
			}
			else
			{
				bDoit = true;
			}
		}
		if(pf_End && ((pf_End->getPos() + pf_End->getLength() - pf_First->getPos())  == (dpos2 - dpos1)))
		{
			bDoit = true;
		}
		if(bDoit)
		{
//
// OK these frags are entirely contained by dpos1  and dpos2
// OK now look to see if there is a hyperlink or annotation just before and after these
//
			if(pf_End->getType() != pf_Frag::PFT_Object)
			{
				pf_End = pf_End->getNext();
			}
			while(pf_End && (pf_End->getLength() == 0))
			{
				pf_End = pf_End->getNext();
			}
			if(pf_First->getType() != pf_Frag::PFT_Object)
			{
				pf_First = pf_First->getPrev();
			}
			while(pf_First && (pf_First->getLength() == 0))
			{
				pf_First = pf_First->getPrev();
			}
			if(pf_First && (pf_First->getType() == pf_Frag::PFT_Object))
			{
				pf_Frag_Object *pFO = static_cast<pf_Frag_Object *>(pf_First);
				bool bFoundBook = false;
				bool bFoundHype = false;
				bool bFoundAnn = false;
				if(pFO->getObjectType() == PTO_Bookmark)
				{
					bFoundBook = true;
				}
				if(pFO->getObjectType() == PTO_Hyperlink)
				{
					bFoundHype = true;
				}
				if(pFO->getObjectType() == PTO_Annotation)
				{
					bFoundAnn = true;
				}
				if(pf_End && (pf_End->getType() == pf_Frag::PFT_Object) && (pf_End != pf_First))
				{
					pFO = static_cast<pf_Frag_Object *>(pf_End);
					if(pFO->getObjectType() == PTO_Bookmark && bFoundBook)
					{
//
// Found a bookmark which will have all contents deleted so delete it too
//
						dpos1--;
						dpos2++;
					}
					else if(pFO->getObjectType() == PTO_Hyperlink && bFoundHype)
					{
//
// Found a Hyperlink which will have all contents deleted so delte it too
//
						dpos1--;
						dpos2++;
					}
					else if(pFO->getObjectType() == PTO_Annotation && bFoundAnn)
					{
//
// Found a Annotation which will have all contents deleted so delte it too
//
						dpos1--;
						dpos2++;
					}
				}
			}
		}
	}
//
// Can't handle a delete span start from an endTOC. sum1 has arranged corner
// cases where this is possible. HAndle this corner case by starting at the 
// next strux
//
	if(!pf_First)
	{
	    return false;
	}
	if(pf_First->getType() == pf_Frag::PFT_Strux)
	{
		pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf_First);
		if(pfs->getStruxType() == PTX_EndTOC)
		{
			pf_Frag * pf = pf_First->getNext();
			while(pf && pf->getLength() == 0)
			{
				pf = pf->getNext();
			}
			if(pf && pf->getType() ==  pf_Frag::PFT_Strux)
			{
				pfs = static_cast<pf_Frag_Strux *>(pf);
				if(pfs->getStruxType() == PTX_Block)
				{
					dpos1++;
				}
			}
		}
	}
	//  We want to keep tweaking the delete span until there is nothing
	//  more to tweak.  We check to see if nothing has changed in the
	//  last tweak, and if so, we are done.
	while (1)
	{
		PT_DocPosition old_dpos1 = dpos1;
		PT_DocPosition old_dpos2 = dpos2;
		UT_sint32 old_iStackSize = pstDelayStruxDelete->getDepth();

		if(!_tweakDeleteSpanOnce(dpos1, dpos2, pstDelayStruxDelete))
			return false;

		if (dpos1 == old_dpos1 && dpos2 == old_dpos2
			&& pstDelayStruxDelete->getDepth() == old_iStackSize)
			return true;
	}
}

bool pt_PieceTable::_deleteFormatting(PT_DocPosition dpos1,
									  PT_DocPosition dpos2)
{
	pf_Frag * pf_First;
	pf_Frag * pf_End;
	PT_BlockOffset fragOffset_First;
	PT_BlockOffset fragOffset_End;

	bool bFound = getFragsFromPositions(dpos1,dpos2,&pf_First,&fragOffset_First,&pf_End,&fragOffset_End);
	UT_return_val_if_fail (bFound,false);

	// before we delete the content, we do a quick scan and delete
	// any FmtMarks first -- this let's us avoid problems with
	// coalescing FmtMarks only to be deleted.

	pf_Frag * pfTemp = pf_First;
	PT_BlockOffset fragOffsetTemp = fragOffset_First;

	PT_DocPosition dposTemp = dpos1;
	while (dposTemp <= dpos2)
	{
		if (pfTemp->getType() == pf_Frag::PFT_EndOfDoc)
			break;

		if (pfTemp->getType() == pf_Frag::PFT_FmtMark)
		{
			pf_Frag * pfNewTemp;
			PT_BlockOffset fragOffsetNewTemp;
			pf_Frag_Strux * pfsContainerTemp = NULL;
			bool bFoundStrux = _getStruxFromPosition(dposTemp,&pfsContainerTemp);
			if(isEndFootnote(pfsContainerTemp))
			{
				xxx_UT_DEBUGMSG(("_deleteSpan 5: orig pfsContainer %x type %d \n",pfsContainerTemp,pfsContainerTemp->getStruxType()));
				_getStruxFromFragSkip(pfsContainerTemp,&pfsContainerTemp);
				xxx_UT_DEBUGMSG(("_deleteSpan 6: After skip  pfsContainer %x type %d \n",pfsContainerTemp,pfsContainerTemp->getStruxType()));
			}
			UT_return_val_if_fail (bFoundStrux,false);
			bool bResult = _deleteFmtMarkWithNotify(dposTemp,static_cast<pf_Frag_FmtMark *>(pfTemp),
										 pfsContainerTemp,&pfNewTemp,&fragOffsetNewTemp);
			UT_return_val_if_fail (bResult,false);

			// FmtMarks have length zero, so we don't need to update dposTemp.
			pfTemp = pfNewTemp;
			fragOffsetTemp = fragOffsetNewTemp;
		}
		else if(pfTemp->getType() == pf_Frag::PFT_Strux)
		{
			pf_Frag_Strux * pfFragStrux = static_cast<pf_Frag_Strux *>(pfTemp);
			if(pfFragStrux->getStruxType() == PTX_Section)
			{
				pf_Frag_Strux_Section * pfSec = static_cast<pf_Frag_Strux_Section *>(pfFragStrux);
				_deleteHdrFtrsFromSectionStruxIfPresent(pfSec);
			}
			dposTemp += pfTemp->getLength() - fragOffsetTemp;
			pfTemp = pfTemp->getNext();
			fragOffsetTemp = 0;
		}
		else
		{
			dposTemp += pfTemp->getLength() - fragOffsetTemp;
			pfTemp = pfTemp->getNext();
			fragOffsetTemp = 0;
		}
	}

	return true;
}

/*!
 * Returns true if pfs is not a strux connected with a table or frame
 */
bool pt_PieceTable::_StruxIsNotTable(pf_Frag_Strux * pfs)
{
	PTStruxType its = pfs->getStruxType();
	bool b = ((its != PTX_SectionTable) && (its != PTX_SectionCell)
			  && (its != PTX_EndTable) && (its != PTX_EndCell)
			  && (its != PTX_SectionFrame) && (its != PTX_EndFrame));
	return b;
}


/**
 * Since hyperlinks, annotations, and rdf anchors are all very
 * similar code, they are abstracted out here.
 */
bool
pt_PieceTable::_deleteComplexSpanHAR( pf_Frag_Object *pO,
                                      PT_DocPosition dpos1,
                                      PT_DocPosition /*dpos2*/,
                                      UT_uint32& length,
                                      PT_BlockOffset& fragOffset_First,
                                      UT_uint32& lengthThisStep,
                                      pf_Frag_Strux*& pfsContainer,
                                      pf_Frag*& pfNewEnd,
                                      UT_uint32& fragOffsetNewEnd,
                                      const char* startAttrCSTR )
{
    UT_DEBUGMSG(("_deleteComplexSpanHAR() pO:%p\n", pO ));
    
    PTObjectType objType = pO->getObjectType();
    bool bFoundStrux2;
    bool bResult = false;
    UT_DebugOnly<bool> bResult2;
    PT_DocPosition posComrade;
    pf_Frag_Strux * pfsContainer2 = NULL;
    pf_Frag * pF;
    std::string startAttr = startAttrCSTR;
    std::string startAttrInitialCap = startAttr;
    if( !startAttrInitialCap.empty() )
        startAttrInitialCap[0] = toupper( startAttrInitialCap[0] );

    const PP_AttrProp * pAP = NULL;
    pO->getPieceTable()->getAttrProp(pO->getIndexAP(),&pAP);
    UT_return_val_if_fail (pAP, false);
    const gchar* pszHref = NULL;
    const gchar* pszHname  = NULL;
    UT_uint32 k = 0;
    bool bStart = false;
    while((pAP)->getNthAttribute(k++,pszHname, pszHref))
    {
        if((strcmp(pszHname, startAttr.c_str()) == 0) ||(strcmp(pszHname, startAttrInitialCap.c_str()) == 0) )
        {
            bStart = true;
            break;
        }
    }
    UT_DEBUGMSG(("_deleteComplexSpanHAR() bStart:%d\n", bStart ));
    
    if(!bStart)
    {
        // in this case we are looking for the start marker
        // and so we delete it and then move on
        pF = pO->getPrev();
        while(pF)
        {
            if(pF->getType() == pf_Frag::PFT_Object)
            {
                pf_Frag_Object *pOb = static_cast<pf_Frag_Object*>(pF);
                if( pOb->getObjectType() == objType )
                {
                    posComrade = getFragPosition(pOb);
                    bFoundStrux2 = _getStruxFromFragSkip(pOb,&pfsContainer2);
                    UT_return_val_if_fail (bFoundStrux2, false);

                    bResult2 =
                        _deleteObjectWithNotify(posComrade,pOb,0,1,
                                                pfsContainer2,0,0);
		    UT_ASSERT(bResult2);

                    // now adjusting the positional variables
                    if(posComrade <= dpos1)
                        // delete before the start of the segement we are working on
                        dpos1--;
                    else
                    {
                        // we are inside that section
                        length--;
                    }
                    break;
                }
            }
            pF = pF->getPrev();
        }
        UT_ASSERT(pO->getObjectType() == objType);
        bResult
            = _deleteObjectWithNotify(dpos1,pO,fragOffset_First,lengthThisStep,
                                      pfsContainer,&pfNewEnd,&fragOffsetNewEnd);

    }
    else
    {
        // in this case we are looking for the end marker,
        // so we have to be careful to get rid of the start
        // marker first
        pF = pO->getNext();
        while(pF)
        {
            UT_DEBUGMSG(("_deleteComplexSpanHAR() loop pF:%p\n", pF ));
            if(pF->getType() == pf_Frag::PFT_Object)
            {
                pf_Frag_Object *pOb = static_cast<pf_Frag_Object*>(pF);
                if( pOb->getObjectType() == objType )
                {
                    posComrade = getFragPosition(pOb);
                    bFoundStrux2 = _getStruxFromFragSkip(pOb,&pfsContainer2);
                    UT_return_val_if_fail (bFoundStrux2, false);

                    UT_DEBUGMSG(("_deleteComplexSpanHAR() posComrade:%d\n", posComrade ));
                    
                    // delete the original start marker
                    bResult
                        = _deleteObjectWithNotify(dpos1,pO,fragOffset_First,lengthThisStep,
                                                  pfsContainer,&pfNewEnd,&fragOffsetNewEnd);

                    // now adjusting the positional variables
                    posComrade--;


                    // One last twist make sure the next frag from the previous
                    // delete isn't the same as this this other we need to get the next frag from the
                    // second delete
                    if(pfNewEnd != static_cast<pf_Frag *>(pOb))
                    {
                        bResult2 =
                            _deleteObjectWithNotify(posComrade,pOb,0,1,
                                                    pfsContainer2,0,0);
			UT_ASSERT(bResult2);
                    }
                    else
                    {
                        bResult2 =
                            _deleteObjectWithNotify(posComrade,pOb,0,1,
                                                    pfsContainer2,&pfNewEnd,&fragOffsetNewEnd);
			UT_ASSERT(bResult2);
                    }

                    if( objType == PTO_Annotation )
                    {
                        // FIXME!! Need to work out how to delete the content of the annotation
                        // at this point
                    }
                    
                    if(posComrade >= dpos1 && posComrade <= dpos1 + length - 2)
                    {
                        // the endmarker was inside of the segment we are working
                        // so we have to adjust the length
                        length--;
                    }

                    break;
                }
            }
            pF = pF->getNext();
        }
    }
                
    return bResult;
}


/*
    Because complex span can involve deletion of a bookmark the comrade of which is outside of the
    deletion range, this function can change dpos1 and dpos2 to indicate which document positions
    after the deletion correspond to the original values passed to the function -- the caller needs
    to take this into account
 */
bool pt_PieceTable::_deleteComplexSpan(PT_DocPosition & origPos1,
									   PT_DocPosition & origPos2,
									   UT_Stack * stDelayStruxDelete)
{
	pf_Frag * pfNewEnd = NULL;
	UT_uint32 fragOffsetNewEnd = 0;
	bool bPrevWasCell = false;
	bool bPrevWasEndTable = false;
	bool bPrevWasFrame = false;
	pf_Frag * pf_First;
	pf_Frag * pf_End;
	PT_BlockOffset fragOffset_First;
	PT_BlockOffset fragOffset_End;

	PT_DocPosition dpos1 = origPos1;
	PT_DocPosition dpos2 = origPos2;

	bool bFound = getFragsFromPositions(dpos1,dpos2,&pf_First,&fragOffset_First,&pf_End,&fragOffset_End);
	UT_return_val_if_fail (bFound, false);
	UT_DEBUGMSG(("deleteComplex span dpos1 %d dpos2 %d pf_First %p pf_First pos %d \n",dpos1,dpos2,pf_First,pf_First->getPos()));
	pf_Frag_Strux * pfsFirstBlock = NULL;
	if ((pf_First !=pf_End) && (pf_First->getType() == pf_Frag::PFT_Strux))
	{
	    pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux*>(pf_First);
		if ((pfs->getStruxType() == PTX_Block) && pfs->getPrev() &&
			(pfs->getPrev()->getType() == pf_Frag::PFT_Strux) &&
			(static_cast<pf_Frag_Strux*>(pfs->getPrev())->getStruxType() == PTX_Section))
		{
			// We are trying to delete the first block of the section. We will keep that for the end
			pfsFirstBlock = static_cast<pf_Frag_Strux*>(pf_First);
			dpos1 = dpos1 + 1;
			bFound = getFragsFromPositions(dpos1,dpos2,&pf_First,&fragOffset_First,&pf_End,&fragOffset_End);
			UT_return_val_if_fail (bFound, false);
		}
	}

	pf_Frag_Strux * pfsContainer = NULL;
	bool bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
	UT_return_val_if_fail (bFoundStrux, false);
	if(isEndFootnote(pfsContainer))
	{
		xxx_UT_DEBUGMSG(("_deleteSpan 3: orig pfsContainer %x type %d \n",pfsContainer,pfsContainer->getStruxType()));
		_getStruxFromFragSkip(pfsContainer,&pfsContainer);
		xxx_UT_DEBUGMSG(("_deleteSpan 4: After skip  pfsContainer %x type %d \n",pfsContainer,pfsContainer->getStruxType()));
	}
	// loop to delete the amount requested, one text fragment at a time.
	// if we encounter any non-text fragments along the way, we delete
	// them too.  that is, we implicitly delete Strux and Objects here.

	UT_uint32 length = dpos2 - dpos1;
	UT_uint32 iTable = 0;
	UT_sint32 iFootnoteCount = 0;
	if(pfsContainer->getStruxType() == PTX_SectionFrame)
	{
		bPrevWasFrame = true;
	        if(pf_First->getType() == pf_Frag::PFT_Strux)
		{
		     pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf_First);
		     if(pfs->getStruxType() == PTX_SectionTable)
		     {
		          bPrevWasFrame = false;
		     }
		}
	}
	if(pfsContainer->getStruxType() == PTX_SectionCell)
	{
		bPrevWasCell = true;
	}
	if(pfsContainer->getStruxType() == PTX_EndTable)
	{
		bPrevWasEndTable = true;
	}
	bool bPrevWasFootnote = false;
	UT_sint32 iLoopCount = -1;
	while ((length > 0) || (iFootnoteCount > 0))
	{
		UT_DEBUGMSG(("_deleteComplexSpan() len:%d\n", length ));
		iLoopCount++;
		UT_uint32 lengthInFrag = pf_First->getLength() - fragOffset_First;
		UT_uint32 lengthThisStep = UT_MIN(lengthInFrag, length);
		
		switch (pf_First->getType())
		{
		case pf_Frag::PFT_EndOfDoc:
		default:
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			return false;

		case pf_Frag::PFT_Strux:
		{
//
// OK this code is leave the cell/table structures in place unless we
// defiantely want to delete them.
//
			pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *> (pf_First);			
			bool bResult = true;
			if(bPrevWasCell && (iLoopCount == 0))
			{
//
// Dont' delete this Block strux if the previous strux is a cell and this
// is the start of the delete phase.
//
				pfNewEnd = pfs->getNext();
				fragOffsetNewEnd = 0;
				pfsContainer = pfs;
				dpos1 = dpos1 +  lengthInFrag;
				break;
			}
			if(bPrevWasFrame && (iLoopCount == 0))
			{
//
// Dont' delete this Block strux if the previous strux is a cell and this
// is the start of the delete phase.
//
				pfNewEnd = pfs->getNext();
				fragOffsetNewEnd = 0;
				pfsContainer = pfs;
				dpos1 = dpos1 +  lengthInFrag;
				break;
			}
			if(_StruxIsNotTable(pfs))
			{
				if((bPrevWasCell || bPrevWasFootnote || bPrevWasEndTable|| bPrevWasFrame) 
				   && pfs->getStruxType()== PTX_Block)
				{
					bPrevWasCell = false;
					bPrevWasFootnote = false;
					bPrevWasEndTable = false;
					bPrevWasFrame = false;
					pfNewEnd = pfs->getNext();
					fragOffsetNewEnd = 0;
					pfsContainer = pfs;
					dpos1 = dpos1 +  lengthInFrag;
					stDelayStruxDelete->push(pfs);
				}
				else
				{
//
// Now put in code to handle deleting footnote struxtures. We have to reverse
// the order of deleting footnote and Endfootnotes.
// 
					if(!isFootnote(pfs) && !isEndFootnote(pfs))
					{ 
						UT_DEBUGMSG(("Delete Block strux dpos1 %d \n",dpos1));
						bResult = _deleteStruxWithNotify(dpos1,pfs,
										 &pfNewEnd,&fragOffsetNewEnd);
						
						if(!bResult) // can't delete this block strux
						             // but can delete the rest of the content
						{
						  UT_DEBUGMSG(("dpos1 = %d \n",dpos1));
						  pfNewEnd = pfs->getNext();
						  dpos1 += pfs->getLength();
						  pfsContainer = pfs;
						  fragOffsetNewEnd = 0;
						}
						// UT_return_val_if_fail(bResult,false);
						bPrevWasCell = false;
						bPrevWasFrame = false;
						bPrevWasEndTable = false;
						break;
					}
					else
					{
						if(isFootnote(pfs))
						{
							pfNewEnd = pfs->getNext();
							fragOffsetNewEnd = 0;
							pfsContainer = pfs;
							dpos1 = dpos1 +  lengthInFrag;
							UT_DEBUGMSG(("Push footnote strux \n"));
							stDelayStruxDelete->push(pfs);
							iFootnoteCount++;
							bPrevWasFootnote = true;
						}
						else
						{
							UT_DEBUGMSG(("Push endfootnote strux \n"));
							stDelayStruxDelete->push(pfs);
						}
					}
				}
			}
			else
			{
			  bPrevWasCell = (pfs->getStruxType() == PTX_SectionCell );
			  bPrevWasFrame = (pfs->getStruxType() == PTX_SectionFrame );
			  bPrevWasEndTable = (pfs->getStruxType() == PTX_EndTable);
				if(pfs->getStruxType() == PTX_SectionTable)
				{
					iTable++;
				}
				if((pfs->getStruxType() != PTX_EndTable) || (iTable != 1))
				{
					pfNewEnd = pfs->getNext();
					fragOffsetNewEnd = 0;
					dpos1 = dpos1 + lengthInFrag;
				}
				stDelayStruxDelete->push(pfs);
			}
//
// Look to see if we've reached the end of the table, if so delete it all now
//
			pf_Frag *pff;
			PT_DocPosition dpp;
			if(pfs->getStruxType() == PTX_EndTable)
			{
				iTable--;
				if(iTable==0)
				{
//
// Deleting the table so don't delay deleting the following strux.
//
					bPrevWasEndTable = false;
					UT_DEBUGMSG(("Doing Table delete immediately \n"));
//					iTable = 1;
					UT_sint32 myTable =1;
//
// First delete the EndTable Strux
//
					stDelayStruxDelete->pop(reinterpret_cast<void **>(&pfs));
					PT_DocPosition myPos2 = pfs->getPos();
					_deleteFormatting(myPos2 - pfs->getLength(), myPos2);
					UT_DEBUGMSG(("DELeteing EndTable Strux, pos= %d \n",pfs->getPos()));
					bResult = _deleteStruxWithNotify(myPos2, pfs,
													  &pfNewEnd,
													  &fragOffsetNewEnd);
					while(bResult && myTable > 0)
					{
						stDelayStruxDelete->pop(reinterpret_cast<void **>(&pfs));
						if(pfs->getStruxType() == PTX_SectionTable)
						{
							myTable--;
						}
						else if(pfs->getStruxType() == PTX_EndTable)
						{
							myTable++;
						}
						PT_DocPosition myPos = pfs->getPos();
						_deleteFormatting(myPos - pfs->getLength(), myPos);
						bResult = _deleteStruxWithNotify(myPos, pfs, &pff, &dpp);
//
// Each strux is one in length, we've added one while delaying the delete
// so subtract it now.
//
						dpos1 -= 1;
					}
//
// Now we have to update pfsContainer from dpos1
//
					bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
				}
				UT_DEBUGMSG(("Finished doing table delete -1 \n"));
				break;
			}
//
// Look to see if we've reached the end of a footnote section.
//
			if (isEndFootnote(pfs) && (iFootnoteCount > 0))
			{
//
// First delete the EndFootnote Strux
//
				UT_DEBUGMSG(("Doing Footnote delete immediately \n"));
				stDelayStruxDelete->pop(reinterpret_cast<void **>(&pfs));

				PT_DocPosition myPos2 = pfs->getPos();
				_deleteFormatting(myPos2 - pfs->getLength(), myPos2);
				bResult = _deleteStruxWithNotify(myPos2, pfs,
												  &pfNewEnd,
												  &fragOffsetNewEnd);
//
// Now delete the Footnote strux. Doing things in this order works for
// the layout classes for both the delete (needs the endFootnote strux 
// deleted first) and for
// undo where we want the Footnote Strux inserted first.
//
				while(bResult && iFootnoteCount > 0)
				{
					stDelayStruxDelete->pop(reinterpret_cast<void **>(&pfs));
					if(isFootnote(pfs))
					{
						UT_DEBUGMSG(("Found and deleted footnote strux \n"));
						iFootnoteCount--;
					}
					else
					{
						UT_DEBUGMSG(("Found and deleted Block strux in footnote \n"));
					}
					PT_DocPosition myPos = pfs->getPos();
					_deleteFormatting(myPos - pfs->getLength(), myPos);
					bResult = _deleteStruxWithNotify(myPos, pfs, &pff, &dpp);
//
// Each strux is one in length, we've added one while delaying the delete
// so subtract it now.
//
					dpos1 -= 1;
				}
//
// Now we have to update pfsContainer from dpos1
//
				bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
				break;
			}
			else if(isEndFootnote(pfs))
			{
//
// Attempting to delete an EndFootnote end strux without a matching begin.
// terminate the loop now.
//
				return false;

			}
//
// Look to see if we've reached the end of a Frame section.
//
			if (pfs->getStruxType() == PTX_EndFrame)
			{
//
// First delete the EndFrame Strux
//
				UT_DEBUGMSG(("Doing Frame delete now \n"));
				stDelayStruxDelete->pop(reinterpret_cast<void **>(&pfs));
				PT_DocPosition myPos2 = pfs->getPos();
				_deleteFormatting(myPos2 - pfs->getLength(), myPos2);
				bool isFrame =  pfs->getStruxType() == PTX_SectionFrame;
				bResult = _deleteStruxWithNotify(myPos2, pfs,
												  &pfNewEnd,
												  &fragOffsetNewEnd);
//
// Each strux is one in length, we've added one while delaying the delete
// so subtract it now.
//
				dpos1 -= 1;
//
// Now delete the Frame strux. Doing things in this order works for
// the layout classes for both the delete (needs the endFrame strux 
// deleted first) and for
// undo where we want the Frame Strux inserted first.
//
				while(bResult && !isFrame)
				{
					stDelayStruxDelete->pop(reinterpret_cast<void **>(&pfs));
					PT_DocPosition myPos = 0;
					if(pfs)
					{
						myPos = pfs->getPos();
						isFrame =  pfs->getStruxType() == PTX_SectionFrame;
						_deleteFormatting(myPos - pfs->getLength(), myPos);
						bResult = _deleteStruxWithNotify(myPos, pfs, &pff, &dpp);
					}
//
// Each strux is one in length, we've added one while delaying the delete
// so subtract it now.
//
					dpos1 -= 1;
				}
//
// Now we have to update pfsContainer from dpos1
//
				bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
				break;
			}
			
			UT_return_val_if_fail (bResult,false);
			// we do not update pfsContainer because we just deleted pfs.
		}
		break;

		case pf_Frag::PFT_Text:
		{
			if(isEndFootnote(pfsContainer))
			{
				_getStruxFromFragSkip(static_cast<pf_Frag *>(pfsContainer),&pfsContainer);
			}
			bool bResult = _deleteSpanWithNotify(dpos1,static_cast<pf_Frag_Text *>(pf_First),
									  fragOffset_First,lengthThisStep,
									  pfsContainer,&pfNewEnd,&fragOffsetNewEnd);
			UT_return_val_if_fail (bResult, false);
		}
		break;
        //
		// the bookmark, hyperlink and annotation objects require special treatment; since
		// they come in pairs, we have to ensure that both are deleted together
		// so we have to find the other part of the pair, delete it, adjust the
		// positional variables and the delete the one we were originally asked
		// to delete
        //
		case pf_Frag::PFT_Object:
		{
			if(isEndFootnote(pfsContainer))
			{
				_getStruxFromFragSkip(static_cast<pf_Frag *>(pfsContainer),&pfsContainer);
			}
			bool bResult = false;
			UT_DebugOnly<bool> bResult2;
			pf_Frag_Object *pO = static_cast<pf_Frag_Object *>(pf_First);
			switch(pO->getObjectType())
			{
				case PTO_Bookmark:
				{
					bool bFoundStrux3;
					PT_DocPosition posComrade;
					pf_Frag_Strux * pfsContainer2 = NULL;

					po_Bookmark * pB = pO->getBookmark();
					UT_return_val_if_fail (pB, false);
					pf_Frag * pF;
					if(pB->getBookmarkType() == po_Bookmark::POBOOKMARK_END)
					{
				    	pF = pO->getPrev();
				    	while(pF)
				    	{
							if(pF->getType() == pf_Frag::PFT_Object)
							{
								pf_Frag_Object *pOb = static_cast<pf_Frag_Object*>(pF);
								po_Bookmark * pB1 = pOb->getBookmark();
								if(pB1 && !strcmp(pB->getName(),pB1->getName()))
								{
									m_pDocument->removeBookmark(pB1->getName());

									posComrade = getFragPosition(pOb);
									
									if(posComrade < origPos1)
									{
										origPos1--;
									}
									
									bFoundStrux3 = _getStruxFromFragSkip(pOb,&pfsContainer2);
									UT_return_val_if_fail (bFoundStrux3, false);

									bResult2 =
											_deleteObjectWithNotify(posComrade,pOb,0,1,
										  							pfsContainer2,0,0);
									UT_ASSERT(bResult2);

									// now adjusting the positional variables
									if(posComrade <= dpos1)
										// delete before the start of the segement we are working on
										dpos1--;
									else
									{
										// we are inside that section
										length--;

									}
									break;
								}
							}
							pF = pF->getPrev();
				    	}
					}
					else
					{
				    	pF = pO->getNext();
				    	while(pF)
				    	{
							if(pF->getType() == pf_Frag::PFT_Object)
							{
								pf_Frag_Object *pOb = static_cast<pf_Frag_Object*>(pF);
								po_Bookmark * pB1 = pOb->getBookmark();
								if(pB1 && !strcmp(pB->getName(),pB1->getName()))
								{
									m_pDocument->removeBookmark(pB1->getName());

									posComrade = getFragPosition(pOb);
									bool bFoundStrux2 = _getStruxFromFragSkip(pOb,&pfsContainer2);
									UT_return_val_if_fail (bFoundStrux2, false);

									bResult2 =
											_deleteObjectWithNotify(posComrade,pOb,0,1,
										  							pfsContainer2,0,0);
									if(posComrade < dpos1 + length)
										length--;
									break;
								}
							}
							pF = pF->getNext();
				    	}
					}
				bResult
					= _deleteObjectWithNotify(dpos1,pO,fragOffset_First,lengthThisStep,
									  pfsContainer,&pfNewEnd,&fragOffsetNewEnd);

				}
				break;
                //
				// the one singnificant difference compared to the bookmarks is
				// that we have to always delete the start marker first; this is
				// so that in the case of undo the endmarker would be restored
				// first, because the mechanism that marks runs between them
				// as a hyperlink depends on the end-marker being in place before
				// the start marker
                //
				case PTO_Hyperlink:
                    bResult = _deleteComplexSpanHAR( pO,
                                                      dpos1,
                                                      dpos2,
                                                      length,
                                                      fragOffset_First,
                                                      lengthThisStep,
                                                      pfsContainer,
                                                      pfNewEnd,
                                                      fragOffsetNewEnd,
                                                      "xlink:href" );
                // {
				//      bool bFoundStrux2;
				//      PT_DocPosition posComrade;
				//      pf_Frag_Strux * pfsContainer2 = NULL;

				//      pf_Frag * pF;

				//      const PP_AttrProp * pAP = NULL;
				//      pO->getPieceTable()->getAttrProp(pO->getIndexAP(),&pAP);
				//      UT_return_val_if_fail (pAP, false);
				//      const gchar* pszHref = NULL;
				//      const gchar* pszHname  = NULL;
				//      UT_uint32 k = 0;
				//      bool bStart = false;
				//      while((pAP)->getNthAttribute(k++,pszHname, pszHref))
				//      {
				//           if(!strcmp(pszHname, "xlink:href"))
				//     	  {
				//     	      bStart = true;
				// 	      break;
				// 	  }
				//      }

				//      if(!bStart)
				//      {
				// 		// in this case we are looking for the start marker
				// 		// and so we delete it and then move on
				//           pF = pO->getPrev();
				// 	  while(pF)
				// 	  {
				// 	      if(pF->getType() == pf_Frag::PFT_Object)
				// 	      {
				// 		   pf_Frag_Object *pOb = static_cast<pf_Frag_Object*>(pF);
				// 		   if(pOb->getObjectType() == PTO_Hyperlink)
				// 		   {
				// 		        posComrade = getFragPosition(pOb);
				// 			bFoundStrux2 = _getStruxFromFragSkip(pOb,&pfsContainer2);
				// 			UT_return_val_if_fail (bFoundStrux2, false);


				// 			xxx_UT_DEBUGMSG(("Deleting End Hyperlink 1 %p \n",pOb));
				// 			bResult2 =
				// 			  _deleteObjectWithNotify(posComrade,pOb,0,1,
				// 						  pfsContainer2,0,0);

				// 			// now adjusting the positional variables
				// 			if(posComrade <= dpos1)
				// 			  // delete before the start of the segement we are working on
				// 			     dpos1--;
				// 			else
				// 			{
				// 			     // we are inside that section
				// 			     length--;
				// 			}
				// 			break;
				// 		   }
				// 	      }
				// 	      pF = pF->getPrev();
				// 	  }

				// 	  xxx_UT_DEBUGMSG(("Deleting Start Hyperlink 1 %p \n",pO));
				// 	  bResult
				// 	    = _deleteObjectWithNotify(dpos1,pO,fragOffset_First,lengthThisStep,
				// 						  pfsContainer,&pfNewEnd,&fragOffsetNewEnd);

				//      }
				//      else
				//      {
				//        // in this case we are looking for the end marker,
				//        // so we have to be carefult the get rid of the start
				//        // marker first
				//     	   pF = pO->getNext();
				// 	   while(pF)
				// 	   {
				// 	        if(pF->getType() == pf_Frag::PFT_Object)
				// 		{
				// 		     pf_Frag_Object *pOb = static_cast<pf_Frag_Object*>(pF);
				// 		     if(pOb->getObjectType() == PTO_Hyperlink)
				// 		     {
				// 		          posComrade = getFragPosition(pOb);
				// 			  bFoundStrux2 = _getStruxFromFragSkip(pOb,&pfsContainer2);
				// 			  UT_return_val_if_fail (bFoundStrux2, false);
				// 			  // delete the original start marker

				// 			  xxx_UT_DEBUGMSG(("Deleting Start Hyperlink 2 %p \n",pO));
				// 			  bResult
				// 			    = _deleteObjectWithNotify(dpos1,pO,fragOffset_First,lengthThisStep,
				// 						      pfsContainer,&pfNewEnd,&fragOffsetNewEnd);

				// 			  // now adjusting the positional variables
				// 			  posComrade--;
				// 			  xxx_UT_DEBUGMSG(("Deleting End Hyperlink 2 %p \n",pOb));
				// 			  // One last twist make sure the next frag from the previous
				// 			  // delete isn't the same as this this other we need to get the next frag from thi
				// 			  // second delete
				// 			  if(pfNewEnd != static_cast<pf_Frag *>(pOb))
				// 			  {
				// 			        bResult2 =
				// 				  _deleteObjectWithNotify(posComrade,pOb,0,1,
				// 							pfsContainer2,0,0);
				// 			  }
				// 			  else
				// 			  {
				// 			        bResult2 =
				// 				  _deleteObjectWithNotify(posComrade,pOb,0,1,
				// 							pfsContainer2,&pfNewEnd,&fragOffsetNewEnd);
				// 			  }

				// 			  if(posComrade >= dpos1 && posComrade <= dpos1 + length - 2)
				// 			  {
				// 			    // the endmarker was inside of the segment we are working
				// 			    // so we have to adjust the length
				// 			       length--;
				// 			  }

				// 			  break;
				// 		     }
				// 		}
				// 		pF = pF->getNext();
				// 	   }
				//      }
				// }
				xxx_UT_DEBUGMSG(("Finished Deleting Hyperlink %p \n",pO));
				break;

				//
				// When deleting either the start or end of annotation we have to deleted
				// the content of the annotation as well.
				// We have to always delete the start marker first; this is
				// so that in the case of undo the endmarker would be restored
				// first, because the mechanism that marks runs between them
				// as a hyperlink depents on the end-marker being in place before
				// the start marker
                //
				case PTO_Annotation:
                    bResult = _deleteComplexSpanHAR( pO,
                                                      dpos1,
                                                      dpos2,
                                                      length,
                                                      fragOffset_First,
                                                      lengthThisStep,
                                                      pfsContainer,
                                                      pfNewEnd,
                                                      fragOffsetNewEnd,
                                                      "annotation" );
                    //   {
                //     UT_ASSERT(pO->getObjectType() == PTO_Annotation);
				//     bool bFoundStrux2;
				//     PT_DocPosition posComrade;
				//     pf_Frag_Strux * pfsContainer2 = NULL;

				//     pf_Frag * pF;

				//     const PP_AttrProp * pAP = NULL;
				//     pO->getPieceTable()->getAttrProp(pO->getIndexAP(),&pAP);
				//     UT_return_val_if_fail (pAP, false);
				//     const gchar* pszHref = NULL;
				//     const gchar* pszHname  = NULL;
				//     UT_uint32 k = 0;
				//     bool bStart = false;
				//     while((pAP)->getNthAttribute(k++,pszHname, pszHref))
				//     {
				//       if((strcmp(pszHname, "annotation") == 0) ||(strcmp(pszHname, "Annotation") == 0) )
				//     	{
				//     		bStart = true;
		    	// 			break;
				//     	}
				//     }

				// 	if(!bStart)
				// 	{
				// 		// in this case we are looking for the start marker
				// 		// and so we delete it and then move on
				//     	     pF = pO->getPrev();
				// 	     while(pF)
				// 	     {
				// 	          if(pF->getType() == pf_Frag::PFT_Object)
				// 		  {
				// 		       pf_Frag_Object *pOb = static_cast<pf_Frag_Object*>(pF);
				// 		       if(pOb->getObjectType() == PTO_Annotation)
				// 		       {
				// 			   posComrade = getFragPosition(pOb);
				// 			   bFoundStrux2 = _getStruxFromFragSkip(pOb,&pfsContainer2);
				// 			   UT_return_val_if_fail (bFoundStrux2, false);

				// 			   bResult2 =
				// 			     _deleteObjectWithNotify(posComrade,pOb,0,1,
				// 						     pfsContainer2,0,0);

				// 			   // now adjusting the positional variables
				// 			   if(posComrade <= dpos1)
				// 			     // delete before the start of the segement we are working on
				// 			        dpos1--;
				// 			   else
				// 			   {
				// 			        // we are inside that section
				// 			        length--;
				// 			   }
				// 			   break;
				// 		       }
				// 		  }
				// 		  pF = pF->getPrev();
				// 	     }
				// 	     UT_ASSERT(pO->getObjectType() == PTO_Annotation);
				// 	     bResult
				// 	       = _deleteObjectWithNotify(dpos1,pO,fragOffset_First,lengthThisStep,
				// 				    pfsContainer,&pfNewEnd,&fragOffsetNewEnd);

				// 	}
				// 	else
				// 	{
				// 		// in this case we are looking for the end marker,
				// 		// so we have to be carefult the get rid of the start
				// 		// marker first
				// 	     pF = pO->getNext();
				// 	     while(pF)
				// 	     {
				// 	         if(pF->getType() == pf_Frag::PFT_Object)
				// 		 {
				// 		     pf_Frag_Object *pOb = static_cast<pf_Frag_Object*>(pF);
				// 		     if(pOb->getObjectType() == PTO_Annotation)
				// 		     {
				// 		         posComrade = getFragPosition(pOb);
				// 			 bFoundStrux2 = _getStruxFromFragSkip(pOb,&pfsContainer2);
				// 			 UT_return_val_if_fail (bFoundStrux2, false);
				// 	                // delete the original start marker
				// 			 bResult
				// 			   = _deleteObjectWithNotify(dpos1,pO,fragOffset_First,lengthThisStep,
				// 						     pfsContainer,&pfNewEnd,&fragOffsetNewEnd);

				// 			 // now adjusting the positional variables
				// 			 posComrade--;


				// 			 // One last twist make sure the next frag from the previous
				// 			 // delete isn't the same as this this other we need to get the next frag from thi
				// 			 // second delete
				// 			 if(pfNewEnd != static_cast<pf_Frag *>(pOb))
				// 			 {
				// 			      bResult2 =
				// 				_deleteObjectWithNotify(posComrade,pOb,0,1,
				// 							pfsContainer2,0,0);
				// 			 }
				// 			 else
				// 			 {
				// 			      bResult2 =
				// 				_deleteObjectWithNotify(posComrade,pOb,0,1,
				// 							pfsContainer2,&pfNewEnd,&fragOffsetNewEnd);
				// 			 }

				// 			 // FIXME!! Need to work out how to delete the content of the annotation
				// 			 // at this point

				// 			 if(posComrade >= dpos1 && posComrade <= dpos1 + length - 2)
				// 			 {
				// 			      // the endmarker was inside of the segment we are working
				// 			      // so we have to adjust the length
				// 			      length--;
				// 			 }

				// 			 break;
				// 		     }
				// 		 }
				// 		 pF = pF->getNext();
				// 	     }
				// 	}
				// }
				break;

				case PTO_RDFAnchor:
                    bResult = _deleteComplexSpanHAR( pO,
                                                     dpos1,
                                                     dpos2,
                                                     length,
                                                     fragOffset_First,
                                                     lengthThisStep,
                                                     pfsContainer,
                                                     pfNewEnd,
                                                     fragOffsetNewEnd,
                                                     PT_XMLID );
                    break;
                    
				case PTO_Field:
				{
				}
				default:
					bResult
						= _deleteObjectWithNotify(dpos1,pO,fragOffset_First,lengthThisStep,
									  pfsContainer,&pfNewEnd,&fragOffsetNewEnd);

			}


			UT_return_val_if_fail (bResult, false);
		}
		break;

		case pf_Frag::PFT_FmtMark:
			// we already took care of these...
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			break;

		}

		// we do not change dpos1, since we are deleting stuff, but we
		// do decrement the length-remaining.
		// dpos2 becomes bogus at this point.

		length -= lengthThisStep;

		// since _delete{*}WithNotify(), can delete pf_First, mess with the
		// fragment list, and does some aggressive coalescing of
		// fragments, we cannot just do a pf_First->getNext() here.
		// to advance to the next fragment, we use the *NewEnd variables
		// that each of the _delete routines gave us.

		pf_First = pfNewEnd;
		if (!pf_First)
			length = 0;
		fragOffset_First = fragOffsetNewEnd;
	}

	if (pfsFirstBlock)
	{
		UT_DEBUGMSG(("Delete first block of a section\n"));
		bool bResult = _deleteStruxWithNotify(pfsFirstBlock->getPos(),pfsFirstBlock,
											  &pfNewEnd,&fragOffsetNewEnd);
		UT_return_val_if_fail (bResult, false);
	}

	return true;
}


bool pt_PieceTable::_deleteComplexSpan_norec(PT_DocPosition dpos1,
											 PT_DocPosition dpos2)
{
	pf_Frag * pfNewEnd = NULL;
	UT_uint32 fragOffsetNewEnd = 0;

	pf_Frag * pf_First;
	pf_Frag * pf_End;
	PT_BlockOffset fragOffset_First;
	PT_BlockOffset fragOffset_End;

	bool bFound = getFragsFromPositions(dpos1,dpos2,&pf_First,&fragOffset_First,&pf_End,&fragOffset_End);
	UT_return_val_if_fail (bFound, false);

	pf_Frag_Strux * pfsContainer = NULL;
	bool bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
	UT_return_val_if_fail (bFoundStrux, false);

	// loop to delete the amount requested, one text fragment at a time.
	// if we encounter any non-text fragments along the way, we delete
	// them too.  that is, we implicitly delete Strux and Objects here.

	UT_uint32 length = dpos2 - dpos1;
	while (length > 0)
	{
		UT_uint32 lengthInFrag = pf_First->getLength() - fragOffset_First;
		UT_uint32 lengthThisStep = UT_MIN(lengthInFrag, length);

		switch (pf_First->getType())
		{
		case pf_Frag::PFT_EndOfDoc:
		default:
			UT_ASSERT_HARMLESS(0);
			return false;

		case pf_Frag::PFT_Strux:
		{
			pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *> (pf_First);

			bool bResult = _deleteStruxWithNotify(dpos1,pfs,
									   &pfNewEnd,&fragOffsetNewEnd,
									   false);
			UT_return_val_if_fail (bResult, false);
			// we do not update pfsContainer because we just deleted pfs.
		}
		break;

		case pf_Frag::PFT_Text:
		{
			bool bResult = _deleteSpanWithNotify(dpos1,
									  static_cast<pf_Frag_Text *>(pf_First),
									  fragOffset_First,lengthThisStep,
									  pfsContainer,&pfNewEnd,
									  &fragOffsetNewEnd, false);
			UT_return_val_if_fail (bResult, false);
		}
		break;

		case pf_Frag::PFT_Object:
		{
			bool bResult = _deleteObjectWithNotify(dpos1,static_cast<pf_Frag_Object *>(pf_First),
									fragOffset_First,lengthThisStep,
									pfsContainer,&pfNewEnd,&fragOffsetNewEnd,
									false);
			UT_return_val_if_fail (bResult, false);
		}
		break;

		case pf_Frag::PFT_FmtMark:
			// we already took care of these...
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			break;

		}

		// we do not change dpos1, since we are deleting stuff, but we
		// do decrement the length-remaining.
		// dpos2 becomes bogus at this point.

		length -= lengthThisStep;

		// since _delete{*}WithNotify(), can delete pf_First, mess with the
		// fragment list, and does some aggressive coalescing of
		// fragments, we cannot just do a pf_First->getNext() here.
		// to advance to the next fragment, we use the *NewEnd variables
		// that each of the _delete routines gave us.

		pf_First = pfNewEnd;
		if (!pf_First)
			length = 0;
		fragOffset_First = fragOffsetNewEnd;
	}

	return true;
}



bool pt_PieceTable::_realDeleteSpan(PT_DocPosition dpos1,
									PT_DocPosition dpos2,
									PP_AttrProp *p_AttrProp_Before,
									bool bDeleteTableStruxes,
									bool bDontGlob)
{
    UT_DEBUGMSG(("_realDeleteSpan() dpos1:%d dpos2:%d\n", dpos1, dpos2 ));
	// remove (dpos2-dpos1) characters from the document at the given position.

	UT_return_val_if_fail (m_pts==PTS_Editing, false);
	UT_return_val_if_fail (dpos2 > dpos1, false);

	bool bSuccess = true;
	UT_Stack stDelayStruxDelete;

	PT_DocPosition old_dpos2 = dpos2;

	//  Before we begin the delete proper, we might want to adjust the ends
	//  of the delete slightly to account for expected behavior on
	//  structural boundaries.
	bSuccess = _tweakDeleteSpan(dpos1, dpos2, &stDelayStruxDelete);
	if (!bSuccess)
	{
		return false;
	}

	// Get the attribute properties before the delete.

	PP_AttrProp AttrProp_Before;

	{
		pf_Frag * pf1;
		PT_BlockOffset Offset1;
		getFragFromPosition(dpos1, &pf1, &Offset1);
		if(pf1->getType() == pf_Frag::PFT_Text)
		{
			const PP_AttrProp *p_AttrProp;
			getAttrProp(static_cast<pf_Frag_Text *>(pf1)->getIndexAP(), &p_AttrProp);
		  
			AttrProp_Before = *p_AttrProp;
			if(p_AttrProp_Before)
				*p_AttrProp_Before = *p_AttrProp;

			// we do not want to inherit revision attribute
			AttrProp_Before.setAttribute("revision", "");
		}
	}

	// The code used to only glob for the complex case. But when
	// there's a simple delete, we may still end up adding the
	// formatmark below (i.e., when deleting all the text in a
	// document), and thus creating a two-step undo for a perceived
	// one-step operation. See Bug FIXME
	if(!bDontGlob)
	{
		beginMultiStepGlob();
	}

	bool bIsSimple = _isSimpleDeleteSpan(dpos1, dpos2) && stDelayStruxDelete.getDepth() == 0;
	if (bIsSimple)
	{
		//  If the delete is sure to be within a fragment, we don't
		//  need to worry about much of the bookkeeping of a complex
		//  delete.
		bSuccess = _deleteComplexSpan(dpos1, dpos2, &stDelayStruxDelete);
	}
	else
	{
		//  If the delete spans multiple fragments, we need to
		//  be a bit more careful about deleting the formatting
		//  first, and then the actual spans.
		_changePointWithNotify(old_dpos2);

		UT_sint32 oldDepth = stDelayStruxDelete.getDepth();
		bSuccess = _deleteFormatting(dpos1, dpos2);
		if (bSuccess)
		{
			bSuccess = _deleteComplexSpan(dpos1, dpos2, &stDelayStruxDelete);
		}

		bool prevDepthReached = false;
		PT_DocPosition finalPos = dpos1;
		while (bSuccess && stDelayStruxDelete.getDepth() > 0)
		{
			pf_Frag_Strux * pfs;
			if(stDelayStruxDelete.getDepth() <= oldDepth)
			{
				prevDepthReached = true;
			}
			stDelayStruxDelete.pop(reinterpret_cast<void **>(&pfs));

 			pf_Frag *pf;
			PT_DocPosition dp;
			if(bDeleteTableStruxes || prevDepthReached )
			{
				if(!prevDepthReached)
				{				
					_deleteFormatting(dpos1 - pfs->getLength(), dpos1);
//
// FIXME this code should be removed if undo/redo on table manipulations
//       works fine.
#if 0
//					_deleteFormatting(myPos - pfs->getLength(), myPos);
//					bSuccess = _deleteStruxWithNotify(myPos - pfs->getLength(), pfs,
//													  &pf, &dp);
#endif

					PT_DocPosition myPos = pfs->getPos();
					bSuccess = _deleteStruxWithNotify(myPos, pfs, &pf, &dp);
				}
				else if(pfs->getPos() >= dpos1)
				{
					_deleteFormatting(dpos1 - pfs->getLength(), dpos1);
					bSuccess = _deleteStruxWithNotify(dpos1 - pfs->getLength(), pfs,
													  &pf, &dp);
				}
			}
			else
			{
				bSuccess = true;
				pf = pfs->getNext();
				dp = 0;
				dpos1 = dpos1 + pfs->getLength();
			}
		}

		_changePointWithNotify(finalPos);
	}

	// Have we deleted all the text in a paragraph.

	pf_Frag * p_frag_before;
	PT_BlockOffset Offset_before;
	getFragFromPosition(dpos1 - 1, &p_frag_before, &Offset_before);

	pf_Frag * p_frag_after;
	PT_BlockOffset Offset_after;
	getFragFromPosition(dpos1, &p_frag_after, &Offset_after);

	if(((p_frag_before->getType() == pf_Frag::PFT_Strux) ||
		(p_frag_before->getType() == pf_Frag::PFT_EndOfDoc)) &&
	   ((p_frag_after->getType() == pf_Frag::PFT_Strux) ||
		(p_frag_after->getType() == pf_Frag::PFT_EndOfDoc)))
	{
		xxx_UT_DEBUGMSG(("pt_PieceTable::deleteSpan Paragraph empty\n"));

		// All text in paragraph is deleted so insert a text format
		// Unless we've deleted all text in a footnote type structure.
		// or if bDontGlob is true.
		// If we insert an FmtMark is an empty footnote it
		// will appear in the enclosing block and
		// screw up the run list.
		//
		bool bDoit = !bDontGlob;
		if(bDoit && (p_frag_after->getType() == pf_Frag::PFT_Strux))
		{
		     pf_Frag_Strux * pfsa = static_cast<pf_Frag_Strux *>(p_frag_after);
		     if(isEndFootnote(pfsa))
		     {
			 bDoit = false;
		     }
		}
		pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(p_frag_before);
		if(bDoit && ((pfs->getStruxType() == PTX_Block) || (p_frag_before->getType() == pf_Frag::PFT_EndOfDoc) ))
			_insertFmtMarkFragWithNotify(PTC_AddFmt, dpos1, &AttrProp_Before);

	}

	// End the glob after (maybe) having inserted the FmtMark
	if (!bDontGlob)
	{
		endMultiStepGlob();
	}

	return bSuccess;
}

// need a special delete for a field update because otherwise
// the whole field object would be deleted by _tweakDeleteSpan
bool pt_PieceTable::deleteFieldFrag(pf_Frag * pf)
{


	UT_return_val_if_fail (m_pts==PTS_Editing,false);

	bool bSuccess = true;
	UT_Stack stDelayStruxDelete;

	PT_DocPosition dpos1 = getFragPosition(pf);
	UT_return_val_if_fail (dpos1,false);
	PT_DocPosition dpos2 = dpos1 + pf->getLength();


	//  If the delete is sure to be within a fragment, we don't
	//  need to worry about much of the bookkeeping of a complex
	//  delete.
	bSuccess = _deleteComplexSpan_norec(dpos1, dpos2);
	return bSuccess;
}

void pt_PieceTable::_tweakFieldSpan(PT_DocPosition & dpos1,
                                    PT_DocPosition & dpos2) const
{
	if(m_bDoNotTweakPosition)
		return;
	
	//  Our job is to adjust the end positions of the delete
	//  operating to delete those structural object that the
	//  user will expect to have deleted, even if the dpositions
	//  aren't quite right to encompass those.

	pf_Frag * pf_First;
	pf_Frag * pf_End;
	pf_Frag * pf_Other;
	PT_BlockOffset fragOffset_First;
	PT_BlockOffset fragOffset_End;

	bool bFound = getFragsFromPositions(dpos1,dpos2,&pf_First,&fragOffset_First,&pf_End,&fragOffset_End);
	UT_return_if_fail (bFound);

	pf_Frag_Strux * pfsContainer = NULL;
	bool bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
	UT_return_if_fail (bFoundStrux);

    // if start in middle of field widen to include object
    if ((pf_First->getType() == pf_Frag::PFT_Text)&&
        (static_cast<pf_Frag_Text *>(pf_First)->getField()))
    {
        pf_Frag_Text * pft = static_cast<pf_Frag_Text *>(pf_First);
        pf_Frag_Text * pft2 = NULL;
        // we can't delete part of a field so widen deletion to
        // include object at start
        while (pft->getPrev()->getType() == pf_Frag::PFT_Text)
        {
            pft2 = static_cast<pf_Frag_Text *>(pft->getPrev());
            UT_ASSERT_HARMLESS(pft->getField() == pft2->getField());
            pft = pft2;
        }
        UT_return_if_fail (pft->getPrev()->getType() == pf_Frag::PFT_Object);
        pf_Frag_Object *pfo =
            static_cast<pf_Frag_Object *>(pft->getPrev());
        UT_return_if_fail (pfo->getObjectType()==PTO_Field);
        UT_return_if_fail (pfo->getField()==pft->getField());
        dpos1 = getFragPosition(pfo);
    }
    // if end in middle of field widen to include whole Frag_Text
    if (((pf_End->getType() == pf_Frag::PFT_Text)&&
         (pf_End)->getField())/*||
								((pf_End->getType() == pf_Frag::PFT_Object
								)&&
								(static_cast<pf_Frag_Object *>(pf_End)
								->getObjectType()==PTO_Field))*/)
    {
        fd_Field * pField = pf_End->getField();
        UT_return_if_fail (pField);
        pf_Other = pf_End->getNext();
        UT_return_if_fail (pf_Other);
        while (pf_Other->getField()==pField)
        {
            pf_Other = pf_Other->getNext();
            UT_return_if_fail (pf_Other);
        }
        dpos2 = getFragPosition(pf_Other);
    }
}
