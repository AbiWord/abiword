/* AbiWord
 * Copyright (C) 2004 Tomas Frydrych, <tomasfrydrych@yahoo.co.uk>
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

#ifndef GR_RENDERINFO_H
#define GR_RENDERINFO_H

#include "ut_types.h"
#include "ut_vector.h"
#include "gr_ContextGlyph.h"
#include <fribidi.h>

class UT_TextIterator;
class GR_Graphics;
class GR_Font;

/*
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

/*
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
class GR_Itemization
{
  public:
	GR_Itemization(){};
	virtual ~GR_Itemization() {};

	UT_uint32     getItemCount() const {return m_vOffsets.getItemCount();}
	UT_uint32     getNthOffset(UT_uint32 i) const {return m_vOffsets.getNthItem(i);}
	GR_ScriptType getNthType(UT_uint32 i) const {return (GR_ScriptType)m_vTypes.getNthItem(i);}

	void addItem(UT_uint32 offset, UT_uint32 type)
	         { m_vOffsets.addItem(offset); m_vTypes.addItem(type);}

	void insertItem(UT_uint32 indx, UT_uint32 offset, UT_uint32 type)
	         { m_vOffsets.insertItemAt(offset, indx); m_vTypes.insertItemAt(type,indx);}
	
	void clear()
	         {m_vOffsets.clear(); m_vTypes.clear();} 
	
  private:
	UT_NumberVector m_vOffsets;
	UT_NumberVector m_vTypes;
};

/*
   encapsulates output of GR_Graphics::shape() which is passed as
   input to GR_Graphics::renderChars().

   This is abstract class and suitable functionality is to be provided
   by platform code.

   Notes on append(), split() and cut()
   ------------------------------------
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

// add as required
enum GRRI_Type
{
	GRRI_XP = 0,
	GRRI_WIN32,
	GRRI_UNIX,
	GRRI_QNX,
	GRRI_BEOS,
	GRRI_COCOA,
	GRRI_WIN32_UNISCRIBE,

	GRRI_BUILT_IN_LAST = 0x0000ffff,

	GRRI_UNKNOWN = 0xffffffff
};

class GR_RenderInfo
{
  public:
	GR_RenderInfo(GR_ScriptType type)
		: m_iOffset(0), m_iLength(0), m_eShapingResult(GRSR_Unknown),
 		  m_eState(GRSR_Unknown), m_eScriptType(type),
		  m_pText(NULL), m_iVisDir(FRIBIDI_TYPE_LTR),
	      m_xoff(0), m_yoff(), m_pWidths(NULL),
	      m_pGraphics(NULL), m_pFont(NULL),
		  m_iJustificationPoints(0),
		  m_iJustificationAmount(0),
		  m_bLastOnLine(false){};
	
	
	virtual ~GR_RenderInfo(){};

	virtual GRRI_Type getType() const = 0;
	
	virtual bool append(GR_RenderInfo &ri, bool bReverse = false) = 0;
	virtual bool split (GR_RenderInfo *&pri, UT_uint32 offset, bool bReverse = false) = 0;
	virtual bool cut(UT_uint32 offset, UT_uint32 iLen, bool bReverse = false) = 0;

	virtual bool canAppend(GR_RenderInfo &ri) const
	              {return (m_eScriptType == ri.m_eScriptType);}

	virtual bool isJustified() const = 0;
	
	
	UT_uint32           m_iOffset;
	UT_uint32           m_iLength;
	GRShapingResult     m_eShapingResult;
	GRShapingResult     m_eState;
	GR_ScriptType       m_eScriptType;
	UT_TextIterator *   m_pText;
	FriBidiCharType     m_iVisDir;
	UT_sint32           m_xoff;
	UT_sint32           m_yoff;
	UT_sint32 *         m_pWidths;

	GR_Graphics *       m_pGraphics;
	GR_Font     *       m_pFont;

	UT_uint32           m_iJustificationPoints;
	UT_sint32           m_iJustificationAmount;
	bool                m_bLastOnLine;
	

 private:
	// never to be implemented (constructor always has to set
	// m_eScriptType to actual value)
	GR_RenderInfo() {};
};

/*
    This is an xp implementation of GR_RenderInfo for use with the
    built in UT_contextGlyph class.
*/
class GR_XPRenderInfo : public GR_RenderInfo
{
  public:
	GR_XPRenderInfo(GR_ScriptType type);
	
	
	GR_XPRenderInfo(UT_UCS4Char *pChar,
				  UT_sint32 * pAdv,
				  UT_uint32 offset,
				  UT_uint32 len,
				  UT_uint32 iBufferSize,
				  GR_ScriptType type);
	
	
	virtual ~GR_XPRenderInfo();

	virtual GRRI_Type getType() const {return GRRI_XP;}
	
	virtual bool append(GR_RenderInfo &ri, bool bReverse = false);
	virtual bool split (GR_RenderInfo *&pri, UT_uint32 offset, bool bReverse = false);
	virtual bool cut(UT_uint32 offset, UT_uint32 iLen, bool bReverse = false);

	virtual bool isJustified() const {return (m_iSpaceWidthBeforeJustification >= 0);}
	
	void prepareToRenderChars();
	

	UT_UCS4Char *       m_pChars;
	UT_sint32 *         m_pAdvances;
	UT_uint32           m_iBufferSize;
	UT_uint32 *         m_pSegmentOffset;
	UT_uint32           m_iSegmentCount;
	UT_sint32           m_iSpaceWidthBeforeJustification; // <0 for not justified

	// these can be static as for now we do not want to chache anything
	static UT_uint32	    s_iClassInstanceCount;
	static UT_UCS4Char *    s_pCharBuff;
	static UT_sint32 *      s_pWidthBuff;
	static UT_uint32        s_iBuffSize;

  private:
	void                   _constructorCommonCode();
	inline void            _stripLigaturePlaceHolders();
	inline void            _calculateCharAdvances();
	inline bool            _checkAndFixStaticBuffers();
	
};


/*
   Encapsulates input to GR_Graphics::shape()
*/
class GR_ShapingInfo
{
  public:
	GR_ShapingInfo(UT_TextIterator & text, UT_uint32 iLen,
				   GR_ScriptType type, const char * pLang,
				   FriBidiCharType iVisDir,
				   bool (*isGlyphAvailable)(UT_UCS4Char g, void * custom),
				   void * param, GRShapingResult eShapingRequired,
				   GR_Font * pFont)
		:m_Text(text), m_Type(type), m_iLength(iLen), m_pLang(pLang), m_iVisDir(iVisDir),
	     m_isGlyphAvailable(isGlyphAvailable), m_param(param),
		 m_eShapingRequired(eShapingRequired),
	     m_pFont(pFont),
	     m_iJustifyBy(0){};
	
	virtual ~GR_ShapingInfo() {};

	UT_TextIterator &   m_Text;
	GR_ScriptType       m_Type;
	UT_uint32           m_iLength;
	const char *        m_pLang;
	FriBidiCharType     m_iVisDir;
	bool (*m_isGlyphAvailable)(UT_UCS4Char g, void * custom);
	void *              m_param;
	GRShapingResult     m_eShapingRequired;

	GR_Font *           m_pFont;

	UT_uint32           m_iJustifyBy;
	
};


#endif /* GR_RENDERINFO_H */
