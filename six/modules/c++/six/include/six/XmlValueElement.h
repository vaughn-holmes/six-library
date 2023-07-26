/* =========================================================================
 * This file is part of six-c++
 * =========================================================================
 *
 * (C) Copyright 2004 - 2014, MDA Information Systems LLC
 * (C) Copyright 2023, Maxar Technologies, Inc.
 *
 * six-c++ is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; If not,
 * see <http://www.gnu.org/licenses/>.
 *
 */
#pragma once
#ifndef SIX_six_XmlValueElement_h_INCLUDED_
#define SIX_six_XmlValueElement_h_INCLUDED_

#include <std/string>
#include <ostream>

#include <str/EncodedStringView.h>
#include <xml/lite/Element.h>
#include <xml/lite/QName.h>

#include <six/Types.h>
#include <six/Init.h>
#include <six/Utilities.h>
#include <six/XmlLite.h>

namespace six
{
	// Make it easier to manipulate a Xml Element
	// ```
	// struct Foo final
	// { XmlValueElement<int> myInt {"MyInt"}; };
	// ```
	template<typename T>
	class XmlValueElement final
	{
		std::string name_;
		T value_{};

	public:
		explicit XmlValueElement(const std::string& name) : name_(name) {}
		XmlValueElement(const std::string& name, const T& value) : name_(name), value_(value) {}

		const std::string& name() const { return name_; }

		const T& value() const { return value_; }
		void value(const T& value) { value_ = value; }
	};

	template<typename T, typename U = T>
	inline bool operator==(const XmlValueElement<T>& lhs, const U& rhs)
	{
		return lhs.value() == rhs;
	}
	template<typename T, typename U = T>
	inline bool operator!=(const XmlValueElement<T>& lhs, const U& rhs)
	{
		return !(lhs == rhs);
	}

	template<typename T, typename U = T>
	inline bool operator==(const XmlValueElement<T>& lhs, const XmlValueElement<U>& rhs)
	{
		return (lhs.name() == rhs.name()) && (lhs == rhs.value());
	}
	template<typename T, typename U = T>
	inline bool operator!=(const XmlValueElement<T>& lhs, const XmlValueElement<U>& rhs)
	{
		return !(lhs == rhs);
	}

	template<typename T>
	inline std::ostream& operator<<(std::ostream& os, const XmlValueElement<T>& v)
	{
		os << "\t" << v.name() << "\t: " << v.value() << "\n";
		return os;
	}
	template<>
	inline std::ostream& operator<<(std::ostream& os, const XmlValueElement<std::u8string>& v)
	{
		os << "\t" << v.name() << "\t: " << str::EncodedStringView(v.value()).native() << "\n";
		return os;
	}

	xml::lite::Element& create(XmlLite&, const XmlValueElement<double>&, xml::lite::Element& parent);
	void getFirstAndOnly(const XmlLite&, const xml::lite::Element&, XmlValueElement<double>&);
}

#endif // SIX_six_XmlValueElement_h_INCLUDED_
