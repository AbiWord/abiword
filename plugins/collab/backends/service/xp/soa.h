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


#ifndef __SOA__
#define __SOA__

#ifdef _MSC_VER
#include "msc_stdint.h"
#else
#include <stdint.h>
#endif
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include "soa_types.h"
#include "soa_result.h"

namespace soa {

class function_arg {
public:
	function_arg(const std::string& n, Type t)
		: name_(n),
		type_(t)
	{}

	virtual ~function_arg() {}

	const std::string& name() const {
		return name_;
	}

	Type type() const {
		return type_;
	}

	virtual bool type_props() const {
		return false;
	}

	virtual std::string props() const {
		return "";
	}

	// FIXME: returning an std::string is inefficient for large blocks of
	// data; we should make it a boost::shared_ptr<std::string>
	virtual std::string str() const = 0;

private:
	std::string name_;
	Type type_;
};

class function_arg_string : public function_arg {
public:
	function_arg_string(const std::string& n, const std::string& value)
		: function_arg(n, STRING_TYPE),
		value_(value)
	{}

	virtual std::string str() const {
		return value_;
	}

private:
	std::string value_;
};

class function_arg_int : public function_arg {
public:
	function_arg_int(const std::string& n, int64_t value)
		: function_arg(n, INT_TYPE),
		value_(value)
	{}

	virtual std::string str() const {
		try {
			return boost::lexical_cast<std::string>(value_);
		} catch (boost::bad_lexical_cast &) {
			return "0";
		}
	}

private:
	int64_t value_;
};

class function_arg_bool : public function_arg {
public:
	function_arg_bool(const std::string& n, bool value)
		: function_arg(n, BOOL_TYPE),
		value_(value)
	{}

	virtual std::string str() const {
		return value_ ? "true" : "false";
	}

private:
	bool value_;
};

class function_arg_base64bin : public function_arg {
public:
	function_arg_base64bin(Base64Bin value)
		: function_arg(value.name(), BASE64BIN_TYPE),
		value_(value)
	{}

	virtual std::string str() const {
		return value_.value();
	}

private:
	Base64Bin value_;
};

class function_arg_array : public function_arg {
public:
	// TODO: automatically deduce the type of the array
	function_arg_array(const std::string& n, ArrayPtr value, Type element_type)
		: function_arg(n, ARRAY_TYPE),
		value_(value),
		element_type_(element_type)
	{}

	virtual bool type_props() const {
		return true;
	}

	virtual std::string props() const {
		if (value_)
			return "SOAP-ENC:arrayType=\"" + soap_type(element_type_) + "[" + boost::lexical_cast<std::string>(value_->size()) + "]\"" + " " + "SOAP-ENC:offset=\"[0]\"";
		return "SOAP-ENC:arrayType=\"xsd:anyType[0]\" xsi:nil=\"true\"";
	}

	virtual std::string str() const {
		std::string ret = "\n";
		if (!value_)
			return ret;

		for (size_t i = 0; i < value_->size(); i++)
		{
			GenericPtr val = value_->operator[](i);
			if (!val)
				continue;

			// TODO: generalize this; for now, we only support arrays of integers
			IntPtr val_int = boost::dynamic_pointer_cast<soa::Int>(val);
			if (!val_int)
				continue;
			function_arg_int arg(val->name(), val_int->value());

			ret += "<" + arg.name() + " " + "xsi:type=\"" + soap_type(arg.type()) + "\"" + ">" + arg.str() + "</" + arg.name() + ">\n";
		}
		return ret;
	}

private:
	ArrayPtr value_;
	Type element_type_;
};

class function_call {
public:
	function_call() {}

	function_call(const std::string& req, const std::string& resp)
		: request_(req),
		response_(resp)
	{}

	function_call& operator()(std::string name, const char* value) {
		args.push_back(boost::shared_ptr<function_arg>(new function_arg_string(name, value)));
		return *this;
	}

	function_call& operator()(std::string name, std::string value) {
		args.push_back(boost::shared_ptr<function_arg>(new function_arg_string(name, value)));
		return *this;
	}

	function_call& operator()(std::string name, int64_t value) {
		args.push_back(boost::shared_ptr<function_arg>(new function_arg_int(name, value)));
		return *this;
	}

	function_call& operator()(std::string name, bool value) {
		args.push_back(boost::shared_ptr<function_arg>(new function_arg_bool(name, value)));
		return *this;
	}

	function_call& operator()(Base64Bin value) {
		args.push_back(boost::shared_ptr<function_arg>(new function_arg_base64bin(value)));
		return *this;
	}

	function_call& operator()(std::string name, ArrayPtr value, Type type) {
		args.push_back(boost::shared_ptr<function_arg>(new function_arg_array(name, value, type)));
		return *this;
	}

	const std::string& request() const {
		return request_;
	}

	const std::string& response() const {
		return response_;
	}

	void add_arg(boost::shared_ptr<function_arg> arg) {
		args.push_back(arg);
	}

	std::string str() const {
		std::string ret;
		// TODO: XML escape args/values
		for (std::vector< boost::shared_ptr<function_arg> >::const_iterator cit = args.begin(); cit != args.end(); cit++) {
			const function_arg& arg = **cit;
			ret += "<" + arg.name() + " " + "xsi:type=\"" + soap_type(arg.type()) + "\"" +
						(arg.type_props() ? " " + arg.props() : "") +
				">" +
				arg.str() +
				"</" + arg.name() + ">\n";
		}
		return ret;
	}

private:
	std::string request_;
	std::string response_;
	std::vector< boost::shared_ptr<function_arg> > args;
};
typedef boost::shared_ptr<soa::function_call> function_call_ptr;

class header {
public:
	std::string str() const {
		return "";
	}
};

class body {
public:
	body(const std::string& ns_ref_)
		: ns_ref(ns_ref_)
	{}

	body(const std::string& ns_ref_, const function_call& fc)
		: ns_ref(ns_ref_),
		fc_(fc)
	{}

	const function_call& function() const {
		return fc_;
	}

	void set_function(function_call fc) {
		fc_ = fc;
	}

	std::string str() const {
		return "<SOAP-ENV:Body>\n" \
			"<" + ns_ref + ":" + fc_.request() + ">\n" +
			fc_.str() +
			"</" + ns_ref + ":" + fc_.request() + ">\n" \
			"</SOAP-ENV:Body>\n";
	}

private:
	std::string ns_ref;
	function_call fc_;
};

class method_invocation {
public:
	method_invocation(const std::string& custom_ns)
		: custom_ns_(custom_ns),
		custom_ns_ref_("nsref"),
		body_(custom_ns_ref_)
	{}

	method_invocation(const std::string& custom_ns, function_call fc)
		: custom_ns_(custom_ns),
		custom_ns_ref_("nsref"),
		body_(custom_ns_ref_, fc)
	{}

	const function_call& function() const {
		return body_.function();
	}

	std::string str() const {
		return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" \
			"<SOAP-ENV:Envelope " +
			default_namespaces() +
			" xmlns:" + custom_ns_ref_ + "=\"" + custom_ns_ + "\"" +
			" " + encoding_style() + ">\n" +
			header_.str() +
			body_.str() +
			"</SOAP-ENV:Envelope>";
	}

private:
	std::string default_namespaces() const {
		return "xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\"" \
			" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"" \
			" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" \
			" xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\"";
	}

	std::string encoding_style() const {
		return "SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"";
	}

	std::string input_name_;
	std::string output_name_;
	std::string custom_ns_;
	std::string custom_ns_ref_;
	header header_;
	body body_;
};
}

#endif /* __SOA__ */
