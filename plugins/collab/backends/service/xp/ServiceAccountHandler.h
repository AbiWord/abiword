/* Copyright (C) 2006,2007 Marc Maurer <uwog@uwog.net>
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

#ifndef __SERVICEACCOUNTHANDLER__
#define __SERVICEACCOUNTHANDLER__

#include <vector>
#include "ut_string_class.h"

#include <backends/xp/AccountHandler.h>

class PD_Document;
typedef long long UT_sint64;

extern AccountHandlerConstructor ServiceAccountHandlerConstructor;

class ServiceAccountHandler : public AccountHandler
{
public:
	ServiceAccountHandler();
	virtual ~ServiceAccountHandler();

	// housekeeping
	virtual UT_UTF8String					getDescription();
	virtual UT_UTF8String					getDisplayType();
	virtual UT_UTF8String					getStorageType();

	// dialog management 
	virtual void							storeProperties();

	// connection management
	virtual ConnectResult					connect();
	virtual bool							disconnect();
	virtual bool							isOnline();

	// user management
	virtual Buddy*							constructBuddy(const PropertyMap& props);
	virtual bool							allowsManualBuddies()
		{ return false; }
	
	// packet management
	virtual bool							send(const Packet* packet);
	virtual bool							send(const Packet*, const Buddy& buddy);

	// service specific functions
	void				    			  	updateDocumentsAsync();
	void                                    saveDocumentAsync(UT_sint64 docId, PD_Document * pDoc);	
	void									populateDocuments(const UT_UTF8String& packet);	
	void									openDocumentAsync(UT_sint64 docId);
	void									closeDocument(UT_sint64 docId, PD_Document * pDoc);

private:
};

#endif /* __SERVICEACCOUNTHANDLER__ */
