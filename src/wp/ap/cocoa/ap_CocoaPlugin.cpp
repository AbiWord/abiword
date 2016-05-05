/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiSource Application Framework
 * Copyright (C) 2005 Francis James Franklin
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

#include "xap_App.h"
#include "xap_CocoaAppController.h"
#include "xap_DialogFactory.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_FileOpenSaveAs.h"
#include "xap_Frame.h"
#include "xap_Menu_Layouts.h"

#include "ev_EditMethod.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu_Labels.h"

#include "pd_Document.h"

#include "fd_Field.h"

#include "fl_BlockLayout.h"

#include "fp_Line.h"
#include "fp_Run.h"

#include "fv_View.h"

#include "ie_exp.h"
#include "ie_mailmerge.h"

#include "ap_CocoaPlugin.h"

static void s_updateMailMergeFields(XAP_Frame * pFrame, PD_Document * pDoc);

class XAP_Cocoa_MailMerge_Listener : public IE_MailMerge::IE_MailMerge_Listener
{
	IE_MailMerge *		m_Importer;

	NSMutableArray *	m_DataSet;

public:
	XAP_Cocoa_MailMerge_Listener(IE_MailMerge * importer, NSMutableArray * dataset) :
		m_Importer(importer),
		m_DataSet(dataset)
	{
		[m_DataSet retain];
	}

	virtual ~XAP_Cocoa_MailMerge_Listener()
	{
		[m_DataSet release];
	}
		
	virtual PD_Document * getMergeDocument() const
	{
		return 0;
	}
	virtual bool fireUpdate()
	{
		const std::map<std::string, std::string> & map = m_Importer->getCurrentMapping();

		NSMutableArray * fieldArray = (NSMutableArray *) [m_DataSet objectAtIndex:0];

		NSMutableArray * valueArray = [NSMutableArray arrayWithCapacity:16];

		unsigned int count = [fieldArray count];

		for (unsigned int i = 0; i < count; i++)
			{
				NSString * field_name = (NSString *) [fieldArray objectAtIndex:i];

				std::map<std::string, std::string>::const_iterator iter = map.find([field_name UTF8String]);
				if (iter != map.end())
					{
						[valueArray addObject:[NSString stringWithUTF8String:(iter->second.c_str())]];
					}
				else
					{
						[valueArray addObject:@""];
					}
			}
		[m_DataSet addObject:valueArray];

		return true;
	}
};

static unsigned long s_EditMethod_Number = 0;

static bool s_EditMethod_CtxtFn (AV_View * /*pView*/, EV_EditMethodCallData * /*pCallData*/, void * context)
{
	AP_CocoaPlugin_EditMethod * editMethod = (AP_CocoaPlugin_EditMethod *) context;

	[editMethod trigger];
	return true;
}

@implementation AP_CocoaPlugin_EditMethod

- (id)init
{
	if (![super init]) {
		return nil;
	}
	m_Action = 0;
	m_Target = 0;

	m_EditMethod = 0;

	m_EditMethod_Name[0] = 0;

	EV_EditMethodContainer * pEMC = XAP_App::getApp()->getEditMethodContainer();
	if (pEMC) {
		sprintf(m_EditMethod_Name, "[%lX] CocoaPlugin EditMethod", s_EditMethod_Number++);

		m_EditMethod = new EV_EditMethod(m_EditMethod_Name, s_EditMethod_CtxtFn, 0, "", self);
	}
	if (m_EditMethod) {
		if (!pEMC->addEditMethod(m_EditMethod))	{
			DELETEP(m_EditMethod);
		}
	}
	if (!m_EditMethod) {
		[self release];
		return nil;
	}
	return self;
}

- (void)dealloc
{
	if (m_EditMethod)
		{
			EV_EditMethodContainer * pEMC = XAP_App::getApp()->getEditMethodContainer();
			if (pEMC)
				{
					pEMC->removeEditMethod(m_EditMethod);
					delete m_EditMethod;
				}
			m_EditMethod = 0;
		}
	[super dealloc];
}

- (const char *)editMethodName
{
	return m_EditMethod_Name;
}

- (void)trigger
{
	if (m_Action && m_Target)
		if ([m_Target respondsToSelector:m_Action])
			{
				[NSApp sendAction:m_Action to:m_Target from:self];
			}
}

