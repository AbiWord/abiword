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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef PL_LISTENERCOUPLECLOSER_H
#define PL_LISTENERCOUPLECLOSER_H

#include "ut_types.h"
#include "pt_Types.h"
class PX_ChangeRecord;
#include "pl_Listener.h"

#include <list>
#include <string>

class fl_ContainerLayout;

/**
 * A subclass of Listener which includes default implementations of
 * some methods we dont care about and an isFinished() method to tell
 * the caller that it doesn't need to keep walking the document
 * anymore.
 */
struct ABI_EXPORT PL_FinishingListener : public PL_Listener
{
    virtual bool isFinished() = 0;

    virtual bool		change(fl_ContainerLayout* /*sfh*/,
                               const PX_ChangeRecord * /*pcr*/)
    {return true;}
    virtual bool		insertStrux(fl_ContainerLayout* /*sfh*/,
                                    const PX_ChangeRecord * /*pcr*/,
                                    pf_Frag_Strux* /*sdhNew*/,
                                    PL_ListenerId /*lid*/,
                                    void (* /*pfnBindHandles*/)(pf_Frag_Strux* sdhNew,
                                                                PL_ListenerId lid,
                                                                fl_ContainerLayout* sfhNew))
    { return true;}
    virtual bool		signal(UT_uint32 /*iSignal*/)
    { return true;}
};


/**
 * When copying a selection from a document, one might have the case
 * where there are matched pairs of markers in the document. When only
 * one of the pair of markers is in the selection to copy, a simple
 * copy using a listener on just that range will likely output an
 * invalid document.
 *
 * Consider the document below with the selection highlighted below
 * it. If one attaches a pl_listener and calls
 * PD_Document::tellListenerSubset() they will only see the bm-start
 * tag. To generate a valid document, a listener would have to
 * remember which pair tags are still open and close them in an
 * appropriate order.
 *
 * This is some text <bm-start>I can jump here<bm-end>
 *             ^-------------------------^
 *
 * The closer class maintains a reference to the document and a
 * delegate listener. The delegate listener will see all the
 * populate() calls for the given document range, and then the closer
 * will call populate() on the delegate for tags which are open in the
 * range but not closed in the range. And the converse, where a tag is
 * closed in the range but opened before the start of the range.
 *
 * For the above document fragment, the delegate will the following
 * because they are in the selected document range:
 * " text "
 * <bm-start>
 * "I can jump"
 *
 * From there the closer will find the matching <bm-end> and call
 * delegate->populate(). So the delegate will see something like the
 * following as the selection and have matched start and end bookmark
 * tags.
 *
 * text <bm-start>I can jump<bm-end>
 *
 * Having this functionality in a separate closer class allows for
 * extension by inheritance, and allows tellListenerSubset() to handle
 * cases that one might not consider right off the bat, such as the
 * following where the missing matching tag is actually before the
 * selected range. In this case the closer will emit the required
 * matching populate(bm-start) before the selected document content.
 * Handling open tags before the range is a bit tricky because the call
 * to delegate->populate() must be in document order and not the reverse
 * order that might be used to step backwards from the desired range.
 * This scenario is handled for you with the PL_ListenerCoupleCloser
 * and pt_PieceTable::tellListenerSubset(): The delegate listener
 * only ever sees items once, and in document ordering.
 *
 * Some text <bm-start>I can jump here<bm-end> and then more waffle
 *                          ^-------------------------^
 *
 * While all range cases might not be covered right now, this class is
 * a good foundation for adding new cases as they are found such that
 * all listeners can benefit from proper closed ranges.
 *
 * This class can be passed as the "closer" parameter to
 * PD_Document::tellListenerSubset() and maintains the open/closed
 * state of matched tags. Once the document range has been visited
 * PD_Document::tellListenerSubset() calls methods like
 * populateClose() in the closer which in turn might call populate()
 * on the delegate listener.
 *
 */
class ABI_EXPORT PL_ListenerCoupleCloser : public PL_Listener
{
  protected:
    PD_Document* m_pDocument;
    PL_Listener* m_delegate;
    typedef std::list< std::string > stringlist_t;
    stringlist_t m_rdfUnclosedAnchorStack;
    stringlist_t m_rdfUnopenedAnchorStack;
    stringlist_t m_bookmarkUnclosedStack;
    stringlist_t m_bookmarkUnopenedStack;

