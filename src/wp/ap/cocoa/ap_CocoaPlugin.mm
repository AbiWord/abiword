/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include "ut_string_class.h"
#include "ut_vector.h"

#include "xap_App.h"
#include "xap_DialogFactory.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_FileOpenSaveAs.h"
#include "xap_Frame.h"

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
		const UT_GenericStringMap<UT_UTF8String *> & map = m_Importer->getCurrentMapping();

		NSMutableArray * fieldArray = (NSMutableArray *) [m_DataSet objectAtIndex:0];

		NSMutableArray * valueArray = [NSMutableArray arrayWithCapacity:16];

		unsigned int count = [fieldArray count];

		for (unsigned int i = 0; i < count; i++)
			{
				NSString * field_name = (NSString *) [fieldArray objectAtIndex:i];

				if (UT_UTF8String * value = map.pick([field_name UTF8String]))
					{
						[valueArray addObject:[NSString stringWithUTF8String:(value->utf8_str())]];
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

@implementation AP_CocoaPlugin_FramelessDocument

+ (NSString *)optionsPropertyString:(NSDictionary *)options
{
	NSString * props = @"";

	if (options)
		{
			NSEnumerator * enumerator = [options keyEnumerator];

			NSString * key   = nil;
			NSString * value = nil;

			bool first = true;

			while (key = (NSString *) [enumerator nextObject])
				if (value = (NSString *) [options objectForKey:key])
					{
						if (first)
							props = [NSString stringWithFormat:@"%@: %@", key, value];
						else
							props = [NSString stringWithFormat:@"%@; %@: %@", props, key, value];

						first = false;
					}
		}
	return props;
}

+ (AP_CocoaPlugin_FramelessDocument *)documentFromFile:(NSString *)path importOptions:(NSDictionary *)options
{
	if (!path)
		return nil;

	NSString * impProps = [AP_CocoaPlugin_FramelessDocument optionsPropertyString:options];

	PD_Document * pNewDoc = new PD_Document(XAP_App::getApp());
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
	if (self = [super init])
		{
			m_pDocument = REFP(pDoc);
		}
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

	UT_Error error = m_pDocument->saveAs([path UTF8String], ieft, [expProps UTF8String]);

	return (error == UT_OK) ? YES : NO;
}

@end

@implementation AP_CocoaPlugin_Document

+ (id <NSObject, XAP_CocoaPlugin_Document>)currentDocument // may return nil;
{
	AP_CocoaPlugin_Document * pImpl = 0;

	XAP_App * pApp = XAP_App::getApp();

	if (XAP_Frame * pFrame = pApp->getLastFocussedFrame())
		if (pImpl = [[AP_CocoaPlugin_Document alloc] initWithXAPFrame:pFrame])
			{
				[pImpl autorelease];
			}
	return pImpl;
}

+ (NSArray *)documents
{
	XAP_App * pApp = XAP_App::getApp();

	UT_uint32 count = pApp->getFrameCount();

	if (count == 0)
		{
			return [NSArray array];
		}

	NSMutableArray * documentArray = [NSMutableArray arrayWithCapacity:((unsigned) count)];

	if (documentArray)
		for (UT_uint32 i = 0; i < count; i++)
			if (XAP_Frame * pFrame = pApp->getFrame(i))
				if (AP_CocoaPlugin_Document * pImpl = [[AP_CocoaPlugin_Document alloc] initWithXAPFrame:pFrame])
					{
						[documentArray addObject:pImpl];
						[pImpl release];
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
			path = [NSString stringWithUTF8String:(pDialog->getPathname())];
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

	UT_Vector vecFields;

	IE_MailMerge * pie = 0;
	UT_Error errorCode = IE_MailMerge::constructMerger([path UTF8String], IEMT_Unknown, &pie);
	if (!errorCode && pie)
		{
			pie->getHeaders([path UTF8String], vecFields);

			if (UT_uint32 count = vecFields.size())
				{
					NSMutableArray * fieldArray = [NSMutableArray arrayWithCapacity:((unsigned) count)];

					for (UT_uint32 i = 0; i < count; i++)
						{
							const UT_UTF8String * field = (const UT_UTF8String *) vecFields[i];
							[fieldArray addObject:[NSString stringWithUTF8String:(field->utf8_str())]];
						}
					[dataset addObject:fieldArray];

					UT_VECTOR_PURGEALL(UT_UTF8String *, vecFields);

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

	if (UT_uint32 count = pApp->getFrameCount())
		for (UT_uint32 i = 0; i < count; i++)
			if (frame == pApp->getFrame(i))
				{
					bFrameExists = YES;
					break;
				}

	return bFrameExists;
}

- (id)initWithXAPFrame:(XAP_Frame *)frame
{
	if (self = [super init])
		{
			m_pFrame = frame;
		}
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

- (NSString *)documentMailMergeSource
{
	NSString * path = 0;

	if ([AP_CocoaPlugin_Document frameExists:m_pFrame])
		if (PD_Document * pDoc = static_cast<PD_Document *>(m_pFrame->getCurrentDoc()))
			{
				UT_UTF8String link = pDoc->getMailMergeLink();
				if (link.size())
					path = [NSString stringWithUTF8String:(link.utf8_str())];
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
	if (field_name)
		if ([field_name length])
			if ([AP_CocoaPlugin_Document frameExists:m_pFrame])
				if (FV_View * pView = static_cast<FV_View *>(m_pFrame->getCurrentView()))
					{
						const XML_Char param_name[] = "param";
						const XML_Char * pParam = (const XML_Char *) [field_name UTF8String];
						const XML_Char * pAttr[3];

						pAttr[0] = static_cast<const XML_Char *>(&param_name[0]);
						pAttr[1] = pParam;
						pAttr[2] = 0;

						pView->cmdInsertField("mail_merge", static_cast<const XML_Char **>(&pAttr[0]));
					}
}

- (NSArray *)documentMailMergeFields
{
	NSMutableArray * fieldArray = [NSMutableArray arrayWithCapacity:16];

	if ([AP_CocoaPlugin_Document frameExists:m_pFrame])
		if (PD_Document * pDoc = static_cast<PD_Document *>(m_pFrame->getCurrentDoc()))
			{
				const UT_GenericStringMap<UT_UTF8String *> & map = pDoc->getMailMergeMap();

				UT_GenericStringMap<UT_UTF8String *>::UT_Cursor cursor(&map);

				UT_UTF8String * val = 0;

				for (val = cursor.first(); cursor.is_valid(); val = cursor.next())
					{
						[fieldArray addObject:[NSString stringWithUTF8String:(cursor.key().c_str())]];
					}
			}
	return fieldArray;
}

- (void)setDocumentMailMergeFields:(NSArray *)field_array
{
	if ([AP_CocoaPlugin_Document frameExists:m_pFrame])
		if (PD_Document * pDoc = static_cast<PD_Document *>(m_pFrame->getCurrentDoc()))
			{
				UT_UTF8String empty;
				UT_String key;

				pDoc->clearMailMergeMap();

				unsigned count = [field_array count];

				for (unsigned i = 0; i < count; i++)
					{
						NSString * field_name = (NSString *) [field_array objectAtIndex:i];
						key = [field_name UTF8String];
						pDoc->setMailMergeField(key, empty);
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
	if ([AP_CocoaPlugin_Document frameExists:m_pFrame])
		if (PD_Document * pDoc = static_cast<PD_Document *>(m_pFrame->getCurrentDoc()))
			{
				const UT_GenericStringMap<UT_UTF8String *> & map = pDoc->getMailMergeMap();

				UT_GenericStringMap<UT_UTF8String *>::UT_Cursor cursor(&map);

				UT_UTF8String * val = 0;

				UT_UTF8String new_value;

				for (val = cursor.first(); cursor.is_valid(); val = cursor.next())
					{
						NSString * value = (NSString *) [value_dictionary objectForKey:[NSString stringWithUTF8String:(cursor.key().c_str())]];
						if (value)
							new_value = [value UTF8String];
						else
							new_value = "";

						pDoc->setMailMergeField(cursor.key(), new_value);
					}

				s_updateMailMergeFields(m_pFrame, pDoc);
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
										pBlock->setNeedsReformat();
										pBlock->redrawUpdate();
									}

					pf = pf->getNext();
				}
			pView->updateLayout();
		}
}

