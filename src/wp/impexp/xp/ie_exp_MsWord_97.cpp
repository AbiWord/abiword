/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

/*
 What's been done:
 
 We Create a open a file, open the relevant streams, and
 create a FIB.  The FIB is written to give us a correct offset into the file,
 but is also stored in memory and updated when appropriate to take into account
 information learned about the document.
 
 When we are finished, the updated FIB is written to the beginning of the stream
 again, overwriting the original (wrong) FIB.
 
 actual document output is simply a basic ASCII dump of the file (this will
 need to be changed, for obvious reasons).
 
 i.e. we don't do very much yet.
 
 
 What's left to do:
 
 Find out a valid LID, and put it in the FIB (locale information.  We should
 probably choose a constant one for this, and use 16-bit character output
 instead of ASCII to make things simpler).  If we do this, we also need to make
 our character output 16 bit and modify the peicetable to show this.  Once we
 have a peice table being output, that is :-)
 
 To have a 'working' exporter we need the following in the main stream:
 
 FIB [File Information Block];
 text of body, footnotes, headers 
    Begins at the [stream, not file, I hope] position recorded in
    fib.fcMin. 
 FKPs [Formatted disK Pages] for CHPs [CHaracter Properties], 
    PAPs [PAragraph Properties] and LVCs [Not sure what these are...] 
    The first FKP begins at the 512-byte boundary after the last byte of text
    written.  The remaining FKPs are recorded in the 512-byte pages that
    immediately follow. The FKPs for CHPs PAPs and LVCs are interleaved.
    Previous versions of Word wrote them in contiguous chunks. The
    hplcfbte's of the three flavors (CHP, PAP and LVC) are used to find the
    relevant FKP of the appropriate type. 
 group of SEPXs 
    SEPXs immediately follow the FKPs and are concatenated one after the 
    other. 
    
 
 And the following in the table stream:
 
 sttbfUssr      \
 plcupcRgbuse    >- Undouumented.  Something to do with undo data.
 plcupcUsp       |  Hopefully not important.
 uskf           /

 stsh [style sheet] 
 plcffndRef     [footnote reference position table] 
    Written if the document contains footnotes.
 plcffndTxt     [footnote text position table]
    Written if the document contains footnotes.
 pgdFtn         [footnote text page description table]
    Written if the document contains footnotes.
 bkdFtn         [footnote text break descriptor table]
    Written if the document contains footnotes. 
 plcfendRef     [endnote reference position table]
    Written recorded table if the document contains endnotes.
 plcfendTxt     [endnote text position table]
    Written if the document contains endnotes.
 pgdEdn         [endnote text page description table]
    Written if the document contains endnotes.
 bkdEdn         [endnote text break descriptor table]
    Written if the document contains endnotes.
 plcftxbxTxt    [text box link table]
    Written if the document contains textboxes 
 plcftxbxBkd    [text box break descriptor table]
    Written if the document contains textboxes 
 plcfhdrtxbxTxt [header text box link table] 
    Written if the header subdocument contains textboxes 
 plcfhdrtxbxBkd [header text box break descriptor table]
    Written if the header subdocument contains textboxes. 
 grpXstAtnOwners [annotation owner table] 
    Written if the document contains annotations. 
 plcfandRef     [annotation reference position table] 
    Written if the document contains annotations 
 plcfandTxt     [annotation text position table] 
    Written if the document contains annotations. 
 plcfsed        [section table] 
    'Recorded' in all Word documents (why not 'written'?).
 pgdMother      [page description table] 
    Written in all Word documents 
 bkdMother      [break descriptor table] 
    Written in all Word documents 
 plcfphe        [paragraph height table] 
    Written if paragraph heights have been recorded. Only written during a 
    fast save (so we won't need to write it). 
 plcfsea        [private] 
    'PLCF reserved for private use by Word' - lets hope we don't need to put
    something valid into this. 
 plcflvc        [list and outline level table] 
    Written during fast save only (i.e. we don't need to worry about it). 
 plcasumy       [AutoSummary analysis] 
    Written if the document stored is in AutoSummary view mode. 
 sttbGlsy       [glossary name string table] 
    Written if the document stored is a glossary. 
 sttbGlsyStyle  [glossary style name string table] 
    Written if the document stored is a glossary. 
 plcfglsy       [glossary entry text position table] 
    Written if the document stored is a glossary. 
 plcfhdd        [header text position table] 
    Written if the document contains headers or footers. 
 plcfbteChpx    [bin table for CHP FKPs] 
    Recorded in all Word documents. 
 plcfbtePapx    [bin table for PAP FKPs] 
    Recorded in all Word documents. 
 plcfbteLvc     [bin table for LVC FKPs] 
    Recorded in all Word documents. 
 sttbfRMark     [revision mark author string table] 
    Written if the document contains revision marks. 
 PlcffldMom     [table of field positions and statuses for main document] 
    Written recorded table if the main document contains fields. 
 PlcffldHdr     [table of field positions and statuses for header subdocument] 
    Written if the header subdocument contains fields. 
 PlcffldFtn     [table of field positions and statuses for footnote subdocument] 
    Written if the footnote subdocument contains fields. 
 PlcffldAtn     [table of field positions and statuses for annotation subdocument] 
    Written if the annotation subdocument contains fields. 
 PlcffldEdn     [table of field positions and statuses for endnote subdocument] 
    Written if the endnote subdocument contains fields. 
 PlcffldTxbx    [table of field positions and statuses for textbox subdocument] 
    Written if the textbox subdocument contains fields. 
 plcOcx         [ocx position table] 
    Written if the document contains ole controls. Undocumented, but we won't
    need it anyway. 
 PlcffldHdrTxbx [table of field positions and statuses for textbox subdocument
                 of header subdocument] 
    Written if the textbox subdocument of the header subdocument contains
    fields. 
 dggInfo        [office drawing information] 
    Written recorded table. Format is apparently described in the 'Office 
    drawing group format document'.  Perhaps we can save our vector graphics in
    it? :-) 
 plcspaMom      [office drawing table] 
    Written if the document contains office drawings. 
 plcspaHdr      [header office drawing table] 
    Written if the header subdocument contains office drawings.
 sttbfBkmk      [table of bookmark name strings] 
    Written if the document contains bookmarks. 
 plcfBkmkf      [table recording beginning CPs of bookmarks] 
    Written if the document contains bookmarks. 
 plcfBkmkl      [table recording limit CPs of bookmarks] 
    Written if the document contains bookmarks. 
 sttbfAtnBkmk   [table of annotation bookmark string names] 
    Written if the document contains annotations with bookmarks. 
 plcfAtnbkf     [table recording beginning CPs of bookmarks in the annotation
                 subdocument] 
    Written previously recorded table, if the document contains annotations 
    with bookmarks. 
 plcfAtnbkl     [table recording limit CPs of bookmarks in the annotation 
                 subdocument] 
    Written previously recorded table, if the document contains anotations with
    bookmarks. 
 plcfspl        [spelling state table] 
    Written recorded table. Records state of spell checking in a PLCF of SPLS
    structures. 
 plcfgram       [grammar state table] 
    Written recorded table. Records state of grammar checking in a PLCF of
    SPLS structures.
 plcfwkb        [work book document partition table] 
    Written if the document is a master document. 
 formFldSttbs   [form field dropdown string tables] 
    Written if the document contains form field dropdown controls. 
 sttbCaption    [caption title string table] 
    Written if the document contains captions. 
 sttbAutoCaption [auto caption string table] 
    Written if the document contains auto captions. 
 sttbFnm        [filename reference string table] 
    Written if the document references other documents. 
 sttbSavedBy    [last saved by string table] 
    Written immedietly after the previously recorded table (Hmmm...?)
 plcflst        [list formats] 
    Written if there are any lists defined in the document. This begins
    with a short count of LSTF structures followed by those LSTF structures. 
    This is immediately followed by the allocated data hanging off the LSTFs.
    This data consists of the array of LVLs for each LSTF. (Each LVL consists
    of an LVLF followed by two grpprls and an XST.) 
 plflfo         [more list formats] 
    Written if there are any lists defined in the document.
    This consists first of a PL of LFO records, followed by the allocated data 
    (if any) hanging off the LFOs. The allocated data consists of the array of
    LFOLVLFs for each LFO (and each LFOLVLF is immediately followed by some 
    LVLs). 
 sttbfListNames [more list formats] 
    Written if there are any lists defined in the document.  This is a string
    table containg the list names for each list. It is parallel with the
    plcflst, and may contain null strings if the corresponding LST does not
    have a list name. 
 hplgosl        [grammar option settings] 
    This undocumented structure maps LID and grammar checker type
    to grammar checking options.  I hope we don't need to write this.  If we do,
    perhaps we can just copy one from an existing document, and hope everything
    will be okay...
 stwUser        [macro user storage] 
 routeSlip      [mailer routing slip] 
    Written if this document has a mailer routing slip (i.e. we won't need it). 
 cmds [recording of command data structures] 
    Written if special commands are linked to this document (i.e. not written). 
 prDrvr         [printer driver information] 
    Written if a print environment is recorded for the document (no). 
 prEnvPort      [print environment in portrait mode] 
    Written if a portrait mode print environment is recorded for this document
    (no). 
 prEnvLand      [print environment in landscape mode] 
    Written if a landscape mode print environment is recorded for this
    document (no). 
 wss            [window state structure] 
    Written if the document was saved while a window was open (we should be able
    to ignore this). 
 pms            [print merge state] 
    Written if information about the print / mail merge state is recorded for
    the document (no). 
 clx            [encoding of the sprm lists for a complex file and piece table
                 for a any file] 
    Recorded in all Word documents. 
 sttbfffn       [table of font name strings] 
    Recorded in all Word documents. The sttbfffn is an sttbf where each string
    is instead an FFN structure [note that just as for a pascal-style string, 
    the first byte in the FFN records the total number of bytes not counting 
    the count byte itself]. The names of the fonts correspond to the ftc codes 
    in the CHP structure. For example, the first font name listed corresponds
    is the name for ftc = 0. [this sounds a bit nasty...]
 sttbttmbd      [true type font embedding string table] 
    Writtenif document contains embedded true type fonts (no [at least, not yet
    :-]). 
 dop            [document properties record] 
    Recorded in all Word documents.
 sttbfAssoc     [table of associated strings] 
    ?
 autosaveSource [name of original] 
    This field only appears in autosave files.  So we won't need it.
    
 (Don't you love Microsoft's abbreviations?)
 
 First to be tackled should be the style sheets and peice table, I think.
 
 Later we can do translation of fancy things like felds and pictures :-)
 
 */