- (void)setAction:(SEL)aSelector
{
	m_Action = aSelector;
}

- (SEL)action
{
	return m_Action;
}

- (void)setTarget:(id <NSObject>)target
{
	m_Target = target;
}

- (id <NSObject>)target
{
	return m_Target;
}

@end

@implementation AP_CocoaPlugin_MenuIDRef

+ (AP_CocoaPlugin_MenuIDRef *)menuIDRefWithMenuItem:(id <XAP_CocoaPlugin_MenuItem>)menuItem
{
	AP_CocoaPlugin_MenuIDRef * ref = [[AP_CocoaPlugin_MenuIDRef alloc] initWithMenuItem:menuItem];

	[ref autorelease];

	return ref;
}

- (id)initWithMenuItem:(id <XAP_CocoaPlugin_MenuItem>)menuItem
{
	if (![super init]) {
		return nil;
	}
	m_NonRetainedRef = menuItem;
	return self;
}

- (id <XAP_CocoaPlugin_MenuItem>)menuItem
{
	return m_NonRetainedRef;
}

@end

static EV_Menu_ItemState s_GetMenuItemState_Fn (AV_View * /*pView*/, XAP_Menu_Id menuid)
{
	XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];

	AP_CocoaPlugin_MenuIDRef * ref = [pController refForMenuID:[NSNumber numberWithInt:((int) menuid)]];

	int _state = EV_MIS_ZERO;

	if (ref) {
		id <XAP_CocoaPlugin_MenuItem> menuItem = [ref menuItem];

		// TODO: validate against target ??

		if ([menuItem isEnabled] == NO) {
			_state |= EV_MIS_Gray;		//	= 0x01,		/* item is or should be gray */
		}
		if ([menuItem state] == NSOnState) {
			_state |= EV_MIS_Toggled;	//	= 0x02,		/* checkable item should be checked */
		}
	}
	return (EV_Menu_ItemState)_state;
}

static const char * s_GetMenuItemComputedLabel_Fn (const EV_Menu_Label * pLabel, XAP_Menu_Id menuid)
{
	XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];

	AP_CocoaPlugin_MenuIDRef * ref = [pController refForMenuID:[NSNumber numberWithInt:((int) menuid)]];

	const char * label = pLabel->getMenuLabel();

	if (ref)
		{
			id <XAP_CocoaPlugin_MenuItem> menuItem = [ref menuItem];

			// TODO: validate against target ??

			label = [[menuItem label] UTF8String];
		}
	return label;
}

@implementation AP_CocoaPlugin_ContextMenuItem

+ (AP_CocoaPlugin_ContextMenuItem *)itemWithLabel:(NSString *)label
{
	AP_CocoaPlugin_ContextMenuItem * item = nil;

	if (label) {
		item = [[AP_CocoaPlugin_ContextMenuItem alloc] initWithLabel:label];
		if (item) {
			[item autorelease];
		}
	}
	return item;
}

- (id)initWithLabel:(NSString *)label
{
	if (![super init]) {
		return nil;
	}
	m_Label = [label retain];

	m_Tag = 0;

	m_State = NSOffState;
	m_Enabled = YES;

	m_pAction = 0;

	EV_Menu_ActionSet * pActionSet = XAP_App::getApp()->getMenuActionSet();

	m_MenuID = 0;

	XAP_Menu_Factory * pFact = XAP_App::getApp()->getMenuFactory();

	if (pFact && pActionSet)
	{
		m_MenuID = pFact->addNewMenuAfter("contextText", 0, "Bullets and &Numbering", EV_MLF_Normal);
	}
	if (m_MenuID)
	{
		pFact->addNewLabel(0, m_MenuID, [m_Label UTF8String], [m_Label UTF8String]);

		const char * editMethodName = [self editMethodName];

		/* Create the Action that will be called.
		 */
		m_pAction = new EV_Menu_Action(m_MenuID,                      // id that the layout said we could use
									   false,                         // no, we don't have a sub-menu.
									   false,                         // no, we don't raise a dialog. (this just adds dots after the title)
									   true,                          // yes, we have a check-box.
									   false,                         // no, we aren't a radio.
									   editMethodName,                // name of callback function to call.
									   s_GetMenuItemState_Fn,         // the get-state function
									   s_GetMenuItemComputedLabel_Fn  // the get-label function
									   );
	}
	if (m_pAction)
	{
		if (pActionSet->addAction(m_pAction))
		{
			AP_CocoaPlugin_MenuIDRef * ref = [AP_CocoaPlugin_MenuIDRef menuIDRefWithMenuItem:self];

			XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];
			[pController addRef:ref forMenuID:[NSNumber numberWithInt:((int) m_MenuID)]];
		}
		else
		{
			DELETEP(m_pAction);
		}
	}
	if (!m_pAction) {
		[self release];
		return nil;
	}
	return self;
}

