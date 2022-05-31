#pragma once

#include <std/bit>
#include <std/cstddef>
#include <std/filesystem>
#include <std/memory>
#include <std/optional>
#include <std/span>
#include <std/string>

#pragma warning(disable: 4251) // '...' : class '...' needs to have dll-interface to be used by clients of struct '...'
#pragma warning(push)
#pragma warning(disable: 26447) // The function is declared '...' but calls function '..' which may throw exceptions (f.6).
#pragma warning(disable: 26812) // The enum type '...' is unscoped. Prefer '...' over '...' (Enum.3).
#pragma warning(disable: 26455) //	 Default constructor may not throw.Declare it 'noexcept' (f.6).
#pragma warning(disable: 26458) // Prefer to use gsl::at() instead of unchecked subscript operator (bounds.4).
#pragma warning(disable: 26823) // Dereferencing a possibly null pointer '...' (lifetime.1).
#pragma warning(disable: 26822) // Dereferencing a null pointer 'me' (lifetime.1).

#include <import/str.h>
#include <import/sys.h>
#include <import/io.h>
#include <import/logging.h>

#pragma warning(push)
#pragma warning(disable: 26489) // Don't dereference a pointer that may be invalid: '...'. '...' may have been invalidated at line ... (lifetime.1).

#pragma comment(lib, "xml.lite-c++.lib")
#pragma comment(lib, "xerces-c")

// these are from Xerces
#pragma warning(disable: 5219) // implicit conversion from '...' to '...', possible loss of data
#pragma warning(disable: 4365) // '...': conversion from '...' to '...', signed / unsigned mismatch
#pragma warning(disable: 26477) // Use 'nullptr' rather than 0 or NULL (es.47).
#pragma warning(disable: 26493) // Don't use C-style casts (type.4).
#pragma warning(disable: 26814) // The const variable '...' can be computed at compile-time. Consider using constexpr (con.5).
#pragma warning(disable: 26496) // The variable '...' does not change after construction, mark it as const (con.4).
#pragma warning(disable: 26497) // The function '...' could be marked constexpr if compile-time evaluation is desired (f.4).
#pragma warning(disable: 26475) // Do not use function style casts (es.49). Prefer '...' over '...'.
#pragma warning(disable: 26495) // Variable '...' is uninitialized. Always initialize a member variable (type.6).
#pragma warning(disable: 26461) // The pointer argument '...' for function '...' can be marked as a pointer to const (con.3).
#pragma warning(disable: 26462) // The value pointed to by '...' is assigned only once, mark it as a pointer to const (con.4).
#pragma warning(disable: 26494) // Variable '...' is uninitialized. Always initialize an object (type.5).
#pragma warning(disable: 26489) // Don't dereference a pointer that may be invalid: '...'. '...' may have been invalidated at line ... (lifetime.1).
#pragma warning(disable: 26488) // Do not dereference a potentially null pointer : ...'. '...' was null at line ... (lifetime.1).
#pragma warning(disable: 26447) // The function is declared '...' but calls function '..' which may throw exceptions (f.6).
#pragma warning(disable: 26485) // Expression '...': No array to pointer decay (bounds.3).
#pragma warning(disable: 26457) // (void) should not be used to ignore return values, use '...' instead (es.48).
#pragma warning(disable: 26487) // Don't return a pointer '...' that may be invalid (lifetime.4).
#pragma warning(disable: 26451) // Arithmetic overflow : Using operator '...' on a 4 byte value and then casting the result to a 8 byte value. Cast the value to the wider type before calling operator '...' to avoid overflow (io.2).
#pragma warning(disable: 26455) //	 Default constructor may not throw.Declare it 'noexcept' (f.6).
#pragma warning(disable: 26409) // Avoid calling new and delete explicitly, use std::make_unique<T> instead (r.11).
#pragma warning(disable: 26429) // Symbol '...' is never tested for nullness, it can be marked as not_null(f.23).
#pragma warning(disable: 26400) // Do not assign the result of an allocation or a function call with an owner<T> return value to a raw pointer, use owner<T> instead(i.11).
#pragma warning(disable: 26467) // Converting from floating point to unsigned integral types results in non-portable code if the double/float has a negative value. Use gsl::narrow_cast or gsl::narrow instead to guard against undefined behavior and potential data loss (es.46).
#pragma warning(disable: 26823) // Dereferencing a possibly null pointer '...' (lifetime.1).
#include <import/xml/lite.h>

#pragma warning(pop)
#pragma warning(pop)

