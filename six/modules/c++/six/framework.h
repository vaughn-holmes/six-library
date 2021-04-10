#pragma once

// We're building in Visual Studio ... used to control where we get a little bit of config info
#define NITRO_PCH 1

#pragma warning(disable: 4668) // '...' is not defined as a preprocessor macro, replacing with '...' for '...'
#pragma warning(disable: 4820) // '...': '...' bytes padding added after data member '...'
#pragma warning(disable: 4710) // '...': function not inlined

#pragma warning(disable: 5045) // Compiler will insert Spectre mitigation for memory load if / Qspectre switch specified

#pragma warning(disable: 4625) // '...': copy constructor was implicitly defined as deleted
#pragma warning(disable: 4626) // '...': assignment operator was implicitly defined as deleted
#pragma warning(disable: 5026) // '...': move constructor was implicitly defined as deleted
#pragma warning(disable: 5027) //	'...': move assignment operator was implicitly defined as deleted

// TODO: get rid of these someday?
#pragma warning(disable: 4774) // '...' : format string expected in argument 3 is not a string literal
#pragma warning(disable: 4267) // '...': conversion from '...' to '...', possible loss of data
#pragma warning(disable: 4244) // '...': conversion from '...' to '...', possible loss of data
#pragma warning(disable: 4242) // '...': conversion from '...' to '...', possible loss of data
#pragma warning(disable: 4018) // '...': signed / unsigned mismatch
#pragma warning(disable: 4389) // '...': signed / unsigned mismatch
#pragma warning(disable: 4365) // '...': conversion from '...' to '...', signed / unsigned mismatch
#pragma warning(disable: 5219) // implicit conversion from '...' to '...', possible loss of data
#pragma warning(disable: 4514) // '...': unreferenced inline function has been removed