- (void)dealloc
{
	XAP_Menu_Factory * pFact = XAP_App::getApp()->getMenuFactory();
	if (m_pAction && pFact)
		{
			XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];
			[pController removeRefForMenuID:[NSNumber numberWithInt:((int) m_MenuID)]];

			// there is no way to remove actions, so... don't delete m_pAction? (FIXME??)

			pFact->removeMenuItem("contextText", 0, [m_Label UTF8String]);
		}

	[m_Label release];

	[super dealloc];
}

- (void)setLabel:(NSString *)label
{
	if (label) {
		[m_Label release];
		m_Label = [label retain];
	}
}

- (NSString *)label
{
	return m_Label;
}

- (void)setTag:(int)anInt
{
	m_Tag = anInt;
}

- (int)tag
{
	return m_Tag;
}

- (void)setState:(int)state
{
	m_State = (state == NSOnState) ? NSOnState : NSOffState;
}

- (int)state
{
	return m_State;
}

- (void)setEnabled:(BOOL)enabled
{
	m_Enabled = enabled ? YES : NO;
}

- (BOOL)isEnabled
{
	return m_Enabled;
}

@end

@implementation AP_CocoaPlugin_FramelessDocument

+ (NSString *)optionsPropertyString:(NSDictionary *)options
{
    NSString * props = @"";

    bool first = true;

	NSEnumerator* keyEnumerator = [options keyEnumerator];
    while(NSString *key = [keyEnumerator nextObject])
    {
        NSString *value = (NSString *) [options objectForKey:key];
        if (value)
        {
            if (first)
            {
                first = false;
                props = [NSString stringWithFormat:@"%@: %@", key, value];
                continue;
            }

            props = [NSString stringWithFormat:@"%@; %@: %@", props, key, value];
        }
    }
    return props;
}

+ (AP_CocoaPlugin_FramelessDocument *)documentFromFile:(NSString *)path importOptions:(NSDictionary *)options
{
	if (!path)
		return nil;

	NSString * impProps = [AP_CocoaPlugin_FramelessDocument optionsPropertyString:options];

	PD_Document * pNewDoc = new PD_Document();
	if (!pNewDoc)
		return nil;

	UT_Error error = pNewDoc->readFromFile([path UTF8String], IEFT_Unknown, [impProps UTF8String]);
	if (error != UT_OK)
		{
			UNREFP(pNewDoc);
			return nil;
		}

	AP_CocoaPlugin_FramelessDocument * FD = [[AP_CocoaPlugin_FramelessDocument alloc] initWithPDDocument:pNewDoc];
	[FD autorelease];

	UNREFP(pNewDoc);

	return FD;
}

- (id)initWithPDDocument:(PD_Document *)pDoc
{
	if (![super init]) {
		return nil;
	}
	m_pDocument = REFP(pDoc);
	return self;
}

- (void)dealloc
{
	UNREFP(m_pDocument);
	[super dealloc];
}

/* This identifies target file type by the path extension (e.g., .html)
 */
- (BOOL)exportDocumentToFile:(NSString *)path exportOptions:(NSDictionary *)options
{
	if (!path)
		return NO;

	NSString * expProps = [AP_CocoaPlugin_FramelessDocument optionsPropertyString:options];

	NSString * ext = [path pathExtension];

	if (![ext length])
		return NO;

	ext = [NSString stringWithFormat:@".%@", ext];

	IEFileType ieft = IE_Exp::fileTypeForSuffix([ext UTF8String]);
	GsfOutput * out = UT_go_file_create([path UTF8String], NULL);

	if (out) {
		UT_Error error = m_pDocument->saveAs(out, ieft, [expProps UTF8String]);
		
		gsf_output_close (out);
		g_object_unref (G_OBJECT (out));

		return (error == UT_OK) ? YES : NO;
	}
	return NO;
}

@end

