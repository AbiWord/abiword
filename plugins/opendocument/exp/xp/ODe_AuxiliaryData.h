/* AbiSource
 * 
 * Copyright (C) 2005 INdT
 * Author: Daniel d'Andrada T. de Carvalho <daniel.carvalho@indt.org.br>
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

#ifndef ODE_AUXILIARYDATA_H_
#define ODE_AUXILIARYDATA_H_

// AbiWord includes
#include <ut_vector.h>
#include <ut_string_class.h>
#include <gsf/gsf-output-memory.h>

#include "pt_Types.h"
#include <map>
#include <list>
class PP_AttrProp;
class PP_RevisionAttr;

/**
 * All paragraph styles used to define the chapter levels of a document are
 * called heading styles. Paragraphs that use heading styles are the ones that
 * appear in a table of contents.
 * 
 * This class stores the name and the respective outline level of all those
 * styles. It's necessary to do this because, in an OpenDocument document,
 * a standard paragraph is <text:p [...]>, but a heading paragraph is a
 * <text:h text:outline-level="x" [...]>
 * 
 * So, when translating an AbiWord paragraph, we must know wheter it will map
 * into an OpenDocument <text:p> or into a <text:h>.
 */
class ODe_HeadingStyles {
public:

    virtual ~ODe_HeadingStyles();

    /**
     * Given a paragraph style name, this method returns its outline level.
     * 0 (zero) is returned it the style name is not used by heading paragraphs.
     */
    UT_uint8 getHeadingOutlineLevel(const UT_UTF8String& rStyleName) const;
    
    void addStyleName(const gchar* pStyleName, UT_uint8 outlineLevel);
    
private:
    UT_GenericVector<UT_UTF8String*> m_styleNames;
    UT_GenericVector<UT_uint8> m_outlineLevels;
};

/**
 * In ABW land every paragraph contains spans represented inside <c> tags.
 * Each <c> tag might have an old and new change revision and some formatting
 * information or be simply intro,-last in which case the span was deleted
 * in the "last" revision.
 * 
 *   <section xid="1" props="...">
 *    <p style="Normal" xid="2">
 *     <c revision="1,-4">This paragraph </c>
 *     <c revision="3,-4">in the </c>
 *     <c revision="3{font-weight:bold}{author:0},-4">middle</c>
 *     <c revision="3,-4">, </c>
 *     <c revision="1,-4">was inserted.</c>
 *     <c revision="2,-4"> Something more.</c>
 *   </p>
 *
 * This class is setup by the ODe_ChangeTrackingParagraph_Listener
 * and uses ODe_ChangeTrackingParagraph_Data to store the information
 * for each paragraph, keyed of the attributeProperties for the para.
 * 
 * This listener collects information about these revisions so that when
 * writing to odt+ct we can use <text:p delta:insertion-type...> to
 * introduce a paragraph and <delta:removed-content...> to remove a paragraph
 * which contains only <c> tags with the same -last version (ie, paragraph
 * was completely deleted in revision "last").
 *
 */
class ODe_ChangeTrackingParagraph_Data
{
    UT_uint32 m_lastSpanVersion;
    
  public:
    UT_uint32 m_minRevision;  //< lowest revision appearing in any <c> tag
    UT_uint32 m_maxRevision;  //< highest revision appearing in any <c> tag
    UT_uint32 m_maxDeletedRevision; //< lowest -revision appearing in any <c> tag
    bool      m_allSpansAreSameVersion; //< all <c> tags are the same revision
    ODe_ChangeTrackingParagraph_Data()
        : m_minRevision(-1)
        , m_maxRevision(0)
        , m_maxDeletedRevision(0)
        , m_allSpansAreSameVersion(true)
        , m_lastSpanVersion(-1)
    {
    }
    void update( const PP_RevisionAttr* ra );
    bool isParagraphDeleted();
    UT_uint32 getVersionWhichRemovesParagraph();
    UT_uint32 getVersionWhichIntroducesParagraph();

        
};

/**
 * A cache is built up with a parse of the document structure in order to
 * calculate min, max change versions and other information which might be handy
 * to know before an element is seen in normal pass order
 *
 * This class allows data to be collected for a document range begin - end.
 * It is mainly a holder for a DataPayloadClass object which is associated
 * with a document range.
 */
template < typename DataPayloadClass >
class ODe_ChangeTrackingScopedData
{
    PT_DocPosition   m_begin;
    PT_DocPosition   m_end;
    DataPayloadClass m_data;

  public:

    ODe_ChangeTrackingScopedData( PT_DocPosition  begin,
                                 PT_DocPosition  end,
                                 DataPayloadClass data = DataPayloadClass() )
        : m_begin( begin )
        , m_end( end )
        , m_data( data )
    {
    }
    
    bool contains( PT_DocPosition pos )
    {
        bool ret = false;
        if( m_begin <= pos && m_end > pos )
            return true;
        return ret;
    }
    PT_DocPosition getBeginPosition()
    {
        return m_begin;
    }
    PT_DocPosition getEndPosition()
    {
        return m_end;
    }
    void setBeginPosition(PT_DocPosition pos)
    {
        m_begin = pos;
    }
    void setEndPosition(PT_DocPosition pos)
    {
        m_end = pos;
    }
    DataPayloadClass& getData()
    {
        return m_data;
    }
};

typedef ODe_ChangeTrackingScopedData< ODe_ChangeTrackingParagraph_Data >   ChangeTrackingParagraphData_t;
typedef ODe_ChangeTrackingScopedData< ODe_ChangeTrackingParagraph_Data >* pChangeTrackingParagraphData_t;


/**
 * Auxiliary data used and shared by all listener implementations.
 */
class ODe_AuxiliaryData {
public:
    ODe_AuxiliaryData();
    ~ODe_AuxiliaryData();

    ODe_HeadingStyles m_headingStyles;
    
    // Content of the TOC
    // Note: we only support 1 TOC body per document right now. It's wasted
    // effort try to manually build up multiple different TOC bodies,
    // until we can get to the actual TOC data that AbiWord generates.
    GsfOutput* m_pTOCContents;

    // The destination TOC style names for all levels
    std::map<UT_sint32, UT_UTF8String> m_mDestStyles;

    // The number of tables already added to the document.
    UT_uint32 m_tableCount;
    
    // The number of frames already added to the document.
    UT_uint32 m_frameCount;
    
    // The number of notes (footnotes and endnotes) already added to the document.
    UT_uint32 m_noteCount;

    /////////////////////////////////
    // ODT Change Tracking Support //
    /////////////////////////////////

    // Lookahead information for each <p> tag
    
    typedef std::list< pChangeTrackingParagraphData_t > m_ChangeTrackingParagraphs_t;
    m_ChangeTrackingParagraphs_t m_ChangeTrackingParagraphs;

    pChangeTrackingParagraphData_t getChangeTrackingParagraphData( PT_DocPosition pos );
    pChangeTrackingParagraphData_t ensureChangeTrackingParagraphData( PT_DocPosition pos );
    void deleteChangeTrackingParagraphData();
    void dumpChangeTrackingParagraphData();
     
};

#endif /*ODE_AUXILIARYDATA_H_*/