#include <stdlib.h>
#include <string.h>

/* These following included files can probably be thinned down a bit/lot */

#include "ut_string.h"
#include "ut_types.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "pt_Types.h"
#include "ie_exp_MsWord_97.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_units.h"
#include "ie_types.h"

#include "pp_AttrProp.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "xap_App.h"

/********************************************************/
/********************************************************/

IE_Exp_MsWord_97::IE_Exp_MsWord_97(PD_Document *pDocument)
	: IE_Exp(pDocument)
{
	m_error = 0;
	m_pListener = NULL;
}

IE_Exp_MsWord_97::~IE_Exp_MsWord_97()
{
}

/********************************************************/
/********************************************************/

UT_Bool IE_Exp_MsWord_97::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".doc") == 0);
}

UT_Error IE_Exp_MsWord_97::StaticConstructor(PD_Document * pDocument,
                                             IE_Exp ** ppie)
{
	IE_Exp_MsWord_97 * p = new IE_Exp_MsWord_97(pDocument);
        *ppie = p;
	return UT_OK;
}

UT_Bool IE_Exp_MsWord_97::GetDlgLabels(const char ** pszDesc,
                                        const char ** pszSuffixList,
                                        IEFileType * ft)
{
	*pszDesc = "Microsoft Word (.doc)";
	*pszSuffixList = "*.doc";
	*ft = IEFT_MsWord_97;
	return UT_TRUE;
}