@implementation AP_CocoaPlugin_Document

+ (id <NSObject, XAP_CocoaPlugin_Document>)currentDocument // may return nil;
{
	AP_CocoaPlugin_Document * pImpl = nil;

	XAP_App * pApp = XAP_App::getApp();

	if (XAP_Frame * pFrame = pApp->getLastFocussedFrame()) {
		pImpl = [[AP_CocoaPlugin_Document alloc] initWithXAPFrame:pFrame];
		if (pImpl) {
			[pImpl autorelease];
		}
	}
	return pImpl;
}

+ (NSArray *)documents
{
	XAP_App * pApp = XAP_App::getApp();

	UT_sint32 count = pApp->getFrameCount();

	if (count == 0)
	{
		return [NSArray array];
	}

	NSMutableArray * documentArray = [NSMutableArray arrayWithCapacity:((unsigned) count)];

	if (documentArray) {
		for (UT_sint32 i = 0; i < count; i++) {
			XAP_Frame * pFrame = pApp->getFrame(i);
			if (pFrame) {
				AP_CocoaPlugin_Document * pImpl = [[AP_CocoaPlugin_Document alloc] initWithXAPFrame:pFrame];
				if (pImpl) {
					[documentArray addObject:pImpl];
					[pImpl release];
				}
			}
		}
	}

	return documentArray;
}

+ (NSString *)selectMailMergeSource // may return nil
{
	XAP_App * pApp = XAP_App::getApp();

	XAP_Dialog_Id dlgid = XAP_DIALOG_ID_FILE_OPEN;
	
	XAP_DialogFactory * pDialogFactory = static_cast<XAP_DialogFactory *>(pApp->getDialogFactory());

	XAP_Dialog_FileOpenSaveAs * pDialog = static_cast<XAP_Dialog_FileOpenSaveAs *>(pDialogFactory->requestDialog(dlgid));
	if (!pDialog)
		{
			return nil;
		}

	UT_uint32 filterCount = IE_MailMerge::getMergerCount();

	const char ** szDescList   = static_cast<const char **>(UT_calloc(filterCount + 1, sizeof(char *)));
	const char ** szSuffixList = static_cast<const char **>(UT_calloc(filterCount + 1, sizeof(char *)));

	IEMergeType * nTypeList = static_cast<IEMergeType *>(UT_calloc(filterCount + 1, sizeof(IEMergeType)));

	UT_uint32 k = 0;
	while (IE_MailMerge::enumerateDlgLabels(k, &szDescList[k], &szSuffixList[k], &nTypeList[k]))
		k++;

	pDialog->setFileTypeList(szDescList, szSuffixList, static_cast<const UT_sint32 *>(nTypeList));

	pDialog->setDefaultFileType(IE_MailMerge::fileTypeForSuffix(".xml"));

	pDialog->runModal(0);

	NSString * path = nil;

	if (pDialog->getAnswer() == XAP_Dialog_FileOpenSaveAs::a_OK)
		{
			path = [NSString stringWithUTF8String:pDialog->getPathname().c_str()];
		}
	pDialogFactory->releaseDialog(pDialog);

	FREEP(szDescList);
	FREEP(szSuffixList);
	FREEP(nTypeList);

	return path;
}

/* Returns an NSMutableArray whose objects are NSMutableArray of NSString, the first row holding the
 * field names, the rest being records; returns nil on failure.
 */
+ (NSMutableArray *)importMailMergeSource:(NSString *)path
{
	if (!path)
		{
			return nil;
		}

	NSMutableArray * dataset = [NSMutableArray arrayWithCapacity:32];

	std::vector<std::string> vecFields;

	IE_MailMerge * pie = 0;
	UT_Error errorCode = IE_MailMerge::constructMerger([path UTF8String], IEMT_Unknown, &pie);
	if (!errorCode && pie)
		{
			pie->getHeaders([path UTF8String], vecFields);

			if (UT_sint32 count = vecFields.size())
				{
					NSMutableArray * fieldArray = [NSMutableArray arrayWithCapacity:((unsigned) count)];

					for (UT_sint32 i = 0; i < count; i++)
						{
							const std::string & field = vecFields[i];
							[fieldArray addObject:[NSString stringWithUTF8String:field.c_str()]];
						}
					[dataset addObject:fieldArray];

					XAP_Cocoa_MailMerge_Listener listener(pie, dataset);

					pie->setListener(&listener);
					pie->mergeFile([path UTF8String]);
				}
			else
				{
					dataset = 0;
				}
			DELETEP(pie);
		}
	else
		{
			dataset = 0;
		}
	return dataset;
}

