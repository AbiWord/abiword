/* Copyright (C) 2008-2009 AbiSource Corporation B.V.
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

#ifndef __ABICOLLAB_SAVE_INTERCEPTOR__
#define __ABICOLLAB_SAVE_INTERCEPTOR__

#include "ev_EditMethod.h"
#include "RealmConnection.h"

class AV_View;
class EV_EditMethodCallData;
class EV_EditMethod;
class PD_Document;
class ServiceAccountHandler;

class AbiCollabSaveInterceptor
{
public:
	AbiCollabSaveInterceptor();

	bool intercept(AV_View * v, EV_EditMethodCallData * d);
	bool save(PD_Document * pDoc);

private:
	bool _save(std::string uri, bool verify_webapp_host, std::string ssl_ca_file,
			soa::function_call_ptr fc_ptr, boost::shared_ptr<std::string> result_ptr);
	void _save_cb(bool success, ServiceAccountHandler* pAccount,
			AbiCollab* pSession, ConnectionPtr connection_ptr,
			soa::function_call_ptr fc_ptr, boost::shared_ptr<std::string> result_ptr);
	void _saveFailed(AbiCollab* pSession);

	EV_EditMethod* m_pOldSaveEM;
};

#endif /* __ABICOLLAB_SAVE_INTERCEPTOR__ */