UT_Bool IE_Exp_MsWord_97::SupportsFileType(IEFileType ft)
{
	return (IEFT_MsWord_97 == ft);
}

/********************************************************/
/********************************************************/

class s_MsWord_97_Listener : public PL_Listener
{
public:

	s_MsWord_97_Listener (PD_Document *pDocument,
						  IE_Exp_MsWord_97 * pie);

	virtual ~s_MsWord_97_Listener();

	virtual UT_Bool populate(PL_StruxFmtHandle sfh,
		  				     const PX_ChangeRecord * pcr);

	virtual UT_Bool populateStrux(PL_StruxDocHandle sdh,
							 	  const PX_ChangeRecord * pcr,
							 	  PL_StruxFmtHandle * psfh);


	virtual UT_Bool change(PL_StruxFmtHandle sfh,
						   const PX_ChangeRecord * pcr);

	virtual UT_Bool insertStrux(PL_StruxFmtHandle sfh,
								const PX_ChangeRecord * pcr,
	                            PL_StruxDocHandle sdh,
	                            PL_ListenerId lid,
                                void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
		                                                PL_ListenerId lid,
		                                                PL_StruxFmtHandle sfhNew));

	virtual UT_Bool signal(UT_uint32 iSignal);

protected:	
    void				_closeSection(void);
	void				_closeBlock(void);
	void				_closeSpan(void);
	void				_openSpan(PT_AttrPropIndex apiSpan);
	void				_openTag(const char * szPrefix, const char * szSuffix,
								 UT_Bool bNewLineAfter, PT_AttrPropIndex api);
	void				_outputData(const UT_UCSChar * p, UT_uint32 length);
	void				_handleStyles(void);
	void				_handleDataItems(void);


	void _convertColor (UT_sint32 * msCol, const char * szFrom);

	void _convertFontSize  (UT_sint32 * szDest, const char * szFrom);

	void _convertFontFace  (char * szDest, const char * szFrom);

	PD_Document *		m_pDocument;
	IE_Exp_MsWord_97*	m_pie;
	UT_Bool				m_bInSection;
	UT_Bool				m_bInBlock;
	UT_Bool				m_bInSpan;
	PT_AttrPropIndex	m_apiLastSpan;
};