+ (BOOL)frameExists:(XAP_Frame *)frame
{
	BOOL bFrameExists = NO;

	XAP_App * pApp = XAP_App::getApp();

	for (UT_sint32 i = 0; i < pApp->getFrameCount(); i++) {
		if (frame == pApp->getFrame(i))	{
			bFrameExists = YES;
			break;
		}
	}

	return bFrameExists;
}

- (id)initWithXAPFrame:(XAP_Frame *)frame
{
	if (![super init]) {
		return nil;
	}
	m_pFrame = frame;
	return self;
}

/* XAP_CocoaPlugin_Document implementation
 */

- (BOOL)documentStillExists
{
	return [AP_CocoaPlugin_Document frameExists:m_pFrame];
}

- (NSString *)title
{
	if (![AP_CocoaPlugin_Document frameExists:m_pFrame])
		{
			return @"(This document no longer exists!)";
		}
	return [NSString stringWithUTF8String:(m_pFrame->getNonDecoratedTitle())];
}

- (NSString *)selectWord
{
	NSString * selection = nil;

	if ([AP_CocoaPlugin_Document frameExists:m_pFrame])
		if (FV_View * pView = static_cast<FV_View *>(m_pFrame->getCurrentView()))
			{
				/* If the user is on a word, but does not have it selected, we need
				 * to go ahead and select that word.
				 */
				pView->moveInsPtTo(FV_DOCPOS_EOW_MOVE);
				pView->moveInsPtTo(FV_DOCPOS_BOW);

				pView->extSelTo(FV_DOCPOS_EOW_SELECT);

				if (!pView->isSelectionEmpty())
					{
						UT_UCS4Char * ucs4 = 0;
						pView->getSelectionText(ucs4);
						if (ucs4)
							{
								UT_UTF8String utf8(ucs4);
								selection = [NSString stringWithUTF8String:(utf8.utf8_str())];
							}
					}
			}
	return selection;
}

- (NSString *)selectedText
{
	NSString * selection = nil;

	if ([AP_CocoaPlugin_Document frameExists:m_pFrame])
		if (FV_View * pView = static_cast<FV_View *>(m_pFrame->getCurrentView()))
			{
				if (!pView->isSelectionEmpty())
					{
						UT_UCS4Char * ucs4 = 0;
						pView->getSelectionText(ucs4);
						if (ucs4)
							{
								UT_UTF8String utf8(ucs4);
								selection = [NSString stringWithUTF8String:(utf8.utf8_str())];
							}
					}
			}
	return selection;
}

- (void)insertText:(NSString *)text
{
	if (text)
		if ([text length])
			if ([AP_CocoaPlugin_Document frameExists:m_pFrame])
				if (FV_View * pView = static_cast<FV_View *>(m_pFrame->getCurrentView()))
					{
						UT_UCS4String ucs4([text UTF8String]);

						pView->cmdCharInsert(ucs4.ucs4_str(), ucs4.length());
					}
}

- (NSString *)documentMailMergeSource
{
	NSString * path = 0;

	if ([AP_CocoaPlugin_Document frameExists:m_pFrame])
		if (PD_Document * pDoc = static_cast<PD_Document *>(m_pFrame->getCurrentDoc()))
			{
				const std::string & link = pDoc->getMailMergeLink();
				if (!link.empty())
					path = [NSString stringWithUTF8String:(link.c_str())];
			}

	return path;
}

- (void)setDocumentMailMergeSource:(NSString *)path
{
	if (path)
		if ([AP_CocoaPlugin_Document frameExists:m_pFrame])
			if (PD_Document * pDoc = static_cast<PD_Document *>(m_pFrame->getCurrentDoc()))
				{
					pDoc->setMailMergeLink([path UTF8String]);
				}
}

- (void)insertDocumentMailMergeField:(NSString *)field_name
{
	if (field_name) {
		if ([field_name length]) {
			if ([AP_CocoaPlugin_Document frameExists:m_pFrame]) {
				if (FV_View * pView = static_cast<FV_View *>(m_pFrame->getCurrentView())) {
						const PP_PropertyVector pAttr = {
							"param", [field_name UTF8String]
						};

						pView->cmdInsertField("mail_merge", pAttr);
					}
			}
		}
	}
}

