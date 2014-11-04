/* Copyright (c) 2008-2009, AbiSource Corporation B.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of AbiSource Corporation B.V. nor the
 *       names of other contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ABISOURCE CORPORATION B.V. AND OTHER
 * CONTRIBUTORS ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ABISOURCE
 * CORPORATION B.V OR OTHER CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef __TLS_TUNNEL_H__
#define __TLS_TUNNEL_H__

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <asio.hpp>
#include <string>
#include <vector>
#ifdef _MSC_VER
typedef long ssize_t;
typedef int pid_t;
#endif
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

namespace tls_tunnel {

typedef boost::shared_ptr<asio::ip::tcp::socket> socket_ptr_t;
typedef boost::shared_ptr<gnutls_session_t> session_ptr_t;
typedef boost::shared_ptr< std::vector<char> > buffer_ptr_t;

class Exception {
public:
	Exception(const std::string& message);
	const std::string& message() const;
private:
	std::string message_;
};

class Transport : public boost::enable_shared_from_this<Transport> {
public:
	asio::io_service& io_service();
	void run();
	void stop();

protected:
	Transport();
	virtual ~Transport();

private:
	asio::io_service io_service_;
	asio::io_service::work work_;
};

typedef boost::shared_ptr<Transport> transport_ptr_t;

class ClientTransport : public Transport {
public:
	ClientTransport(const std::string& host, unsigned short port,
			boost::function<void (transport_ptr_t, socket_ptr_t)> on_connect);
	void connect();
private:
	std::string host_;
	unsigned short port_;
	boost::function<void (transport_ptr_t, socket_ptr_t)> on_connect_;
};


class ServerTransport : public Transport {
public:
	ServerTransport(const std::string& ip, unsigned short port,
			boost::function<void (transport_ptr_t, socket_ptr_t)> on_connect);
	void accept();
private:
	void on_accept(const asio::error_code& error, socket_ptr_t socket_ptr);

	asio::ip::tcp::acceptor acceptor_;
	boost::function<void (transport_ptr_t, socket_ptr_t)> on_connect_;
};

class Proxy {
public:
	virtual ~Proxy();
	static bool tls_tunnel_init();
	static void tls_tunnel_deinit();
	virtual void setup() = 0;
	void run();
	virtual void stop();

protected:
	Proxy(const std::string& ca_file);

	void on_local_read(const asio::error_code& error, std::size_t bytes_transferred,
			transport_ptr_t transport_ptr, session_ptr_t session_ptr, socket_ptr_t local_socket_ptr,
			buffer_ptr_t local_buffer_ptr, socket_ptr_t remote_socket_ptr);
	void tunnel(transport_ptr_t transport_ptr, session_ptr_t session_ptr,
			socket_ptr_t local_socket_ptr, socket_ptr_t remote_socket_ptr);
	void disconnect_(transport_ptr_t transport_ptr, session_ptr_t session_ptr,
			socket_ptr_t local_socket_ptr, socket_ptr_t remote_socket_ptr);

	gnutls_certificate_credentials_t x509cred;

	transport_ptr_t transport_ptr_; // we only store this as a member so we are able to start/stop the transport

private:
	void tunnel_(transport_ptr_t transport_ptr, session_ptr_t session_ptr,
			socket_ptr_t local_socket_ptr, buffer_ptr_t local_buffer_ptr,
			socket_ptr_t remote_socket);

	asio::thread* t;
};

// FIXME: this clientproxy can only handle 1 SSL connection at the same time
class ClientProxy : public Proxy {
public:
	ClientProxy(const std::string& connect_address, unsigned short connect_port,
			const std::string& ca_file, bool check_hostname);

	virtual void setup();
	virtual void stop();

	const std::string& local_address() const;
	unsigned short local_port() const;

private:
	void on_transport_connect(transport_ptr_t transport_ptr, socket_ptr_t remote_socket_ptr);
	void on_client_connect(const asio::error_code& error, transport_ptr_t transport_ptr,
			session_ptr_t session_ptr, socket_ptr_t local_socket_ptr, socket_ptr_t remote_socket_ptr);
	session_ptr_t setup_tls_session(socket_ptr_t remote_socket_ptr);

	std::string local_address_;
	unsigned short local_port_;
	std::string connect_address_;
	unsigned short connect_port_;
	boost::shared_ptr<asio::ip::tcp::acceptor> acceptor_ptr;
	bool check_hostname_;
};

class ServerProxy : public Proxy {
public:
	ServerProxy(const std::string& bind_ip, unsigned short bind_port, unsigned short local_port,
			const std::string& ca_file, const std::string& cert_file, const std::string& key_file);

	virtual void setup();

private:
	void on_transport_connect(transport_ptr_t transport_ptr, socket_ptr_t remote_socket_ptr);
	session_ptr_t setup_tls_session(socket_ptr_t remote_socket_ptr);

	std::string bind_ip_;
	unsigned short bind_port_;
	unsigned short local_port_;
	gnutls_dh_params_t dh_params;
};

} /* namespace tls_tunnel */

#endif /* __TLS_TUNNEL_H__ */
