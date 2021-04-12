#pragma once

// We're building in Visual Studio ... used to control where we get a little bit of config info
#define NITRO_PCH 1

#pragma warning(disable: 4668) // '...' is not defined as a preprocessor macro, replacing with '...' for '...'
#pragma warning(disable: 4820) // '...': '...' bytes padding added after data member '...'
#pragma warning(disable: 4710) // '...': function not inlined
#pragma warning(disable: 5045) // Compiler will insert Spectre mitigation for memory load if / Qspectre switch specified
#pragma warning(disable: 4514) // '...': unreferenced inline function has been removed

#pragma warning(disable: 26812) // The enum type '...' is unscoped. Prefer '...' over '...' (Enum.3).

// TODO: get rid of these someday?
#pragma warning(disable: 4774) // '...' : format string expected in argument 3 is not a string literal
#pragma warning(disable: 4267) // '...': conversion from '...' to '...', possible loss of data
#pragma warning(disable: 4244) // '...': conversion from '...' to '...', possible loss of data
#pragma warning(disable: 4242) // '...': conversion from '...' to '...', possible loss of data
#pragma warning(disable: 4018) // '...': signed / unsigned mismatch
#pragma warning(disable: 4389) // '...': signed / unsigned mismatch
#pragma warning(disable: 4365) // '...': conversion from '...' to '...', signed / unsigned mismatch
#pragma warning(disable: 5219) // implicit conversion from '...' to '...', possible loss of data

// TODO: get rid of these someday? High(?) priority ... from Visual Studio code-analysis
#pragma warning(disable: 26487) // Don't return a pointer '...' that may be invalid (lifetime.4).
#pragma warning(disable: 26451) // Arithmetic overflow : Using operator '...' on a 4 byte value and then casting the result to a 8 byte value. Cast the value to the wider type before calling operator '*' to avoid overflow(io.2).
#pragma warning(disable: 26400) // Do not assign the result of an allocation or a function call with an owner<T> return value to a raw pointer, use owner<T> instead(i.11).
#pragma warning(disable: 26409) // Avoid calling new and delete explicitly, use std::make_unique<T> instead (r.11).
#pragma warning(disable: 26429) // Symbol '...' is never tested for nullness, it can be marked as not_null(f.23).
#pragma warning(disable: 26486) //	 Don't pass a pointer that may be invalid to a function. Parameter ... '...' in call to '...' may be invalid (lifetime.3).
#pragma warning(disable: 26489) // Don't dereference a pointer that may be invalid: '...'. '...' may have been invalidated at line ... (lifetime.1).
#pragma warning(disable: 26402) // Return a scoped object instead of a heap-allocated if it has a move constructor(r.3).
#pragma warning(disable: 26414) // Move, copy, reassign or reset a local smart pointer '...' (r.5).
#pragma warning(disable: 26472) //	 Don't use a static_cast for arithmetic conversions. Use brace initialization, gsl::narrow_cast or gsl::narrow (type.1).

// TODO: get rid of these someday? ... from Visual Studio code-analysis
#pragma warning(disable: 26440) // Function '...' can be declared '...' (f.6).
#pragma warning(disable: 26455) // Default constructor may not throw. Declare it 'noexcept' (f.6).
#pragma warning(disable: 26433) // Function '...' should be marked with '...' (c.128).
#pragma warning(disable: 26457) // (void) should not be used to ignore return values, use '...' instead(es.48).
#pragma warning(disable: 26456) // Operator '...' hides a non - virtual operator '...' (c.128).
#pragma warning(disable: 26481) // Don't use pointer arithmetic. Use span instead (bounds.1).
#pragma warning(disable: 26446) // Prefer to use gsl::at() instead of unchecked subscript operator (bounds.4).
#pragma warning(disable: 26485) // Expression '...': No array to pointer decay(bounds.3).
#pragma warning(disable: 26401) // Do not delete a raw pointer that is not an owner<T>(i.11).
#pragma warning(disable: 26482) // Only index into arrays using constant expressions(bounds.2).
#pragma warning(disable: 26434) // Function '...' hides a non-virtual function '...' (c.128).
#pragma warning(disable: 26435) // Function '...' should specify exactly one of '...', '...', or '...' (c.128).
#pragma warning(disable: 26408) // Avoid malloc() and free(), prefer the nothrow version of new with delete (r.10).
#pragma warning(disable: 26447) // The function is declared '...' but calls function '...' which may throw exceptions(f.6).
#pragma warning(disable: 6262) // Function uses '...' bytes of stack : exceeds analyze : stacksize '...'.Consider moving some data to heap.
#pragma warning(disable: 26459) // You called an STL function '...' with a raw pointer parameter at position '...' that may be unsafe-this relies on the caller to check that the passed values are correct. Consider wrapping your range in a gsl::span and pass as a span iterator(stl.1).
#pragma warning(disable: 26432) // If you define or delete any default operation in the type '...', define or delete them all(c.21).


// too hard to get write with older C++11 compilers
#pragma warning(disable: 26497) // The function '...' could be marked constexpr if compile-time evaluation is desired(f.4).


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

