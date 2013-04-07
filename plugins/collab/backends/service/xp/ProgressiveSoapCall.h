/* Copyright (C) 2008 AbiSource Corporation B.V.
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

#ifndef __PROGRESSIVE_SOAP_CALL__
#define __PROGRESSIVE_SOAP_CALL__

#ifdef _MSC_VER
#include "msc_stdint.h"
#else
#include <stdint.h>
#endif
#include <boost/bind.hpp>
#include "InterruptableAsyncWorker.h"
#include "soa_soup.h"

class ProgressiveSoapCall : public boost::enable_shared_from_this<ProgressiveSoapCall>
{
public:
	ProgressiveSoapCall(const std::string& uri, soa::function_call& fc, const std::string& ssl_ca_file)
		: m_uri(uri),
		m_mi(soa::method_invocation("urn:AbiCollabSOAP", fc)),
		m_ssl_ca_file(ssl_ca_file),
		m_worker_ptr()
	{}

	soa::GenericPtr run()
	{
		UT_DEBUGMSG(("ProgressiveSoapCall::run()\n"));

		m_worker_ptr.reset(new InterruptableAsyncWorker<bool>(
					boost::bind(&ProgressiveSoapCall::invoke, shared_from_this())
				));

		// start the asynchronous process and display the dialog
		try
		{
			bool res = m_worker_ptr->run();
			if (!res)
				return soa::GenericPtr();
			return soa::parse_response(m_result, m_mi.function().response());
		}
		catch (InterruptedException e)
		{
			UT_DEBUGMSG(("Soap call interrupted!\n"));
			return soa::GenericPtr();
		}
	}

private:
	bool invoke()
	{
		UT_DEBUGMSG(("ProgressiveSoapCall::invoke()\n"));
		return soup_soa::invoke(
						m_uri, m_mi, m_ssl_ca_file,
						boost::bind(&ProgressiveSoapCall::_progress_cb, this, _1, _2, _3),
						m_result
					);
	}

	void _progress_cb(SoupSession* session, SoupMessage* msg, uint32_t progress)
	{
		UT_DEBUGMSG(("ProgressiveSoapCall::_progress_cb()\n"));
		UT_return_if_fail(session && msg);
		UT_return_if_fail(m_worker_ptr);

		if (m_worker_ptr->cancelled())
		{
#ifdef SOUP24
			soup_session_cancel_message(session, msg, SOUP_STATUS_CANCELLED);
#else
			soup_message_set_status(msg, SOUP_STATUS_CANCELLED);
			soup_session_cancel_message(session, msg);
#endif
			return;
		}

		m_worker_ptr->progress(progress);
	}

	std::string							m_uri;
	soa::method_invocation				m_mi;
	std::string							m_ssl_ca_file;

	boost::shared_ptr< InterruptableAsyncWorker<bool> >
										m_worker_ptr;

	std::string							m_result;
};

#endif /* __PROGRESSIVE_SOAP_CALL__ */