- (NSArray *)documentMailMergeFields
{
	NSMutableArray * fieldArray = [NSMutableArray arrayWithCapacity:16];

	if ([AP_CocoaPlugin_Document frameExists:m_pFrame]) {
		if (PD_Document * pDoc = static_cast<PD_Document *>(m_pFrame->getCurrentDoc()))	{
			const std::map<std::string, std::string> & map = pDoc->getMailMergeMap();

			std::string val;
			
			for (auto iter = map.begin(); iter != map.end(); ++iter) {
				[fieldArray addObject:[NSString stringWithUTF8String:(iter->first.c_str())]];
			}
		}
	}
	return fieldArray;
}

- (void)setDocumentMailMergeFields:(NSArray *)field_array
{
	if ([AP_CocoaPlugin_Document frameExists:m_pFrame]) {
		if (PD_Document * pDoc = static_cast<PD_Document *>(m_pFrame->getCurrentDoc()))	{
			std::string empty;

			pDoc->clearMailMergeMap();

			unsigned count = [field_array count];

			for (unsigned i = 0; i < count; i++) {
				NSString * field_name = [field_array objectAtIndex:i];
				pDoc->setMailMergeField([field_name UTF8String], empty);
			}
		}
	}
}

- (void)unsetDocumentMailMergeFields
{
	if ([AP_CocoaPlugin_Document frameExists:m_pFrame])
		if (PD_Document * pDoc = static_cast<PD_Document *>(m_pFrame->getCurrentDoc()))
			{
				pDoc->clearMailMergeMap();

				s_updateMailMergeFields(m_pFrame, pDoc);
			}
}

/* value_dictionary maps (NSString *) keys [field names] to (NSString *) values.
 */
- (void)setDocumentMailMergeValues:(NSDictionary *)value_dictionary
{
	if ([AP_CocoaPlugin_Document frameExists:m_pFrame]) {
		if (PD_Document * pDoc = static_cast<PD_Document *>(m_pFrame->getCurrentDoc()))	{
			const std::map<std::string, std::string> & map = pDoc->getMailMergeMap();

			std::string new_value;

			for (auto iter = map.begin(); iter != map.end(); ++iter) {
				NSString * value = [value_dictionary objectForKey:[NSString stringWithUTF8String:(iter->first.c_str())]];
				if (value) {
					new_value = [value UTF8String];
				} else {
					new_value = "";
				}
				pDoc->setMailMergeField(iter->first, new_value);
			}

			s_updateMailMergeFields(m_pFrame, pDoc);
		}
	}
}

@end

/* TODO: move this to FV_View ??
 */
static void s_updateMailMergeFields(XAP_Frame * pFrame, PD_Document * pDoc)
{
	if (FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView()))
		{
			pf_Frag * pf = 0;

			fl_BlockLayout * pLastBlock = 0;

			while (true)
				{
					pf = pDoc->findFragOfType(pf_Frag::PFT_Object, (UT_sint32) PTO_Field, pf);

					if (!pf || (pf == pDoc->getLastFrag()))
						break;

					if (fd_Field * pField = pf->getField())
						if (pField->getFieldType() == fd_Field::FD_MailMerge)
							if (fl_BlockLayout * pBlock = pView->getBlockAtPosition(pf->getPos()))
								if (pLastBlock != pBlock)
									{
										pLastBlock = pBlock;

										fp_Run * pRun = pBlock->getFirstRun();
										while (pRun)
											{
												if (pRun->getBlock() != pBlock)
													break;

												if (pRun->getType() == FPRUN_FIELD)
													{
														fp_FieldRun * pFieldRun = static_cast<fp_FieldRun *>(pRun);

														if (pFieldRun->getFieldType() == FPFIELD_mail_merge)
															{
																pFieldRun->calculateValue();
																pFieldRun->recalcWidth();

																if (fp_Line * pLine = pRun->getLine())
																	{
																		pLine->layout();
																	}
															}
													}
												pRun = pRun->getNextRun();
											}
										pBlock->setNeedsReformat(pBlock);
										pBlock->redrawUpdate();
									}

					pf = pf->getNext();
				}
			pView->updateLayout();
		}
}

