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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef __SOA_SOUP__
#define __SOA_SOUP__

#include <stdio.h>
#include <string>
#include <glib.h>
#include <libsoup/soup.h>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <soa.h>

namespace soup_soa {

	/* public types */
	
	struct SoaSoupSession
	{
		SoaSoupSession(SoupMessage* msg, const std::string& ssl_ca_file)
			: m_session(NULL),
			m_msg(msg),
			progress_cb_ptr(),
			received_content_length(0)
		{
			_set_session(ssl_ca_file);
		}
	
		SoaSoupSession(SoupMessage* msg, const std::string& ssl_ca_file, boost::function<void (SoupSession*, SoupMessage*, uint32_t)> progress_cb_)
			: m_session(NULL),
			m_msg(msg),
			progress_cb_ptr(new boost::function<void (SoupSession*, SoupMessage*, uint32_t)>(progress_cb_)),
			received_content_length(0)
		{
			_set_session(ssl_ca_file);
		}
		
		~SoaSoupSession() {
			if (m_session)
				g_object_unref(m_session);
			if (m_msg)
				g_object_unref(m_msg);
		}
		
		void callback(uint32_t progress) {
			if (!progress_cb_ptr)
				return;
			(*progress_cb_ptr)(m_session, m_msg, progress);
		}
		
		SoupSession* m_session;
		SoupMessage* m_msg;
		boost::shared_ptr<boost::function<void (SoupSession*, SoupMessage*, uint32_t)> > progress_cb_ptr;
		uint32_t received_content_length;
		
		private:
			void _set_session(const std::string& ssl_ca_file) {
				m_session = 
					ssl_ca_file.size() == 0
						? soup_session_sync_new_with_options(NULL)
						: soup_session_sync_new_with_options(				
											SOUP_SESSION_SSL_CA_FILE, ssl_ca_file.c_str(),
											/* TODO: add user agent */
											NULL
									);			
			}

	};

	/* private function prototypes */

#ifdef SOUP24	
	static void _got_chunk_cb(SoupMessage* msg, SoupBuffer *chunk, SoaSoupSession* progress_info);
#else
	static void _got_chunk_cb(SoupMessage *msg, SoaSoupSession* user_data);
#endif
	static soa::GenericPtr _invoke(const std::string& url, const soa::method_invocation& mi, SoaSoupSession& sess);
	
	/* public functions */
	
	soa::GenericPtr invoke(const std::string& url, const soa::method_invocation& mi, const std::string& ssl_ca_file) {
		std::string soap_msg = mi.str();
		UT_DEBUGMSG(("SOAP Request: %s\n", soap_msg.c_str()));		
		SoupMessage* msg = soup_message_new ("POST", url.c_str());
#ifdef SOUP24
		soup_message_set_request(msg, "text/xml", SOUP_MEMORY_STATIC, &soap_msg[0], soap_msg.size());
#else
		soup_message_set_request(msg, "text/xml", SOUP_BUFFER_USER_OWNED, &soap_msg[0], soap_msg.size());
#endif
		SoaSoupSession sess(msg, ssl_ca_file);
		return _invoke(url, mi, sess);
	}
	
	soa::GenericPtr invoke(const std::string& url, const soa::method_invocation& mi, const std::string& ssl_ca_file,
						   boost::function<void (SoupSession*, SoupMessage*, uint32_t)> progress_cb) {
		std::string soap_msg = mi.str();
		UT_DEBUGMSG(("SOAP Request: %s\n", soap_msg.c_str()));							   
		SoupMessage* msg = soup_message_new ("POST", url.c_str());
		SoaSoupSession sess(msg, ssl_ca_file, progress_cb);
		g_signal_connect(G_OBJECT (msg), "got-chunk", G_CALLBACK(_got_chunk_cb), (gpointer)(&sess));
#ifdef SOUP24
		soup_message_set_request(msg, "text/xml", SOUP_MEMORY_STATIC, &soap_msg[0], soap_msg.size());
#else
		soup_message_set_request(msg, "text/xml", SOUP_BUFFER_USER_OWNED, &soap_msg[0], soap_msg.size());
#endif
		return _invoke(url, mi, sess);
	}	
	
	/* private functions */
	
	static soa::GenericPtr _invoke(const std::string& /*url*/, const soa::method_invocation& mi, SoaSoupSession& sess) {
		if (!sess.m_session || !sess.m_msg )
			return soa::GenericPtr();

		guint status = soup_session_send_message (sess.m_session, sess.m_msg);
		if (!(SOUP_STATUS_IS_SUCCESSFUL (status) ||
			status == SOUP_STATUS_INTERNAL_SERVER_ERROR /* used for SOAP Faults */))
		{
			UT_DEBUGMSG(("Error executing SOAP call: %s\n", sess.m_msg->reason_phrase));
			return soa::GenericPtr();
		}
		
		// store the SOAP result in a string
		// FIXME: ineffecient copy
		std::string result;
#ifdef SOUP24
		if (!sess.m_msg->response_body || !sess.m_msg->response_body->data)
			return soa::GenericPtr();
		result.resize(sess.m_msg->response_body->length);
		std::copy(sess.m_msg->response_body->data, sess.m_msg->response_body->data+sess.m_msg->response_body->length, result.begin());
#else		
		result.resize(sess.m_msg->response.length);
		std::copy(sess.m_msg->response.body, sess.m_msg->response.body+sess.m_msg->response.length, result.begin());
#endif
		UT_DEBUGMSG(("SOAP Response: %s\n", result.c_str()));		
		return soa::parse_response(result, mi.function().response());
	}

#ifdef SOUP24
	static void _got_chunk_cb(SoupMessage* msg, SoupBuffer * /*chunk*/, SoaSoupSession* progress_info)
#else
	static void _got_chunk_cb(SoupMessage* msg, SoaSoupSession* progress_info)
#endif
	{
		UT_return_if_fail(msg && msg->response_headers);
		if (!progress_info)
			return;
		
		uint32_t content_length = 0;
#ifdef SOUP24
		content_length = (uint32_t)soup_message_headers_get_content_length(msg->response_headers);
#else
		const char* content_length_str = soup_message_get_header(msg->response_headers, "Content-Length");
		if (!content_length_str)
			return;

		try
		{
			content_length = boost::lexical_cast<uint32_t>(content_length_str);
		}
		catch (boost::bad_lexical_cast &)
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN); // unless the server is really broken
			return;
		}
#endif

		if (content_length == 0)
			return;
		
#ifdef SOUP24
		if (!msg->response_body)
			return;
		progress_info->received_content_length = msg->response_body->length;
#else
		progress_info->received_content_length += msg->response.length;
#endif
		uint32_t progress = (uint32_t)(((float)progress_info->received_content_length / content_length)*100);
		if (progress > 100)
			progress = 100;
		progress_info->callback(progress);
	}

}

#endif /* __SOA_SOUP__ */