// TODO: get rid of these someday? ... from Visual Studio code-analysis
#pragma warning(disable: 26440) // Function '...' can be declared '...' (f.6).
#pragma warning(disable: 26451) // Arithmetic overflow : Using operator '...' on a 4 byte value and then casting the result to a 8 byte value. Cast the value to the wider type before calling operator '*' to avoid overflow(io.2).
#pragma warning(disable: 6385) // Reading invalid data from '...':  the readable size is '...' bytes, but '...' bytes may be read.
#pragma warning(disable: 6386) // Buffer overrun while writing to '...':  the writable size is '...' bytes, but '...' bytes might be written.
#pragma warning(disable: 26455) // Default constructor may not throw. Declare it 'noexcept' (f.6).
#pragma warning(disable: 26409) // Avoid calling new and delete explicitly, use std::make_unique<T> instead (r.11).
#pragma warning(disable: 26433) // Function '...' should be marked with '...' (c.128).
#pragma warning(disable: 26455) //	 Default constructor may not throw.Declare it 'noexcept' (f.6).
#pragma warning(disable: 26457) // (void) should not be used to ignore return values, use '...' instead(es.48).
#pragma warning(disable: 26488) // Do not dereference a potentially null pointer : '...'. '...' was null at line ... (lifetime.1).
#pragma warning(disable: 26432) // If you define or delete any default operation in the type '...', define or delete them all(c.21).
#pragma warning(disable: 26456) // Operator '...' hides a non - virtual operator '...' (c.128).
#pragma warning(disable: 26481) // Don't use pointer arithmetic. Use span instead (bounds.1).
#pragma warning(disable: 26429) // Symbol '...' is never tested for nullness, it can be marked as not_null(f.23).
#pragma warning(disable: 26446) // Prefer to use gsl::at() instead of unchecked subscript operator (bounds.4).
#pragma warning(disable: 26052) // Potentially unconstrained access using expression '...' ...
#pragma warning(disable: 26400) // Do not assign the result of an allocation or a function call with an owner<T> return value to a raw pointer, use owner<T> instead(i.11).
#pragma warning(disable: 26487) // Don't return a pointer '...' that may be invalid (lifetime.4).
#pragma warning(disable: 26485) // Expression '...': No array to pointer decay(bounds.3).
#pragma warning(disable: 26401) // Do not delete a raw pointer that is not an owner<T>(i.11).
#pragma warning(disable: 26482) // Only index into arrays using constant expressions(bounds.2).
#pragma warning(disable: 26434) // Function '...' hides a non-virtual function '...' (c.128).
#pragma warning(disable: 26494) // Variable '...' is uninitialized.Always initialize an object(type.5).
#pragma warning(disable: 26496) // The variable '...' does not change after construction, mark it as const (con.4).
#pragma warning(disable: 26460) // The reference argument '...' for function '...' can be marked as const (con.3).
#pragma warning(disable: 26461) // The pointer argument '...' for function '...' can be marked as a pointer to const (con.3).
#pragma warning(disable: 26490) //	 Don't use reinterpret_cast (type.1).
#pragma warning(disable: 26435) // Function '...' should specify exactly one of '...', '...', or '...' (c.128).
#pragma warning(disable: 26493) // Don't use C-style casts (type.4).
#pragma warning(disable: 26472) //	 Don't use a static_cast for arithmetic conversions. Use brace initialization, gsl::narrow_cast or gsl::narrow (type.1).
#pragma warning(disable: 26486) //	 Don't pass a pointer that may be invalid to a function. Parameter 5 '...' in call to '...' may be invalid (lifetime.3).
#pragma warning(disable: 26489) // Don't dereference a pointer that may be invalid: '...'. '...' may have been invalidated at line ... (lifetime.1).
#pragma warning(disable: 26474) // Don't cast between pointer types when the conversion could be implicit (type.1).
#pragma warning(disable: 26408) // Avoid malloc() and free(), prefer the nothrow version of new with delete (r.10).
#pragma warning(disable: 26814) // The const variable '... can be computed at compile-time. Consider using constexpr (con.5).
#pragma warning(disable: 26416) //	 Shared pointer parameter '...' is passed by rvalue reference. Pass by value instead(r.34).
#pragma warning(disable: 26812) // The enum type '...' is unscoped. Prefer '...' over '...' (Enum.3).
#pragma warning(disable: 26473) //	 Don't cast between pointer types where the source type and the target type are the same (type.1).
#pragma warning(disable: 26402) // Return a scoped object instead of a heap-allocated if it has a move constructor(r.3).
#pragma warning(disable: 6326) // Potential comparison of a constant with another constant.
#pragma warning(disable: 26414) // Move, copy, reassign or reset a local smart pointer '...' (r.5).
#pragma warning(disable: 26497) // The function '...' could be marked constexpr if compile-time evaluation is desired(f.4).
#pragma warning(disable: 26415) //	 Smart pointer parameter '...' is used only to access contained pointer. Use T * or T & instead(r.30).
#pragma warning(disable: 26447) // The function is declared '...' but calls function '...' which may throw exceptions(f.6).
#pragma warning(disable: 26459) // You called an STL function '...' with a raw pointer parameter at position '...' that may be unsafe-this relies on the caller to check that the passed values are correct. Consider wrapping your range in a gsl::span and pass as a span iterator(stl.1).
#pragma warning(disable: 6262) // Function uses '...' bytes of stack : exceeds analyze : stacksize '...'.Consider moving some data to heap.

#pragma warning(push)
#pragma warning(disable: 4464) // relative include path contains '..'
#include "../../nitro_pch.h"
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 26493) // Don't use C-style casts (type.4).
#pragma warning(disable: 26473) // Don't cast between pointer types where the source type and the target type are the same (type.1).

#include <import/scene.h>
#pragma warning(pop)

#pragma comment(lib, "logging-c++")
#pragma comment(lib, "xml.lite-c++.lib")
#pragma comment(lib, "xerces-c")

