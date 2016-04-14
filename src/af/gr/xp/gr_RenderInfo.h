/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2004 Tomas Frydrych
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

#ifndef GR_RENDERINFO_H
#define GR_RENDERINFO_H

#include "ut_types.h"
#include "ut_vector.h"
#include "gr_Graphics.h"

class UT_TextIterator;

/**
   Identifies scripts for the shaping engine; the actual values are
   shaper specific and of no consequence to the xp layer; the only
   values that are defined are:

   Undefined - identifies text that requires no special processing.
   Void      - indicates the record does not represent real item;
   only used with the last (dummy) record in GR_Itemization
*/
enum GR_ScriptType
{
	GRScriptType_Undefined = 0,
	GRScriptType_Void = 0xffffffff
};

/** used to identify GR_RenderInfo and related classes
	add as required */
enum GRRI_Type
{
	GRRI_XP = 0,
	GRRI_WIN32,
	GRRI_UNIX,
	GRRI_COCOA,
	GRRI_WIN32_UNISCRIBE,
	GRRI_CAIRO_PANGO,

	GRRI_BUILT_IN_LAST = 0x0000ffff,

	GRRI_UNKNOWN = 0xffffffff
};


/**
   This is an abstract class that describes an item of text and is passed
   to the shaper. Each platform needs to implement a derived class that
   would hold item information required by the specific shaper.

   \note constructor should be protected; new instances can only be created via
   GR_Graphics::newItem()
*/
class ABI_EXPORT GR_Item
{
  public:
	virtual ~GR_Item(){};
	virtual GR_ScriptType getType() const = 0;
	virtual GR_Item * makeCopy() const = 0; // make a copy of this item
	virtual GRRI_Type getClassId() const = 0;

  protected:
	GR_Item(){};
};


class ABI_EXPORT GR_XPItem : public GR_Item
{
	friend class GR_Graphics;

  public:
	virtual ~GR_XPItem(){};

	virtual GR_ScriptType getType() const {return m_eType;}
	virtual GR_Item * makeCopy() const {return new GR_XPItem(m_eType);}
	virtual GRRI_Type getClassId() const {return GRRI_XP;}

  protected:
	GR_XPItem():
	   m_eType(GRScriptType_Undefined){};
	GR_XPItem(GR_ScriptType t):
	   m_eType(t){};

	GR_ScriptType m_eType;
};


/**
   describes itemization of text

   offset is where an item starts
   type   is GR_ScriptType for the item

   length of item is calculated as difference of offsets between
   neighbouring items

   there is always to be one dummy item of type GRScriptType_Void
   after the last item with offset set so as to alow to calculate
   length of the last valid item

   getItemCount() returns the count of all items, including the dummy
   GRScriptType_Void item.
*/
class ABI_EXPORT GR_Itemization
{
  public:
	GR_Itemization():
	    m_iEmbedingLevel(0),
		m_iDirOverride(0),
		m_bShowControlChars(false),
	    m_pLang(NULL),
	    m_pFont(NULL)
	{};

	virtual ~GR_Itemization() {clear();} // do not delete the actual
										 // items, they get passed on
										 // to the runs

	UT_sint32       getItemCount() const {return m_vOffsets.getItemCount();}
	UT_sint32       getNthOffset(UT_sint32 i) const {return m_vOffsets.getNthItem(i);}
	GR_ScriptType   getNthType(UT_sint32 i) const
	                   {return m_vItems.getNthItem(i)->getType();}

	UT_uint32       getNthLength(UT_sint32 i)
	                   {
						  UT_return_val_if_fail(i < m_vOffsets.getItemCount()-1, 0);
						  return m_vOffsets.getNthItem(i+1) - m_vOffsets.getNthItem(i);
					   }

	GR_Item *       getNthItem(UT_sint32 i) const {return m_vItems.getNthItem(i);}

	void            addItem(UT_sint32 offset, GR_Item *item)
	                    { m_vOffsets.addItem(offset); m_vItems.addItem(item);}

	void            insertItem(UT_sint32 indx, UT_sint32 offset, GR_Item *item)
	                    { m_vOffsets.insertItemAt(offset, indx); m_vItems.insertItemAt(item,indx);}

	void            clear();

	void            setEmbedingLevel(UT_uint32 l) {m_iEmbedingLevel = l;}
	UT_uint32       getEmbedingLevel() const {return m_iEmbedingLevel;}

	void            setDirOverride(UT_BidiCharType o) {m_iDirOverride = o;}
	UT_BidiCharType getDirOverride() const {return m_iDirOverride;}

	void            setShowControlChars(bool s) {m_bShowControlChars = s;}
	bool            getShowControlChars() const {return m_bShowControlChars;}

	void            setLang(const char * l) {m_pLang = l;}
	const char *    getLang()const {return m_pLang;}

	void            setFont(const GR_Font * pFont) {m_pFont = pFont;}
	const GR_Font * getFont()const {return m_pFont;}

  private:
	UT_NumberVector m_vOffsets;
	UT_GenericVector<GR_Item*>  m_vItems;

	UT_uint32       m_iEmbedingLevel;
	UT_BidiCharType m_iDirOverride;
	bool            m_bShowControlChars;
	const char *    m_pLang;
	const GR_Font * m_pFont;
};

