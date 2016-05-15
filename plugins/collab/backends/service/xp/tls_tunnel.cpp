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


#include "tls_tunnel.h"

#define return_val_if_neg(C, val) { if (C < 0) {return val;} }

namespace tls_tunnel {

#define MIN_CLIENT_PORT 50000
#define MAX_CLIENT_PORT 50100

#define TUNNEL_BUFFER_SIZE 4096
#define LOCAL_BUFFER_SIZE 4096

#define TRANSPORT_ERROR "Transport exception: "
#define TLS_SETUP_ERROR "Error setting up TLS connection"
#define TLS_CREDENTIALS_ERROR "Error setting up TLS connection"
#define TLS_VERIFY_PEER_ERROR "Error verifying peer"
#define TLS_VERIFY_HOSTNAME_ERROR "Error verifying hostname"
#define TLS_CANT_GET_PEER_CERT_ERROR "Failed to get peer certificate"

typedef boost::shared_ptr<asio::ip::tcp::socket> socket_ptr_t;
typedef boost::shared_ptr<gnutls_session_t> session_ptr_t;
typedef boost::shared_ptr< std::vector<char> > buffer_ptr_t;

bool Proxy::tls_tunnel_init() {
	if (gnutls_global_init() != 0)
		return false;
	return true;
}

void Proxy::tls_tunnel_deinit() {
	gnutls_global_deinit();
}

Exception::Exception(const std::string& msg)
	: message_(msg)
{
}
	
const std::string& Exception::message() const {
	return message_;
}
	
// private class; should't be in the public api
class _SessionPtrDestuctor {
public:
	void operator()(gnutls_session_t* session) {
		if (!session || !*session)
			return;
		gnutls_deinit(*session);			
		delete session;
	}
};

asio::io_service& Transport::io_service() {
	return io_service_;
}

void Transport::run() {
	asio::error_code ec;
	io_service_.run(ec);
}

void Transport::stop() {
	io_service_.stop();
}

Transport::Transport()
	: io_service_(),
	work_(io_service_)
{
}

Transport::~Transport()
{
}

ClientTransport::ClientTransport(const std::string& host, unsigned short port, 
		boost::function<void (transport_ptr_t, socket_ptr_t)> on_connect)
	: Transport(),
	host_(host),
	port_(port),
	on_connect_(on_connect)
{
}

void ClientTransport::connect() {
	asio::ip::tcp::resolver resolver(io_service());
	asio::ip::tcp::resolver::query query(host_, boost::lexical_cast<std::string>(port_));
	asio::ip::tcp::resolver::iterator iterator(resolver.resolve(query));
	socket_ptr_t socket_ptr(new asio::ip::tcp::socket(io_service()));

	if (iterator == asio::ip::tcp::resolver::iterator())
		throw asio::system_error(asio::error::host_not_found);

	bool connected = false;
	asio::error_code error_code;
	while (iterator != asio::ip::tcp::resolver::iterator())
	{
		try
		{
			socket_ptr->connect(*iterator);
			connected = true;
			break;
		}
		catch (asio::system_error se)
		{
			error_code = se.code();
			try { socket_ptr->close(); } catch(...) {}
		}
		iterator++;
	}
	if (!connected)
		throw asio::system_error(error_code); // throw the last error on failure
	on_connect_(shared_from_this(), socket_ptr);
}

ServerTransport::ServerTransport(const std::string& ip, unsigned short port, 
		boost::function<void (transport_ptr_t, socket_ptr_t)> on_connect) 
	: Transport(),
	acceptor_(io_service(), asio::ip::tcp::endpoint(asio::ip::address_v4::from_string(ip), port)),
	on_connect_(on_connect)
{
}

void ServerTransport::accept() {
	socket_ptr_t socket_ptr(new asio::ip::tcp::socket(io_service()));
	acceptor_.async_accept(*socket_ptr, boost::bind(&ServerTransport::on_accept, this, asio::placeholders::error, socket_ptr));
}

void ServerTransport::on_accept(const asio::error_code& error, socket_ptr_t socket_ptr) {
	if (error) {
		return;
	}
	on_connect_(shared_from_this(), socket_ptr);
	accept();
}

static ssize_t read(gnutls_transport_ptr_t ptr, void* buffer, size_t size) {
	asio::ip::tcp::socket* socket = reinterpret_cast<asio::ip::tcp::socket*>(ptr);
	try {
		return asio::read(*socket, asio::buffer(buffer, size));
	} catch (asio::system_error& /*se*/) {
		return -1;
	}
}

static ssize_t write(gnutls_transport_ptr_t ptr, const void* buffer, size_t size) {
	asio::ip::tcp::socket* socket = reinterpret_cast<asio::ip::tcp::socket*>(ptr);
	try {
		return asio::write(*socket, asio::buffer(buffer, size));
	} catch (asio::system_error& /*se*/) {
		return -1;
	}
}

Proxy::~Proxy() {
	stop();
	gnutls_certificate_free_credentials(x509cred);
}

void Proxy::run()
{	
	// We copy the transport member pointer here to make sure the transport
	// object can't be deleted another thread (for example by the reset call 
	// in ::stop()), while the transport is still running...
	// This is the reason that I would love to get rid of the transport member
	// variable, but I don't know how yet...
	transport_ptr_t trans(transport_ptr_);
	if (trans)
		trans->run();
	trans.reset();
}

void Proxy::stop()
{
	if (transport_ptr_)
		transport_ptr_->stop();
	
	if (t) {
		t->join();
		t = NULL;
	}

	transport_ptr_.reset();
}

Proxy::Proxy(const std::string& ca_file)
	: transport_ptr_(),
	t(NULL)
{
	// setup certificates
	if (gnutls_certificate_allocate_credentials(&x509cred) < 0)
		throw Exception(TLS_SETUP_ERROR);
	if (gnutls_certificate_set_x509_trust_file(x509cred, ca_file.c_str(), GNUTLS_X509_FMT_PEM) < 0)
		throw Exception(TLS_SETUP_ERROR);
}

void Proxy::on_local_read(const asio::error_code& error, std::size_t bytes_transferred,
		transport_ptr_t transport_ptr, session_ptr_t session_ptr, socket_ptr_t local_socket_ptr, 
		buffer_ptr_t local_buffer_ptr, socket_ptr_t remote_socket_ptr)
{
	if (error) {
		disconnect_(transport_ptr, session_ptr, local_socket_ptr, remote_socket_ptr);
		return;
	}

	// write the data to the tunnel connection
	int num_forwarded = gnutls_record_send(*session_ptr, &(*local_buffer_ptr)[0], bytes_transferred);
	if (num_forwarded < 0) {
		disconnect_(transport_ptr, session_ptr, local_socket_ptr, remote_socket_ptr);
		return;
	}

	local_socket_ptr->async_receive(
			asio::buffer(&(*local_buffer_ptr)[0], local_buffer_ptr->size()),
			boost::bind(&Proxy::on_local_read, this, asio::placeholders::error, asio::placeholders::bytes_transferred,
					transport_ptr, session_ptr, local_socket_ptr, local_buffer_ptr, remote_socket_ptr)
		);
}

void Proxy::tunnel(transport_ptr_t transport_ptr, session_ptr_t session_ptr,
		socket_ptr_t local_socket_ptr, socket_ptr_t remote_socket_ptr)
{
	buffer_ptr_t local_buffer_ptr(new std::vector<char>(LOCAL_BUFFER_SIZE));
	t = new asio::thread(boost::bind(&Proxy::tunnel_, this, transport_ptr,
							session_ptr, local_socket_ptr, local_buffer_ptr, remote_socket_ptr));
}

void Proxy::disconnect_(transport_ptr_t /*transport_ptr*/, session_ptr_t session_ptr,
		socket_ptr_t local_socket_ptr, socket_ptr_t remote_socket_ptr)
{			
	// shutdown the tls session (ignore any error condition)
	if (session_ptr)
		gnutls_bye(*session_ptr, GNUTLS_SHUT_RDWR);

	// shutdown the sockets belonging to this tunnel
	asio::error_code ec;
	if (local_socket_ptr && local_socket_ptr->is_open()) {
		local_socket_ptr->shutdown(asio::ip::tcp::socket::shutdown_both, ec);
		local_socket_ptr->close(ec);
	}

	if (remote_socket_ptr && remote_socket_ptr->is_open()) {	
		remote_socket_ptr->shutdown(asio::ip::tcp::socket::shutdown_both, ec);
		remote_socket_ptr->close(ec);
	}
}

void Proxy::tunnel_(transport_ptr_t transport_ptr, session_ptr_t session_ptr, socket_ptr_t local_socket_ptr, 
		buffer_ptr_t local_buffer_ptr, socket_ptr_t remote_socket_ptr)
{
	local_socket_ptr->async_receive(
			asio::buffer(&(*local_buffer_ptr)[0], local_buffer_ptr->size()),
			boost::bind(&Proxy::on_local_read, this, asio::placeholders::error, asio::placeholders::bytes_transferred, 
					transport_ptr, session_ptr, local_socket_ptr, local_buffer_ptr, remote_socket_ptr)
		);
	
	ssize_t bytes_transferred = 0;
	std::vector<char> tunnel_buffer(TUNNEL_BUFFER_SIZE);
	while (true) {
		bytes_transferred = gnutls_record_recv(*session_ptr, &tunnel_buffer[0], tunnel_buffer.size());
		
		if (bytes_transferred == 0)
			break;
			
		// TODO: check return code properly?
		if (bytes_transferred < 0)
			break;
		
		// forward the data over the local connection
		try {
			asio::write(*local_socket_ptr, asio::buffer(&tunnel_buffer[0], bytes_transferred));
		} catch (asio::system_error& /*se*/) {
			break;
		}
	}

	disconnect_(transport_ptr, session_ptr, local_socket_ptr, remote_socket_ptr);		
}

// FIXME: this clientproxy can only handle 1 SSL connection at the same time
ClientProxy::ClientProxy(const std::string& connect_address, unsigned short connect_port, 
		const std::string& ca_file, bool check_hostname)
	: Proxy(ca_file),
	local_address_("127.0.0.1"),
	local_port_(0),
	connect_address_(connect_address),
	connect_port_(connect_port),
	acceptor_ptr(),
	check_hostname_(check_hostname)
{
}

void ClientProxy::setup()
{
	try
	{
		// FIXME: should we make the proxy a shared ptr?
		transport_ptr_.reset(new ClientTransport(connect_address_, connect_port_,
											boost::bind(&ClientProxy::on_transport_connect, this, _1, _2)));

		for (unsigned short port = MIN_CLIENT_PORT; port <= MAX_CLIENT_PORT; port++) {
			try {
				acceptor_ptr.reset(
						new asio::ip::tcp::acceptor(transport_ptr_->io_service(),
														asio::ip::tcp::endpoint(asio::ip::address_v4::from_string(local_address_),
														port), false));
				local_port_ = port;
				break;
			} catch (asio::system_error& se) {
				if (port == MAX_CLIENT_PORT)
					throw se;
				if (se.code() != asio::error::address_in_use)
					throw se;
				// this port is already in use, try another one
				continue;
			}
		}

		// connect the transport
		boost::static_pointer_cast<ClientTransport>(transport_ptr_)->connect();
	} catch (asio::system_error& se) {
		throw Exception(std::string(TRANSPORT_ERROR) + se.what());
	}	
}

void ClientProxy::stop()
{
	acceptor_ptr->close();
	acceptor_ptr.reset();
	Proxy::stop();
}

const std::string& ClientProxy::local_address() const {
	return local_address_;
}

unsigned short ClientProxy::local_port() const {
	return local_port_;
}

void ClientProxy::on_transport_connect(transport_ptr_t transport_ptr, socket_ptr_t remote_socket_ptr) {
	session_ptr_t session_ptr = setup_tls_session(remote_socket_ptr);
	if (!session_ptr) {
		disconnect_(transport_ptr, session_ptr_t(), socket_ptr_t(), remote_socket_ptr);
		throw Exception(TLS_SETUP_ERROR);
	}
	
	// start accepting connections on the local socket
	socket_ptr_t local_socket_ptr(new asio::ip::tcp::socket(transport_ptr->io_service()));
	acceptor_ptr->async_accept(*local_socket_ptr, boost::bind(&ClientProxy::on_client_connect, this, 
			asio::placeholders::error, transport_ptr, session_ptr, local_socket_ptr, remote_socket_ptr));
}

void ClientProxy::on_client_connect(const asio::error_code& error, 
		transport_ptr_t transport_ptr, session_ptr_t session_ptr,
		socket_ptr_t local_socket_ptr, socket_ptr_t remote_socket_ptr) {
	if (error) {
		disconnect_(transport_ptr, session_ptr, local_socket_ptr, remote_socket_ptr);
		return;
	}
	
	tunnel(transport_ptr, session_ptr, local_socket_ptr, remote_socket_ptr);
}

session_ptr_t ClientProxy::setup_tls_session(socket_ptr_t remote_socket_ptr) {
	session_ptr_t session_ptr(new gnutls_session_t(), _SessionPtrDestuctor());

	// setup session
	return_val_if_neg(gnutls_init(session_ptr.get(), GNUTLS_CLIENT), session_ptr_t());
	return_val_if_neg(gnutls_set_default_priority(*session_ptr), session_ptr_t());
	return_val_if_neg(gnutls_credentials_set(*session_ptr, GNUTLS_CRD_CERTIFICATE, x509cred), session_ptr_t());

	// setup transport
	gnutls_transport_set_pull_function(*session_ptr,tls_tunnel::read);
	gnutls_transport_set_push_function(*session_ptr,tls_tunnel::write);
	gnutls_transport_set_ptr(*session_ptr, remote_socket_ptr.get());	

	// handshake	
	return_val_if_neg(gnutls_handshake(*session_ptr), session_ptr_t());

	// verify peer
	unsigned int status;
	if (gnutls_certificate_verify_peers2(*session_ptr, &status) != 0)
		throw Exception(TLS_VERIFY_PEER_ERROR);

	gnutls_x509_crt cert;
	const gnutls_datum* cert_list;
	unsigned int cert_list_size;

	// check hostname
	return_val_if_neg(gnutls_x509_crt_init(&cert), session_ptr_t());
	cert_list = gnutls_certificate_get_peers(*session_ptr, &cert_list_size);
	if (!cert_list)
		throw Exception(TLS_CANT_GET_PEER_CERT_ERROR);

	return_val_if_neg(gnutls_x509_crt_import(cert, &cert_list[0], GNUTLS_X509_FMT_DER), session_ptr_t());
	char name[256] = {0};
	size_t namesize = sizeof(name);
	return_val_if_neg(gnutls_x509_crt_get_dn(cert, name, &namesize), session_ptr_t());
	if (check_hostname_ && gnutls_x509_crt_check_hostname(cert, connect_address_.c_str()) == 0)
		throw Exception(TLS_VERIFY_HOSTNAME_ERROR);
	
	return session_ptr;
}

ServerProxy::ServerProxy(const std::string& bind_ip, unsigned short bind_port, unsigned short local_port,
		const std::string& ca_file, const std::string& cert_file, const std::string& key_file)
try
	: Proxy(ca_file),
	bind_ip_(bind_ip),
	bind_port_(bind_port),
	local_port_(local_port)
{
	// setup tls server state
	if (gnutls_certificate_set_x509_key_file (x509cred, cert_file.c_str(), key_file.c_str(), GNUTLS_X509_FMT_PEM) < 0)
		throw Exception(TLS_SETUP_ERROR);

	if (gnutls_dh_params_init(&dh_params) < 0)
		throw Exception(TLS_SETUP_ERROR);

	if (gnutls_dh_params_generate2(dh_params, 1024) < 0)
		throw Exception(TLS_SETUP_ERROR);

	gnutls_certificate_set_dh_params(x509cred, dh_params);
} catch (asio::system_error& se) {
	throw Exception(std::string(TRANSPORT_ERROR) + se.what());
}

void ServerProxy::setup()
{
	// FIXME: should we make the proxy a shared ptr?
	transport_ptr_.reset(new ServerTransport(bind_ip_, bind_port_,
										boost::bind(&ServerProxy::on_transport_connect, this, _1, _2)));

	// start accepting connections
	boost::static_pointer_cast<ServerTransport>(transport_ptr_)->accept();
}

void ServerProxy::on_transport_connect(transport_ptr_t transport_ptr, socket_ptr_t remote_socket_ptr) {
	session_ptr_t session_ptr = setup_tls_session(remote_socket_ptr);
	if (!session_ptr) {
		disconnect_(transport_ptr, session_ptr_t(), socket_ptr_t(), remote_socket_ptr);
		return;
	}
	
	socket_ptr_t local_socket_ptr(new asio::ip::tcp::socket(transport_ptr->io_service()));
	try {
		asio::ip::tcp::resolver resolver(transport_ptr->io_service());
		asio::ip::tcp::resolver::query query("127.0.0.1", boost::lexical_cast<std::string>(local_port_));
		asio::ip::tcp::resolver::iterator iterator(resolver.resolve(query));

		bool connected = false;
		while (iterator != asio::ip::tcp::resolver::iterator())
		{
			try
			{
				local_socket_ptr->connect(*iterator);
				connected = true;
				break;
			}
			catch (asio::system_error /*se*/)
			{
				// make sure we close the socket after a failed attempt, as it
				// may have been opened by the connect() call.
				try { local_socket_ptr->close(); } catch(...) {}
			}
			iterator++;
		}
		if (!connected)
		{
			disconnect_(transport_ptr, session_ptr, local_socket_ptr, remote_socket_ptr);
			return;
		}
	} catch (asio::system_error& /*se*/) {
		disconnect_(transport_ptr, session_ptr, local_socket_ptr, remote_socket_ptr);
		return;
	}

	tunnel(transport_ptr, session_ptr, local_socket_ptr, remote_socket_ptr);
}

session_ptr_t ServerProxy::setup_tls_session(socket_ptr_t remote_socket_ptr) {
	session_ptr_t session_ptr(new gnutls_session_t());

	// setup session
	return_val_if_neg(gnutls_init(session_ptr.get(), GNUTLS_SERVER), session_ptr_t());
	return_val_if_neg(gnutls_set_default_priority(*session_ptr), session_ptr_t());
	return_val_if_neg(gnutls_credentials_set(*session_ptr, GNUTLS_CRD_CERTIFICATE, x509cred), session_ptr_t());
	gnutls_certificate_server_set_request(*session_ptr,GNUTLS_CERT_REQUEST);
	gnutls_dh_set_prime_bits(*session_ptr, 1024);

	// setup ssl transport
	gnutls_transport_set_pull_function(*session_ptr, tls_tunnel::read);
	gnutls_transport_set_push_function(*session_ptr, tls_tunnel::write);
	gnutls_transport_set_ptr(*session_ptr, remote_socket_ptr.get());	

	// execute ssl handshake
	gnutls_certificate_server_set_request(*session_ptr, GNUTLS_CERT_REQUEST);
	return_val_if_neg(gnutls_handshake(*session_ptr), session_ptr_t());

	return session_ptr;
}
	
} /* namespace tls_tunnel */