/********************************************************/
/********************************************************/

UT_Error IE_Exp_MsWord_97::_writeDocument(void)
{

	// create and install a listener that will recieve the
	// current document and write/export a MsWord 97
	// document
	//

	m_pListener = new s_MsWord_97_Listener (m_pDocument, this);

	if(!m_pListener)
		return UT_IE_NOMEMORY;

	if (m_pDocRange)
	    m_pDocument->tellListenerSubset(static_cast<PL_Listener *>(m_pListener),m_pDocRange);
	else
	    m_pDocument->tellListener(static_cast<PL_Listener *>(m_pListener));

	DELETEP(m_pListener);
	return ((m_error) ? UT_IE_COULDNOTWRITE : UT_OK);
}


// Overriding the methods in our base class is necessary
// because we are relying on wv to do a binary write of our
// data in a OLE stream/document. As of writing this, all
// other exporters are text-based and would thus share much
// in common. Unfortunately, we do not.
//

UT_Bool IE_Exp_MsWord_97::_openFile(const char * szFileName)
{
	UT_ASSERT(szFileName);

	m_pExporter = wvExporter_create(szFileName);
	if(!m_pExporter)
	  {
	    UT_DEBUGMSG(("MSWord Exporter: NULL exporter object\n"));
	    return UT_FALSE;
	  }

	UT_DEBUGMSG(("MSWord Exporter: created exporter object\n"));
	UT_DEBUGMSG(("Populating the summary stream\n"));

	wvExporter_summaryPutString(m_pExporter, PID_APPNAME, "AbiWord");

#if 0
	// we may be able to fill some/all of these in
	wvExporter_summaryPutString(m_pExporter, PID_TITLE, "");
	wvExporter_summaryPutString(m_pExporter, PID_AUTHOR, "");

	wvExporter_summaryPutLong(m_pExporter, PID_PAGECOUNT, 0);
	wvExporter_summaryPutLong(m_pExporter, PID_WORDCOUNT, 0);
	wvExporter_summaryPutLong(m_pExporter, PID_CHARCOUNT, 0);
#endif

	return UT_TRUE;
}

UT_uint32 IE_Exp_MsWord_97::_writeBytes(const UT_Byte * pBytes, UT_uint32 length)
{
	UT_ASSERT(pBytes);
	UT_ASSERT(length);

	UT_DEBUGMSG(("Writing %s", pBytes));

	return wvExporter_writeBytes(m_pExporter, sizeof(UT_Byte), 
				     length, (void*)pBytes);
}

UT_Bool IE_Exp_MsWord_97::_writeBytes(const UT_Byte * pBytes)
{
	UT_ASSERT(pBytes);
	UT_uint32 length = (UT_uint32)strlen((const char *)pBytes);
	UT_ASSERT(length);

	return (_writeBytes(pBytes, length) == length);
}

UT_Bool IE_Exp_MsWord_97::_closeFile(void)
{
	UT_Bool tmp = UT_TRUE;

	wvExporter_close(m_pExporter);
	return tmp;
}

