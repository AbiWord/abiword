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


#ifndef __SOA_SOUP__
#define __SOA_SOUP__

#include <string>
#include <libsoup/soup.h>
#include <boost/function.hpp>
#include "soa.h"

namespace soup_soa {
	
	soa::GenericPtr invoke(const std::string& url, const soa::method_invocation& mi, const std::string& ssl_ca_file);
	
	soa::GenericPtr invoke(const std::string& url, const soa::method_invocation& mi, const std::string& ssl_ca_file,
						   boost::function<void (SoupSession*, SoupMessage*, uint32_t)> progress_cb);

	bool invoke(const std::string& url, const soa::method_invocation& mi, const std::string& ssl_ca_file, std::string& result);

	bool invoke(const std::string& url, const soa::method_invocation& mi, const std::string& ssl_ca_file,
						   boost::function<void (SoupSession*, SoupMessage*, uint32_t)> progress_cb, std::string& result);
}

#endif /* __SOA_SOUP__ */
