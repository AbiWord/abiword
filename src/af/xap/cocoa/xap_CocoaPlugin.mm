/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 2004 Francis James Franklin
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
#include "xap_CocoaAppController.h"
#include "xap_CocoaPlugin.h"
#include "xap_DialogFactory.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_FileOpenSaveAs.h"
#include "xap_Frame.h"

#include "pd_Document.h"

#include "ie_mailmerge.h"

@interface XAP_CocoaPlugin_DocumentImpl : NSObject <XAP_CocoaPlugin_Document>
{
	XAP_Frame *		m_pFrame;
}
+ (BOOL)frameExists:(XAP_Frame *)frame;

- (id)initWithXAPFrame:(XAP_Frame *)frame;

- (BOOL)documentStillExists;

- (NSString *)title;

- (NSString *)documentMailMergeSource; // may return nil
- (void)setDocumentMailMergeSource:(NSString *)path;

- (NSArray *)documentMailMergeFields;
- (void)setDocumentMailMergeFields:(NSArray *)field_array;

/* value_dictionary maps (NSString *) keys [field names] to (NSString *) values.
 */
- (void)setDocumentMailMergeValues:(NSDictionary *)value_dictionary;
@end

@interface XAP_CocoaPluginImpl : NSObject
{
	id <NSObject, XAP_CocoaPluginDelegate>	m_delegate;
}
- (id)init;
- (void)dealloc;

- (void)setDelegate:(id <NSObject, XAP_CocoaPluginDelegate>)delegate;
- (id <NSObject, XAP_CocoaPluginDelegate>)delegate;
@end

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

@implementation XAP_CocoaPlugin_DocumentImpl

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
	return [XAP_CocoaPlugin_DocumentImpl frameExists:m_pFrame];
}

- (NSString *)title
{
	if (![XAP_CocoaPlugin_DocumentImpl frameExists:m_pFrame])
		{
			return @"(This document no longer exists!)";
		}
	return [NSString stringWithUTF8String:(m_pFrame->getNonDecoratedTitle())];
}

- (NSString *)documentMailMergeSource
{
	NSString * path = 0;

	if ([XAP_CocoaPlugin_DocumentImpl frameExists:m_pFrame])
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
	if ([XAP_CocoaPlugin_DocumentImpl frameExists:m_pFrame])
		if (PD_Document * pDoc = static_cast<PD_Document *>(m_pFrame->getCurrentDoc()))
			{
				pDoc->setMailMergeLink([path UTF8String]);
			}
}

- (NSArray *)documentMailMergeFields
{
	NSMutableArray * fieldArray = [NSMutableArray arrayWithCapacity:16];

	if ([XAP_CocoaPlugin_DocumentImpl frameExists:m_pFrame])
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
	if ([XAP_CocoaPlugin_DocumentImpl frameExists:m_pFrame])
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

/* value_dictionary maps (NSString *) keys [field names] to (NSString *) values.
 */
- (void)setDocumentMailMergeValues:(NSDictionary *)value_dictionary
{
	if ([XAP_CocoaPlugin_DocumentImpl frameExists:m_pFrame])
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
			}
}

@end

@implementation XAP_CocoaPluginImpl

- (id)init
{
	if (self = [super init])
		{
			m_delegate = 0;
		}
	return self;
}

- (void)dealloc
{
	// 
	[super dealloc];
}

- (void)setDelegate:(id <NSObject, XAP_CocoaPluginDelegate>)delegate
{
	m_delegate = delegate;
}

- (id <NSObject, XAP_CocoaPluginDelegate>)delegate
{
	return m_delegate;
}

@end

@implementation XAP_CocoaPlugin

- (id)init
{
	if (self = [super init])
		{
			m_pImpl = [[XAP_CocoaPluginImpl alloc] init];
			if (!m_pImpl)
				{
					[self dealloc];
					self = 0;
				}
		}
	return self;
}

- (void)dealloc
{
	if (m_pImpl)
		{
			[m_pImpl release];
			m_pImpl = 0;
		}
	[super dealloc];
}

- (BOOL)loadBundleWithPath:(NSString *)path
{
	BOOL bLoaded = NO;

	if (NSBundle * bundle = [NSBundle bundleWithPath:path])
		if (![bundle isLoaded])
			if ([bundle load])
				if (Class bundleClass = [bundle principalClass])
					if (id <NSObject, XAP_CocoaPluginDelegate> instance = [[bundleClass alloc] init])
					{
						if ([instance respondsToSelector:@selector(pluginCanRegisterForAbiWord:)])
						{
							[self setDelegate:instance];
							bLoaded = [instance pluginCanRegisterForAbiWord:self];
						}
						if (!bLoaded)
						{
							[instance release];
						}
					}
	return bLoaded;
}

- (void)setDelegate:(id <NSObject, XAP_CocoaPluginDelegate>)delegate
{
	[m_pImpl setDelegate:delegate];
}

- (id <NSObject, XAP_CocoaPluginDelegate>)delegate
{
	return [m_pImpl delegate];
}

- (void)appendMenuItem:(NSMenuItem *)menuItem
{
	XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];
	[pController appendPluginMenuItem:menuItem];
}

- (void)removeMenuItem:(NSMenuItem *)menuItem
{
	XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];
	[pController removePluginMenuItem:menuItem];
}

- (id <NSObject, XAP_CocoaPlugin_Document>)currentDocument // may return nil;
{
	XAP_CocoaPlugin_DocumentImpl * pImpl = 0;

	XAP_App * pApp = XAP_App::getApp();

	if (XAP_Frame * pFrame = pApp->getLastFocussedFrame())
		if (pImpl = [[XAP_CocoaPlugin_DocumentImpl alloc] initWithXAPFrame:pFrame])
			{
				[pImpl autorelease];
			}
	return pImpl;
}

- (NSArray *)documents
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
				if (XAP_CocoaPlugin_DocumentImpl * pImpl = [[XAP_CocoaPlugin_DocumentImpl alloc] initWithXAPFrame:pFrame])
					{
						[documentArray addObject:pImpl];
						[pImpl release];
					}

	return documentArray;
}

- (NSString *)selectMailMergeSource // may return nil
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
- (NSMutableArray *)importMailMergeSource:(NSString *)path
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

@end