// NOTE: We may be able to use our base-class' implementation
// NOTE: As the current code is identical
// NOTE: But I'd rather be paranoid :)
//
void IE_Exp_MsWord_97::_abortFile(void)
{
	_closeFile();
	return;
}

void IE_Exp_MsWord_97::write(const char * sz)
{
	write(sz, (UT_uint32)strlen(sz));
}

void IE_Exp_MsWord_97::write(const char * sz, UT_uint32 length)
{
	if(m_error)
		return;

	m_error |= (_writeBytes ((UT_Byte *)sz, length) != length);
}

/********************************************************/
/********************************************************/

s_MsWord_97_Listener::s_MsWord_97_Listener(PD_Document * pDocument,
										   IE_Exp_MsWord_97 * pie)
{
	m_pDocument = pDocument;
	m_pie = pie;

	UT_DEBUGMSG(("Beginning Word Export\n"));
}

s_MsWord_97_Listener::~s_MsWord_97_Listener()
{
        // nothing
}

UT_Bool s_MsWord_97_Listener::populate(PL_StruxFmtHandle /*sfh*/,
									  const PX_ChangeRecord * pcr)
{
	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);

			//PT_AttrPropIndex api = pcr->getIndexAP();
			//_openSpan(api);
			
			PT_BufIndex bi = pcrs->getBufIndex();
			_outputData(m_pDocument->getPointer(bi),pcrs->getLength());

			return UT_TRUE;
		}

	case PX_ChangeRecord::PXT_InsertObject:
		{
			const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);
			//PT_AttrPropIndex api = pcr->getIndexAP();
			switch (pcro->getObjectType())
			{
                
            // Should do something with these.
			case PTO_Image:
				
                return UT_TRUE;

			case PTO_Field:
				
				return UT_TRUE;

			default:
				UT_ASSERT(0);
				return UT_FALSE;
			}
		}

	case PX_ChangeRecord::PXT_InsertFmtMark:
		return UT_TRUE;
		
	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
}

UT_Bool s_MsWord_97_Listener::populateStrux(PL_StruxDocHandle /*sdh*/,
										   const PX_ChangeRecord * pcr,
										   PL_StruxFmtHandle * psfh)
{
	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);
	*psfh = 0;							// we don't need it.

	switch (pcrx->getStruxType())
	{
	case PTX_Section:
		{
			_closeSpan();
			_closeBlock();
			_closeSection();
			_openTag("section","",UT_TRUE,pcr->getIndexAP());
			m_bInSection = UT_TRUE;
			return UT_TRUE;
		}

	case PTX_Block:
		{
			_closeSpan();
			_closeBlock();
			_openTag("p","",UT_FALSE,pcr->getIndexAP());
			m_bInBlock = UT_TRUE;
			return UT_TRUE;
		}

	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
}

UT_Bool s_MsWord_97_Listener::change(PL_StruxFmtHandle /*sfh*/,
									const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(0);						// this function is not used.
	return UT_FALSE;
}

UT_Bool s_MsWord_97_Listener::insertStrux(PL_StruxFmtHandle /*sfh*/,
										  const PX_ChangeRecord * /*pcr*/,
										  PL_StruxDocHandle /*sdh*/,
										  PL_ListenerId /* lid */,
										  void (* /*pfnBindHandles*/)(PL_StruxDocHandle /* sdhNew */,
																	  PL_ListenerId /* lid */,
																	  PL_StruxFmtHandle /* sfhNew */))
{
	UT_ASSERT(0);						// this function is not used.
	return UT_FALSE;
}

UT_Bool s_MsWord_97_Listener::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return UT_FALSE;
}


void s_MsWord_97_Listener::_closeSection(void) {}
void s_MsWord_97_Listener::_closeBlock(void) {}
void s_MsWord_97_Listener::_closeSpan(void) {}
void s_MsWord_97_Listener::_openSpan(PT_AttrPropIndex apiSpan) {}
void s_MsWord_97_Listener::_openTag(const char * szPrefix, const char * szSuffix,
							    UT_Bool bNewLineAfter, PT_AttrPropIndex api) {}

