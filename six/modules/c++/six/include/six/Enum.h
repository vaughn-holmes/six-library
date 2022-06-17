/* =========================================================================
 * This file is part of six-c++
 * =========================================================================
 *
 * (C) Copyright 2004 - 2014, MDA Information Systems LLC
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
#ifndef SIX_SIX_Enum_h_INCLUDED_
#define SIX_SIX_Enum_h_INCLUDED_
#pragma once

#include <string>
#include <map>
#include <ostream>

#include <scene/sys_Conf.h>
#include <import/except.h>
#include <import/str.h>
#include <import/sys.h>
#include <import/scene.h>
#include <nitf/Enum.hpp>

namespace six
{

constexpr int NOT_SET_VALUE = 2147483647; //std::numeric_limits<int>::max()

namespace details
{
    // Base type for all enums; avoids code duplication
    template<typename T>
    class Enum
    {
        static const std::map<std::string, int>& string_to_int()
        {
            return T::string_to_int_();
        }
        static const std::map<int, std::string>& int_to_string()
        {
            static const auto retval = nitf::details::swap_key_value(string_to_int());
            return retval;
        }

        static int index(const std::string& v)
        {
            const auto result = nitf::details::index(string_to_int(), v);
            const except::InvalidFormatException ex(Ctxt(FmtX("Invalid enum value: %s", v.c_str())));
            return nitf::details::value(result, ex);
        }
        static std::string index(int v)
        {
            const auto result = nitf::details::index(int_to_string(), v);
            const except::InvalidFormatException ex(Ctxt(FmtX("Invalid enum value: %d", v)));
            return nitf::details::value(result, ex);
        }

    protected:
        Enum() = default;

        //! string constructor
        explicit Enum(const std::string& s)
        {
            value = index(s);
        }

        //! int constructor
        Enum(int i)
        {
            (void) index(i);
            value = i;
        }

    public:
        //! Returns string representation of the value
        std::string toString(bool throw_if_not_set = false) const
        {
            if (throw_if_not_set && (value == NOT_SET_VALUE))
            {
                throw except::InvalidFormatException(Ctxt(FmtX("Invalid enum value: %d", value)));
            }
            return index(value);
        }

        static T toType(const std::string& v)
        {
            const auto result = nitf::details::index(string_to_int(), v);
            const except::Exception ex(Ctxt("Unknown type '" + v + "'"));
            return nitf::details::value(result, ex);
        }

        operator int() const { return value; }
        //operator std::string() const { return toString(); }

        // needed for SWIG
        static size_t size() { return int_to_string().size(); }
        bool operator<(const int& o) const { return value < o; }
        bool operator<(const Enum& o) const { return *this < o.value; }
        bool operator==(const int& o) const { return value == o; }
        bool operator==(const Enum& o) const { return *this == o.value; }
        bool operator!=(const int& o) const { return !(*this == o); }
        bool operator!=(const Enum& o) const { return !(*this == o); }
        bool operator<=(const int& o) const { return (*this < o) || (*this == o); }
        bool operator<=(const Enum& o) const { return (*this < o) || (*this == o); }
        bool operator>(const int& o) const { return !(*this <= o); }
        bool operator>(const Enum& o) const { return !(*this <= o); }
        bool operator>=(const int& o) const { return !(*this < o); }
        bool operator>=(const Enum& o) const { return !(*this < o); }

        int value = NOT_SET_VALUE;
    };
    template<typename T>
    inline std::ostream& operator<<(std::ostream& os, const Enum<T>& e)
    {
        os << e.toString();
        return os;
    }


    #define SIX_Enum_map_entry_(n) { #n, n }
    #define SIX_Enum_map_entry_NOT_SET SIX_Enum_map_entry_(NOT_SET), { "NOT SET", NOT_SET }

    // Generate an enum class derived from details::Enum
    // There are a few examples of expanded code below.
    #define SIX_Enum_default_ctor_assign_(name) name() = default; name(const name&) = default; name(name&&) = default; \
            name& operator=(const name&) = default; name& operator=(name&&) = default
    #define SIX_Enum_constructors_(name) SIX_Enum_default_ctor_assign_(name);  explicit name(const std::string& s) { *this = std::move(name::toType(s)); } \
            name(int i) : Enum(i) {} name& operator=(int v) { *this = name(v); return *this; } 
    #define SIX_Enum_BEGIN_enum enum values {
    #define SIX_Enum_BEGIN_DEFINE(name) struct name final : public six::details::Enum<name> { 
    #define SIX_Enum_END_DEFINE(name)  SIX_Enum_constructors_(name); }
    #define SIX_Enum_BEGIN_string_to_int static const std::map<std::string, int>& string_to_int_() { static const std::map<std::string, int> retval {
    #define SIX_Enum_END_enum NOT_SET = six::NOT_SET_VALUE };
    #define SIX_Enum_END_string_to_int SIX_Enum_map_entry_NOT_SET }; return retval; }

    #define SIX_Enum_ENUM_begin_(name) SIX_Enum_BEGIN_DEFINE(name) SIX_Enum_constructors_(name); SIX_Enum_BEGIN_enum
    #define SIX_Enum_ENUM_1_ SIX_Enum_END_enum SIX_Enum_BEGIN_string_to_int
    #define SIX_Enum_ENUM_end_ SIX_Enum_END_string_to_int }

    #define SIX_Enum_map_entry_1_(n) SIX_Enum_map_entry_(n)
    #define SIX_Enum_map_entry_2_(n1, n2)  SIX_Enum_map_entry_(n1), SIX_Enum_map_entry_1_(n2)
    #define SIX_Enum_map_entry_3_(n1, n2, n3)  SIX_Enum_map_entry_(n1), SIX_Enum_map_entry_2_(n2, n3)
    #define SIX_Enum_map_entry_4_(n1, n2, n3, n4)  SIX_Enum_map_entry_(n1), SIX_Enum_map_entry_3_(n2, n3, n4)
    #define SIX_Enum_map_entry_5_(n1, n2, n3, n4, n5)  SIX_Enum_map_entry_(n1), SIX_Enum_map_entry_4_(n2, n3, n4, n5)

    #define SIX_Enum_map_1_(n) SIX_Enum_BEGIN_string_to_int  SIX_Enum_map_entry_1_(n), SIX_Enum_END_string_to_int
    #define SIX_Enum_map_2_(n1, n2) SIX_Enum_BEGIN_string_to_int  SIX_Enum_map_entry_2_(n1, n2), SIX_Enum_END_string_to_int
    #define SIX_Enum_map_3_(n1, n2, n3) SIX_Enum_BEGIN_string_to_int  SIX_Enum_map_entry_3_(n1, n2, n3), SIX_Enum_END_string_to_int
    #define SIX_Enum_map_4_(n1, n2, n3, n4) SIX_Enum_BEGIN_string_to_int  SIX_Enum_map_entry_4_(n1, n2, n3, n4), SIX_Enum_END_string_to_int
    #define SIX_Enum_map_5_(n1, n2, n3, n4, n5) SIX_Enum_BEGIN_string_to_int  SIX_Enum_map_entry_5_(n1, n2, n3, n4, n5), SIX_Enum_END_string_to_int

    #define SIX_Enum_ENUM_1(name, n, v) SIX_Enum_BEGIN_DEFINE(name) \
        SIX_Enum_BEGIN_enum n = v,  SIX_Enum_END_enum SIX_Enum_map_1_(n) SIX_Enum_END_DEFINE(name)
    #define SIX_Enum_ENUM_2(name, n1, v1, n2, v2) SIX_Enum_BEGIN_DEFINE(name) \
        SIX_Enum_BEGIN_enum n1 = v1, n2 = v2,  SIX_Enum_END_enum \
        SIX_Enum_map_2_(n1, n2) SIX_Enum_END_DEFINE(name)
    #define SIX_Enum_ENUM_3(name, n1, v1, n2, v2, n3, v3) SIX_Enum_BEGIN_DEFINE(name) \
        SIX_Enum_BEGIN_enum n1 = v1, n2 = v2,  n3 = v3, SIX_Enum_END_enum \
        SIX_Enum_map_3_(n1, n2, n3) SIX_Enum_END_DEFINE(name)
    #define SIX_Enum_ENUM_4(name, n1, v1, n2, v2, n3, v3, n4, v4) SIX_Enum_BEGIN_DEFINE(name) \
        SIX_Enum_BEGIN_enum n1 = v1, n2 = v2,  n3 = v3, n4 = v4, SIX_Enum_END_enum \
        SIX_Enum_map_4_(n1, n2, n3, n4) SIX_Enum_END_DEFINE(name)
    #define SIX_Enum_ENUM_5(name, n1, v1, n2, v2, n3, v3, n4, v4, n5, v5) SIX_Enum_BEGIN_DEFINE(name) \
        SIX_Enum_BEGIN_enum n1 = v1, n2 = v2,  n3 = v3, n4 = v4, n5 = v5, SIX_Enum_END_enum \
        SIX_Enum_map_5_(n1, n2, n3, n4, n5) SIX_Enum_END_DEFINE(name)


} // namespace details
}
#endif // SIX_SIX_Enum_h_INCLUDED_
