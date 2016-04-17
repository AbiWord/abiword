/* AbiSource
 * Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include <string>

// abiword stuff
#include "fd_Field.h"
#include "fl_AutoNum.h"
#include "fp_PageSize.h"

#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"

#include "ut_misc.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_bytebuf.h"
#include "ut_hash.h"
#include "ut_vector.h"
#include "ut_stack.h"
#include "ut_assert.h"

#include <gsf/gsf-output.h>

class IE_Exp_OpenWriter : public IE_Exp
{
public:
  IE_Exp_OpenWriter(PD_Document * pDocument);
  virtual ~IE_Exp_OpenWriter();

protected:
  virtual UT_Error	_writeDocument(void);

private:
  GsfOutfile * m_oo;
};

/*****************************************************************************/
/*****************************************************************************/

class OO_ListenerImpl
{
public:
   OO_ListenerImpl() {}
   virtual ~OO_ListenerImpl() {}
   virtual void insertText(const UT_UCSChar * data, UT_uint32 length) = 0;
   virtual void openBlock(const std::string & styleAtts, const std::string & styleProps,
                          const std::string & font, bool bIsHeading = false) = 0;
   virtual void closeBlock() = 0;
   virtual void openSpan(const std::string & props, const std::string & font) = 0;
   virtual void closeSpan() = 0;
   virtual void openHyperlink(const PP_AttrProp* pAP) = 0;
   virtual void closeHyperlink() = 0;
};

class OO_Listener : public PL_Listener
{
public:
   OO_Listener(PD_Document * pDocument, IE_Exp_OpenWriter * pie, OO_ListenerImpl *pListenerImpl);

   virtual bool populate(fl_ContainerLayout* sfh, const PX_ChangeRecord * pcr);
   virtual bool populateStrux(pf_Frag_Strux* sdh, const PX_ChangeRecord * pcr, fl_ContainerLayout* * psfh);
   virtual bool change(fl_ContainerLayout* sfh, const PX_ChangeRecord * pcr);
   virtual bool insertStrux(fl_ContainerLayout* sfh,
			    const PX_ChangeRecord * pcr,
			    pf_Frag_Strux* sdh,
			    PL_ListenerId lid,
			    void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
						    PL_ListenerId lid,
						    fl_ContainerLayout* sfhNew));
   virtual bool signal(UT_uint32 iSignal);

   void endDocument();

private:
   void _openSpan(PT_AttrPropIndex api);
   void _closeSpan();
   void _openBlock(PT_AttrPropIndex apiSpan);
   void _closeBlock();
   void _openHyperlink(const PP_AttrProp* pAP);
   void _closeHyperlink();

   PD_Document * m_pDocument;
   IE_Exp_OpenWriter * m_pie;
   OO_ListenerImpl * m_pListenerImpl;

   bool m_bInBlock;
   bool m_bInSpan;
   bool m_bInHyperlink;
};

class OO_StylesContainer
{
public:
   OO_StylesContainer() {}
   ~OO_StylesContainer() {
	   m_spanStylesHash.purgeData();
	   m_blockAttsHash.purgeData();
	   m_fontsHash.purgeData();
   }
   void addSpanStyle(const std::string &key);
   void addBlockStyle(const std::string & styleAtts, const std::string & styleProps);
   void addFont(const std::string & font);
   int getSpanStyleNum(const std::string &key) const;
   int getBlockStyleNum(const std::string & styleAtts, const std::string & styleProps) const;
   UT_GenericVector<int*> * enumerateSpanStyles() const;
   UT_String * pickBlockAtts(const UT_String *key);
   UT_GenericVector<const UT_String*> * getSpanStylesKeys() const;
   UT_GenericVector<const UT_String*> * getBlockStylesKeys() const;
   UT_GenericVector<const UT_String*> * getFontsKeys() const;
private:
   UT_GenericStringMap<int*> m_spanStylesHash;
   UT_GenericStringMap<UT_String*> m_blockAttsHash;
   UT_GenericStringMap<int*> m_fontsHash;
};

/*!
 * OO_AccumulatorImpl: This class collects style definitions from the document,
 * gathering information from the listener which references it. It does not
 * actually write anything, merely storing
 */
class OO_AccumulatorImpl : public OO_ListenerImpl
{
public:
   OO_AccumulatorImpl(OO_StylesContainer *pStylesContainer) : OO_ListenerImpl() { m_pStylesContainer = pStylesContainer; }
   virtual void insertText(const UT_UCSChar * /*data*/, UT_uint32 /*length*/) {}
   virtual void openBlock(const std::string & styleAtts, const std::string & styleProps,
                          const std::string & font, bool bIsHeading = false);
   virtual void closeBlock() {};
   virtual void openSpan(const std::string & props, const std::string & font);
   virtual void closeSpan() {}
   virtual void openHyperlink(const PP_AttrProp* /*pAP*/) {}
   virtual void closeHyperlink() {}

private:
   OO_StylesContainer *m_pStylesContainer;
};

/*!
 * OO_WriterImpl: This class writes out the content which reference the style
 * definitions which we collected earlier with the accumulator.
 */
class OO_WriterImpl : public OO_ListenerImpl
{
public:
   OO_WriterImpl(GsfOutfile *pOutfile, OO_StylesContainer *pStylesContainer);
   ~OO_WriterImpl();
   virtual void insertText(const UT_UCSChar * data, UT_uint32 length);
   virtual void openBlock(const std::string & styleAtts, const std::string & styleProps,
                          const std::string & font, bool bIsHeading = false);
   virtual void closeBlock();
   virtual void openSpan(const std::string & props, const std::string & font);
   virtual void closeSpan();
   virtual void openHyperlink(const PP_AttrProp* pAP);
   virtual void closeHyperlink();

private:
   GsfOutput * m_pContentStream;
   OO_StylesContainer *m_pStylesContainer;
   UT_UTF8String m_blockEnd;
};

/*!
 * OO_StylesWriter: This class writes the "styles.xml" portion
 * of the .sxw file. It can also be used to convert Abi's attributes and
 * properties.
 */
class OO_StylesWriter
{
public:
   /*!
    * Write the styles stream.
    */
   static bool writeStyles(PD_Document * pDoc, GsfOutfile * oo, OO_StylesContainer & stylesContainer);
   /*
    * Add <office:font-decls> section to the buffer
    */
   static void addFontDecls(UT_UTF8String & buffer, OO_StylesContainer & stylesContainer);
   /*!
    * Convert attributes and properties.
    */
   static void map(const PP_AttrProp * pAP, UT_UTF8String & styleAtts, UT_UTF8String & propAtts, UT_UTF8String & font);
private:
  OO_StylesWriter ();
};