void s_MsWord_97_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length) 
{
#define MY_BUFFER_SIZE		1024
#define MY_HIGHWATER_MARK	20
	char buf[MY_BUFFER_SIZE];
	char * pBuf;
	const UT_UCSChar * pData;

	for (pBuf=buf, pData=data; (pData<data+length); /**/)
	{
		if (pBuf >= (buf+MY_BUFFER_SIZE-MY_HIGHWATER_MARK))
		{
			wvExporter_writeBytes(m_pie->m_pExporter, sizeof(UT_UCSChar), pBuf-buf, buf);
			pBuf = buf;
		}

		switch (*pData)
		{
            
        // Theses are ripped from the AbiWord format exporter.
        // We should either convert them to their Word equivalents here,
        // or put something in wv to do this and simply run the buffer through
        // it.
		case UCS_LF:					// LF -- representing a Forced-Line-Break
			*pBuf++ = '<';				// these get mapped to <br/>
			*pBuf++ = 'b';
			*pBuf++ = 'r';
			*pBuf++ = '/';
			*pBuf++ = '>';
			pData++;
			break;
			
		case UCS_VTAB:					// VTAB -- representing a Forced-Column-Break
			*pBuf++ = '<';				// these get mapped to <cbr/>
			*pBuf++ = 'c';
			*pBuf++ = 'b';
			*pBuf++ = 'r';
			*pBuf++ = '/';
			*pBuf++ = '>';
			pData++;
			break;
			
		case UCS_FF:					// FF -- representing a Forced-Page-Break
			*pBuf++ = '<';				// these get mapped to <pbr/>
			*pBuf++ = 'p';
			*pBuf++ = 'b';
			*pBuf++ = 'r';
			*pBuf++ = '/';
			*pBuf++ = '>';
			pData++;
			break;
			
		default:
			if (*pData > 0x007f)
			{

				// We want to do proper conversion here.
				char localBuf[20];
				char * plocal = localBuf;
				sprintf(localBuf,"&#x%x;",*pData++);
				while (*plocal)
					*pBuf++ = (UT_Byte)*plocal++;

			}
			else
			{
				*pBuf++ = (UT_Byte)*pData++;
			}
			break;
		}
	}

    wvExporter_writeBytes(m_pie->m_pExporter, sizeof(UT_UCSChar), pBuf-buf, buf);
}

void s_MsWord_97_Listener::_handleStyles(void) {}
void s_MsWord_97_Listener::_handleDataItems(void) {}


void s_MsWord_97_Listener::_convertColor (UT_sint32 * mscol, const char * szFrom)
{
	// convert from our 6 hex-digit color scheme to
	// msword 97's (tiny) 16-color palette
	//

	UT_sint32 ftcol = (UT_sint32)atoi(szFrom);

	// NOTE: this only supports the 16 colors currently handled in
	// NOTE: the import code. everything else gets treated as black
	// NOTE: this is an area for improvement
	//
	switch (ftcol) {

		case 0x0000ff: //blue
			*mscol = 2;
			break;

		case 0x00ffff: //cyan
			*mscol = 3;
			break;

		case 0x00ff00: //green
			*mscol = 4;
			break;

		case 0xff00ff: //magenta
			*mscol = 5;
			break;

		case 0xff0000: //red
			*mscol = 6;
			break;

		case 0xffff00: //yellow
			*mscol = 7;
			break;

		case 0xffffff: //white
			*mscol = 8;
			break;

		case 0x000080: //dark blue
			*mscol = 9;
			break;

		case 0x008080: //dark cyan
			*mscol = 10;
			break;

		case 0x008000: //dark green
			*mscol = 11;
			break;

		case 0x800080: //dark magenta
			*mscol = 12;
			break;

		case 0x800000: //dark red
			*mscol = 13;
			break;

		case 0x808000: //dark yellow
			*mscol = 14;
			break;

		case 0x808080: //dark gray
			*mscol = 15;
			break;

		case 0xc0c0c0: //light gray
			*mscol = 16;
			break;

		default:
			// 0x000000 and anything else/unknown will be treated as black
			//
			*mscol = 1;
			break;
	}
}

void s_MsWord_97_Listener::_convertFontSize  (UT_sint32 * szDest, const char * szFrom)
{
	// msword uses half-points to represent font sizes
	// and we use full points
	//
	*szDest = (UT_sint32)(2 * atoi(szFrom));
}

void s_MsWord_97_Listener::_convertFontFace  (char * szDest, const char * szFrom)
{
	// i believe that there is a 1-1 correspondence here, but
	// we'll let wv handle the proper encoding somewhere else
	//
	strcpy (szDest, szFrom);
}