/**
   encapsulates output of GR_Graphics::shape() which is passed as
   input to GR_Graphics::renderChars().

   This is abstract class and suitable functionality is to be provided
   by platform code.

   <b>Notes on append(), split() and cut()</b>

   These functions allow our fp_TextRun to merge with next or to split
   into two without having to know about various platform dependent
   chaches that speed up shaping and drawing; append()
   joins all such chaches, while split() splits them into two. Before
   spliting, split() must allocate appropriate GR_FERenderInfo into pri.

   cut() attempts to remove a section of length iLen starting at
   offset without re-shaping the whole buffer; if it succeeds it
   returns true; if it fails and the chaches need to be re-calculated
   it return false (it should not carry out the reshaping per se)

   m_iOffset and m_iLength contain offset and length pertinent to the
   current operation and their state is undefined: the user should
   always set them if the function to which GR_RenderInfo is passed is
   going to use them
*/
class ABI_EXPORT GR_RenderInfo
{
  public:
	GR_RenderInfo(GR_ScriptType type)
		: m_iOffset(0), m_iLength(0), m_eShapingResult(GRSR_Unknown),
 		  m_eState(GRSR_Unknown), m_eScriptType(type),
		  m_pText(NULL), m_iVisDir(UT_BIDI_LTR),
	      m_xoff(0), m_yoff(),
	      m_pGraphics(NULL), m_pFont(NULL),
		  m_iJustificationPoints(0),
		  m_iJustificationAmount(0),
		  m_bLastOnLine(false),
		  m_pItem(NULL),
		  m_bInvalidateFontCache(false){};


	virtual ~GR_RenderInfo(){};

	virtual GRRI_Type getType() const = 0;

	virtual bool append(GR_RenderInfo &ri, bool bReverse = false) = 0;
	virtual bool split (GR_RenderInfo *&pri, bool bReverse = false) = 0;
	virtual bool cut(UT_uint32 offset, UT_uint32 len, bool bReverse = false) = 0;

	virtual bool canAppend(GR_RenderInfo &ri) const
	              {return (m_eScriptType == ri.m_eScriptType);}

	virtual bool isJustified() const = 0;


	UT_sint32           m_iOffset;
	UT_sint32           m_iLength;
	GRShapingResult     m_eShapingResult;
	GRShapingResult     m_eState;
	GR_ScriptType       m_eScriptType;
	UT_TextIterator *   m_pText;
	UT_BidiCharType     m_iVisDir;
	UT_sint32           m_xoff;
	UT_sint32           m_yoff;

	const GR_Graphics * m_pGraphics;
	const GR_Font     * m_pFont;

	UT_sint32           m_iJustificationPoints;
	UT_sint32           m_iJustificationAmount;
	bool                m_bLastOnLine;
	const GR_Item *     m_pItem;
	bool                m_bInvalidateFontCache;

 private:
	/** never to be implemented (constructor always has to set
		m_eScriptType to actual value)*/
	GR_RenderInfo() {};
};

/**
   This is an xp implementation of GR_RenderInfo for use with the
   built in UT_contextGlyph class.
*/
class ABI_EXPORT GR_XPRenderInfo : public GR_RenderInfo
{
  public:
	GR_XPRenderInfo(GR_ScriptType type);

	virtual ~GR_XPRenderInfo();

	virtual GRRI_Type getType() const {return GRRI_XP;}

	virtual bool append(GR_RenderInfo &ri, bool bReverse = false);
	virtual bool split (GR_RenderInfo *&pri, bool bReverse = false);
	virtual bool cut(UT_uint32 offset, UT_uint32 len, bool bReverse = false);

	virtual bool isJustified() const {return (m_iJustificationPoints != 0);}

	void prepareToRenderChars();


	UT_UCS4Char *       m_pChars;
	UT_sint32 *         m_pWidths;
	UT_sint32           m_iBufferSize;
	UT_sint32 *         m_pSegmentOffset;
	UT_sint32           m_iSegmentCount;
	UT_sint32           m_iSpaceWidthBeforeJustification; // <0 for not justified
	UT_uint32           m_iTotalLength;

	// these can be static as for now we do not want to chache anything
	static UT_sint32	    s_iClassInstanceCount;
	static UT_UCS4Char *    s_pCharBuff;
	static UT_sint32 *      s_pWidthBuff;
	static UT_sint32        s_iBuffSize;
	static UT_sint32 *      s_pAdvances;
	static GR_RenderInfo *  s_pOwner;
  private:
	void                   _constructorCommonCode();
	inline void            _stripLigaturePlaceHolders();
	inline void            _calculateCharAdvances();
	inline bool            _checkAndFixStaticBuffers();

};


/**
   Encapsulates input to GR_Graphics::shape()
*/
class ABI_EXPORT GR_ShapingInfo
{
  public:
	enum TextTransform {
		NONE = 0,
		CAPITALIZE,
		UPPERCASE,
		LOWERCASE
	};

	GR_ShapingInfo(UT_TextIterator & text, UT_uint32 iLen,
				   const char * pLang,
				   UT_BidiCharType iVisDir,
				   GRShapingResult eShapingRequired,
				   const GR_Font * pFont,
				   const GR_Item * pItem,
				   TextTransform textTransform = NONE,
				   bool previousWasSpace = false)
		:m_Text(text), m_iLength(iLen), m_pLang(pLang), m_iVisDir(iVisDir),
		 m_eShapingRequired(eShapingRequired),
	     m_pFont(pFont),
		m_iJustifyBy(0),
		m_pItem(pItem),
		m_TextTransform(textTransform),
		m_previousWasSpace(previousWasSpace)
	{
	}

	virtual ~GR_ShapingInfo() {};

	UT_TextIterator &   m_Text;
	UT_sint32           m_iLength;
	const char *        m_pLang;
	UT_BidiCharType     m_iVisDir;
	GRShapingResult     m_eShapingRequired;

	const GR_Font *     m_pFont;

	UT_uint32           m_iJustifyBy;

	const GR_Item *     m_pItem;

	TextTransform       m_TextTransform;
	bool                m_previousWasSpace;
};


#endif /* GR_RENDERINFO_H */