    bool shouldClose( const std::string& id, bool isEnd, stringlist_t& sl );
    bool shouldOpen( const std::string& id,  bool isEnd, stringlist_t& sl );
    void trackOpenClose( const std::string& id, bool isEnd,
                         stringlist_t& unclosed, stringlist_t& unopened );


    struct ABI_EXPORT AfterContentListener : public PL_FinishingListener
    {
        PL_ListenerCoupleCloser* m_self;
        AfterContentListener( PL_ListenerCoupleCloser* self )
            : m_self(self)
        {}

        virtual bool isFinished();
        virtual bool populate( fl_ContainerLayout* sfh,
                               const PX_ChangeRecord * pcr );
        virtual bool populateStrux( pf_Frag_Strux* sdh,
                                    const PX_ChangeRecord * pcr,
                                    fl_ContainerLayout* * psfh);
    };
    AfterContentListener m_AfterContentListener;

    struct ABI_EXPORT BeforeContentListener : public PL_FinishingListener
    {
        PL_ListenerCoupleCloser* m_self;
        BeforeContentListener( PL_ListenerCoupleCloser* self )
            : m_self(self)
        {}

        virtual bool isFinished();
        virtual bool populate( fl_ContainerLayout* sfh,
                               const PX_ChangeRecord * pcr );
        virtual bool populateStrux( pf_Frag_Strux* sdh,
                                    const PX_ChangeRecord * pcr,
                                    fl_ContainerLayout* * psfh);
    };
    BeforeContentListener m_BeforeContentListener;

    struct ABI_EXPORT NullContentListener : public PL_FinishingListener
    {
        PL_ListenerCoupleCloser* m_self;
        NullContentListener( PL_ListenerCoupleCloser* self )
            : m_self(self)
        {}

        virtual bool isFinished()
        {
            return true;
        }
        virtual bool populate( fl_ContainerLayout*,
                               const PX_ChangeRecord* )
        {
            return false;
        }
        virtual bool populateStrux( pf_Frag_Strux*,
                                    const PX_ChangeRecord*,
                                    fl_ContainerLayout**)
        {
            return false;
        }
    };
    NullContentListener m_NullContentListener;


  public:
    PL_ListenerCoupleCloser();
    virtual ~PL_ListenerCoupleCloser();
    void setDelegate( PL_Listener* delegate );
	PD_Document*  getDocument(void);
	void          setDocument(PD_Document * pDoc);
    void          reset();

    virtual bool populate( fl_ContainerLayout* sfh,
                           const PX_ChangeRecord * pcr );
    virtual bool populateStrux( pf_Frag_Strux* sdh,
                                const PX_ChangeRecord * pcr,
                                fl_ContainerLayout* * psfh);

    PL_FinishingListener* getAfterContentListener();
    PL_FinishingListener* getBeforeContentListener();
    PL_FinishingListener* getNullContentListener();


    virtual bool		change(fl_ContainerLayout* /*sfh*/,
                               const PX_ChangeRecord * /*pcr*/)
    {return true;}

    virtual bool		insertStrux(fl_ContainerLayout* /*sfh*/,
                                    const PX_ChangeRecord * /*pcr*/,
                                    pf_Frag_Strux* /*sdhNew*/,
                                    PL_ListenerId /*lid*/,
                                    void (* /*pfnBindHandles*/)(pf_Frag_Strux* sdhNew,
                                                                PL_ListenerId lid,
                                                                fl_ContainerLayout* sfhNew))
    { return true;}

	virtual bool		signal(UT_uint32 /*iSignal*/)
    { return true;}

  private:

    virtual bool populateAfter( fl_ContainerLayout* sfh,
                                const PX_ChangeRecord * pcr );
    virtual bool populateStruxAfter( pf_Frag_Strux* sdh,
                                     const PX_ChangeRecord * pcr,
                                     fl_ContainerLayout* * psfh);


    virtual bool populateBefore( fl_ContainerLayout* sfh,
                                 const PX_ChangeRecord * pcr );
    virtual bool populateStruxBefore( pf_Frag_Strux* sdh,
                                      const PX_ChangeRecord * pcr,
                                      fl_ContainerLayout* * psfh);



};

#endif
