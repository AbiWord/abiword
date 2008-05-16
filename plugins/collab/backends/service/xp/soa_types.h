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

#ifndef __SOA_TYPES__
#define __SOA_TYPES__

#include <stdint.h>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace soa {

enum Type {
	ARRAY_TYPE = 0,
	COLLECTION_TYPE,
	STRING_TYPE,
	INT_TYPE,
	BOOL_TYPE,
	BASE64BIN_TYPE,
	QNAME_TYPE
};

class Generic : public boost::enable_shared_from_this<Generic> {
public:
	Generic(const std::string& name, Type type)
		: name_(name),
		type_(type)
	{}

	virtual ~Generic() {
	}

	virtual bool complex() {
		return false;
	}

	const std::string& name() const {
		return name_;
	}

	const Type type() const {
		return type_;
	}

	template <class T>
	boost::shared_ptr<T> as() {
		return boost::dynamic_pointer_cast<T>(shared_from_this()); 
	}

	template <class T>
	boost::shared_ptr<T> as(const std::string& name) {
		if (name_ != name)
			return boost::shared_ptr<T>();
		return boost::dynamic_pointer_cast<T>(shared_from_this()); 
	}

private:
	std::string name_;
	Type type_;
};
typedef boost::shared_ptr<Generic> GenericPtr;

template <class T, Type Y>
class Primitive : public Generic {
public:
	Primitive(const std::string& name, T t)
		: Generic(name, Y),
		value_(t)
	{}

	const T& value() const {
		return value_;
	}

private:
	T value_;
};

typedef Primitive<std::string, STRING_TYPE> String;
typedef boost::shared_ptr<String> StringPtr;

typedef Primitive<int64_t, INT_TYPE> Int;
typedef boost::shared_ptr<Int> IntPtr;

typedef Primitive<bool, BOOL_TYPE> Bool;
typedef boost::shared_ptr<Bool> BoolPtr;

class Base64Bin : public Generic {
public:
	Base64Bin(const std::string& name, boost::shared_ptr<std::string> data)
		: Generic(name, BASE64BIN_TYPE),
		m_data(data)
	{}
	
	const std::string& value() const {
		return *m_data;
	}
	
private:
	boost::shared_ptr<std::string> m_data;
};
typedef boost::shared_ptr<Base64Bin> Base64BinPtr;

typedef Primitive<std::string, QNAME_TYPE> QName; // FIXME: QName's are not simple strings, but they have a Namespace URI, local part and prefix
typedef boost::shared_ptr<QName> QNamePtr;

class Complex : public Generic {
public:
	Complex(const std::string& name, Type t)
		: Generic(name, t)
	{}

	virtual bool complex() {
		return true;
	}
};

template <class T>
class Array : public Complex {
public:
	Array(const std::string& name)
		: Complex(name, ARRAY_TYPE)
	{}

	size_t size() const {
		return values_.size();
	}

	template <class Y>
	boost::shared_ptr< Array< boost::shared_ptr<Y> > > construct() const {
		boost::shared_ptr< Array< boost::shared_ptr<Y> > > conv(new Array< boost::shared_ptr<Y> >(name()));
		for (typename std::vector< T >::const_iterator it = values_.begin(); it != values_.end(); it++) {
			conv->add(Y::construct(*it));
		}
		return conv;
	}

	T get(size_t i) {
		return values_[i];
	}

	T operator[](size_t i) {
		return values_[i];
	}

	virtual void add(T element) {
		values_.push_back(element);
	}

private:
	std::vector< T > values_;
};
typedef boost::shared_ptr< Array<GenericPtr> > ArrayPtr;

class Collection : public Complex {
public:
	Collection(const std::string& name)
		: Complex(name, COLLECTION_TYPE)
	{}

	// TODO: back this by a multimap

	size_t size() const {
		return values_.size();
	}

	template <class T>
	boost::shared_ptr<T> get(const std::string& name) {
		for (std::vector<GenericPtr>::iterator it = values_.begin(); it != values_.end(); it++) {
			if ((*it)->name() == name) {
				return (*it)->as<T>();
			}
		}
		return boost::shared_ptr<T>();
	}

	template <class T>
	boost::shared_ptr<T> operator[](const std::string& name) {
		return get<T>(name);
	}

	virtual void add(GenericPtr element) {
		values_.push_back(element);
	}

	std::vector<GenericPtr>& values() {
		return values_;
	}

private:
	std::vector<GenericPtr> values_;
};
typedef boost::shared_ptr<Collection> CollectionPtr;

class SoapFault {
public:
	SoapFault()
		: code_(), string_(), detail_() {
	}
	
	SoapFault(QNamePtr code, StringPtr string, StringPtr detail)
		: code_(code), string_(string), detail_(detail) {
	}
	
	QNamePtr code() const {
		return code_;
	}
	
	StringPtr string() const {
		return string_;
	}
	
	StringPtr detail() const {
		return detail_;
	}
	
private:
	QNamePtr code_;
	StringPtr string_;
	// TODO: add faultactor actor_;
	StringPtr detail_;
};

}

#endif /* __SOA_TYPES__ */
