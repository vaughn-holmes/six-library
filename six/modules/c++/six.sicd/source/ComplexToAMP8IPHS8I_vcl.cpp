/* =========================================================================
* This file is part of six.sicd-c++
* =========================================================================
*
* (C) Copyright 2021, Maxar Technologies, Inc.
*
* six.sicd-c++ is free software; you can redistribute it and/or modify
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
#include "six/AmplitudeTable.h"

#include <math.h>
#include <assert.h>

#include <cassert>
#include <memory>
#include <std/numbers>
#include <algorithm>
#include <functional>
#include <type_traits>

#include <coda_oss/CPlusPlus.h>
#if CODA_OSS_cpp17
    // <execution> is broken with the older version of GCC we're using
    #if (__GNUC__ >= 10) || _MSC_VER
    #include <execution>
    #define SIX_six_sicd_ComplexToAMP8IPHS8I_has_execution 1
    #endif
#endif

#include <gsl/gsl.h>
#include <math/Utilities.h>
#include <units/Angles.h>
#include <sys/Span.h>

#include "six/sicd/Utilities.h"

#undef min
#undef max

#if SIX_sicd_has_VCL

#define VCL_NAMESPACE vcl
#if _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100) // '...': unreferenced formal parameter
#pragma warning(disable: 4127) // conditional expression is constant
#pragma warning(disable: 4244) // '...': conversion from '...' to '...', possible loss of data
#endif
#include "six/sicd/vectorclass/version2/vectorclass.h"
#include "six/sicd/vectorclass/version2/vectormath_trig.h"
#include "six/sicd/vectorclass/complex/complexvec1.h"
#if _MSC_VER
#pragma warning(pop)
#endif

// https://en.cppreference.com/w/cpp/experimental/simd
using zfloatv = vcl::Complex8f;
using floatv = vcl::Vec8f;
using intv = vcl::Vec8i;

auto real(const zfloatv& z)
{
    return z.real();
}
auto imag(const zfloatv& z)
{
    return z.imag();
}

static inline auto load(std::span<const six::zfloat> p)
{
    // https://en.cppreference.com/w/cpp/numeric/complex
    // > For any pointer to an element of an array of `std::complex<T>` named `p` and any valid array index `i`, ...
    // > is the real part of the complex number `p[i]`, ...
    zfloatv retval;
    retval.load(reinterpret_cast<const float*>(p.data()));
    return retval;
}

// https://en.cppreference.com/w/cpp/numeric/complex/arg
// > `std::atan2(std::imag(z), std::real(z))`
inline auto arg(const floatv& real, const floatv& imag)
{
    return atan2(imag, real); // arg()
}
inline auto arg(const zfloatv& z)
{
    return arg(real(z), imag(z));
}

static auto getPhase(const zfloatv& v, float phase_delta)
{
    // Phase is determined via arithmetic because it's equally spaced.
    // There's an intentional conversion to zero when we cast 256 -> uint8. That wrap around
    // handles cases that are close to 2PI.
    auto phase = arg(v);
    phase = if_add(phase < 0.0, phase, std::numbers::pi_v<float> * 2.0f); // Wrap from [0, 2PI]
    return roundi(phase / phase_delta);
}

// https://en.cppreference.com/w/cpp/algorithm/lower_bound
/*
template<class ForwardIt, class T>
ForwardIt lower_bound(ForwardIt first, ForwardIt last, const T& value)
{
    ForwardIt it;
    typename std::iterator_traits<ForwardIt>::difference_type count, step;
    count = std::distance(first, last);

    while (count > 0)
    {
        it = first;
        step = count / 2;
        std::advance(it, step);

        if (*it < value)
        {
            first = ++it;
            count -= step + 1;
        }
        else
            count = step;
    }

    return first;
}
*/
inline auto lower_bound_(std::span<const float> magnitudes, const floatv& v)
{
    intv first = 0;
    const intv last = gsl::narrow<int>(magnitudes.size());

    auto count = last - first;
    while (horizontal_or(count > 0))
    {
        auto it = first;
        const auto step = count / 2;
        it += step;

        auto next = it; ++next; // ... ++it;
        auto advance = count; advance -= step + 1;  // ...  -= step + 1;

        const auto c = vcl::lookup<six::AmplitudeTableSize>(it, magnitudes.data()); // magnituides[it]
        const auto test = c < v;
        it = select(test, next, it); // ... ++it
        first = select(test, it, first); // first = ...
        count = select(test, advance, step); // count -= step + 1 <...OR...> count = step
    }
    return first;
}
inline auto lower_bound(std::span<const float> magnitudes, const floatv& value)
{
    auto retval = lower_bound_(magnitudes, value);

    #ifndef NDEBUG // i.e., debug
    for (int i = 0; i < value.size(); i++)
    {
        const auto it = std::lower_bound(magnitudes.begin(), magnitudes.end(), value[i]);
        const auto result = gsl::narrow<int>(std::distance(magnitudes.begin(), it));
        assert(retval[i] == result);
    }
    #endif

    return retval;
}

static auto nearest(std::span<const float> magnitudes, const floatv& value)
{
    assert(magnitudes.size() == six::AmplitudeTableSize);

    /*
        const auto it = std::lower_bound(begin, end, value);
        if (it == begin) return 0;

        const auto prev_it = std::prev(it);
        const auto nearest_it = it == end ? prev_it  :
            (value - *prev_it <= *it - value ? prev_it : it);
        const auto distance = std::distance(begin, nearest_it);
        assert(distance <= std::numeric_limits<uint8_t>::max());
        return gsl::narrow<uint8_t>(distance);
    */
    const auto it = ::lower_bound(magnitudes, value);
    const auto prev_it = it - 1; // const auto prev_it = std::prev(it);

    const auto v0 = value - vcl::lookup<six::AmplitudeTableSize>(prev_it, magnitudes.data()); // value - *prev_it
    const auto v1 = vcl::lookup<six::AmplitudeTableSize>(it, magnitudes.data()) - value; // *it - value
    //const auto nearest_it = select(v0 <= v1, prev_it, it); //  (value - *prev_it <= *it - value ? prev_it : it);
    
    const intv end = gsl::narrow<int>(magnitudes.size());
    //const auto end_test = select(it == end, prev_it, nearest_it); // it == end ? prev_it  : ...
    const intv zero = 0;
    auto retval = select(it == 0, zero, // if (it == begin) return 0;
        select(it == end, prev_it,  // it == end ? prev_it  : ...
            select(v0 <=v1, prev_it, it) //  (value - *prev_it <= *it - value ? prev_it : it);
        ));
    return retval;
}

static auto find_nearest(std::span<const float> magnitudes, const floatv& phase_direction_real, const floatv& phase_direction_imag,
    const zfloatv& v)
{
    // We have to do a 1D nearest neighbor search for magnitude.
    // But it's not the magnitude of the input complex value - it's the projection of
    // the complex value onto the ray of candidate magnitudes at the selected phase.
    // i.e. dot product.
    const auto projection = (phase_direction_real * real(v)) + (phase_direction_imag * imag(v));
    //assert(std::abs(projection - std::abs(v)) < 1e-5); // TODO ???
    return nearest(magnitudes, projection);
}
static inline auto find_nearest(std::span<const float> magnitudes, const zfloatv& phase_direction, const zfloatv& v)
{
    return find_nearest(magnitudes, phase_direction.real(), phase_direction.imag(), v);
}

void six::sicd::details::ComplexToAMP8IPHS8I::Impl::nearest_neighbors_unseq_(std::span<const six::zfloat> p, std::span<AMP8I_PHS8I_t> results) const
{
    const auto v = load(p);

    const auto phase = ::getPhase(v, phase_delta);

    const auto phase_direction_real = vcl::lookup<six::AmplitudeTableSize>(phase, phase_directions_real.data());
    const auto phase_direction_imag = vcl::lookup<six::AmplitudeTableSize>(phase, phase_directions_imag.data());
    const auto amplitude = ::find_nearest(magnitudes, phase_direction_real, phase_direction_imag, v);
    #ifndef NDEBUG // i.e., debug
    for (int i = 0; i < amplitude.size(); i++)
    {
        const auto i_ = phase[i];
        const auto a = find_nearest(phase_directions[i_], p[i]);
        assert(a == amplitude[i]);
    }
    #endif

    // interleave() and store() is slower than an explicit loop.
    auto dest = results.begin();
    for (int i = 0; i < v.size(); i++)
    {
        dest->phase = gsl::narrow_cast<uint8_t>(phase[i]);
        dest->amplitude = gsl::narrow_cast<uint8_t>(amplitude[i]);

        ++dest;
    }
}

void six::sicd::details::ComplexToAMP8IPHS8I::Impl::nearest_neighbors_unseq(std::span<const six::zfloat> inputs, std::span<AMP8I_PHS8I_t> results) const
{
    auto first = inputs.begin();
    const auto last = inputs.end();
    auto dest = results.begin();

    // The above code is simpler (no templates) if we use just a single VCL
    // complex type: `zfloatv`.  If there is any performance difference,
    // it will only be for extreme edge cases since the smaller types are only used
    // at the end of the loop.
    //
    // It also makes this loop simpler as we handle all non-multiples-of-8 in
    // the same way.
    constexpr auto elements_per_iteration = 8;

    // Can do these checks one-time outside of the loop
    const auto distance = std::distance(first, last);

    // First, do multiples of 8
    const auto distance_ = distance - (distance % elements_per_iteration);
    const auto last_ = first + distance_;
    for (; first != last_; first += elements_per_iteration, dest += elements_per_iteration)
    {
        const auto f = sys::make_span(&(*first), elements_per_iteration);
        const auto d = sys::make_span(&(*dest), elements_per_iteration);
        nearest_neighbors_unseq_(f, d);
    }

    // Then finish off anything left
    assert(std::distance(first, last) < elements_per_iteration);
    for (; first != last; ++first, ++dest)
    {
        const auto f = sys::make_span(&(*first), 1);
        const auto d = sys::make_span(&(*dest), 1);
        nearest_neighbors_seq(f, d);
    }
}
std::vector<six::AMP8I_PHS8I_t> six::sicd::details::ComplexToAMP8IPHS8I::nearest_neighbors_unseq_vcl(
    std::span<const zfloat> inputs, const six::AmplitudeTable* pAmplitudeTable)
{
    // make a structure to quickly find the nearest neighbor
    const auto& converter = make_(pAmplitudeTable);

    std::vector<six::AMP8I_PHS8I_t> retval(inputs.size());
    converter.impl.nearest_neighbors_unseq(inputs, sys::make_span(retval));
    return retval;
}

/**********************************************************************

// This is here (instead of **ComplexToAMP8IPHS8I.cpp**) because par_unseq() might
// need to know implementation details of _unseq()
using input_it = std::span<const six::zfloat>::iterator;
using output_it = std::span<six::AMP8I_PHS8I_t>::iterator;

struct const_iterator final
{
    using Type = input_it::value_type;

    using iterator_category = std::random_access_iterator_tag;
    using value_type = std::remove_cv_t<Type>;
    using difference_type = std::ptrdiff_t;
    using pointer = Type*;
    using reference = Type&;
    using const_reference = const Type&;

    input_it current_;

    const_iterator() = default;
    const_iterator(input_it it) : current_(it) {}

    const_reference operator*() const noexcept
    {
        return *current_;
    }

    const_iterator& operator++() noexcept
    {
        for (ptrdiff_t i = 0; i < 8; i++)
        {
            ++current_;
        }
        return *this;
    }

    const_iterator& operator--() noexcept
    {
        --current_;
        return *this;
    }

    const_iterator& operator-=(const difference_type n) noexcept
    {
        current_ -= n;
        return *this;
    }
    const_iterator operator-(const difference_type n) const noexcept
    {
        auto ret = *this;
        ret -= n;
        return ret;
    }
    difference_type operator-(const const_iterator& rhs) const noexcept
    {
        return current_ - rhs.current_;
    }

    bool operator!=(const const_iterator& rhs) const noexcept
    {
        return current_ != rhs.current_;
    }
};

struct result_wrapper final
{
    output_it current_;

    result_wrapper& operator=(const std::vector<six::AMP8I_PHS8I_t>& values)
    {
        for (auto& v : values)
        {
            *current_ = v;
            ++current_;
        }
        return *this;
    }
};

struct iterator final
{
    using Type = output_it::value_type;

    using iterator_category = std::random_access_iterator_tag;
    using value_type = std::remove_cv_t<Type>;
    using difference_type = std::ptrdiff_t;
    using pointer = Type*;
    using reference = Type&;

    output_it current_;

    iterator() = default;
    iterator(output_it it) : current_(it) {}

    result_wrapper operator*() noexcept
    {
        return result_wrapper{ current_};
    }

    iterator& operator++() noexcept
    {
        for (ptrdiff_t i = 0; i < 8; i++)
        {
            ++current_;
        }
        return *this;
    }

    iterator& operator--() noexcept
    {
        --current_;
        return *this;
    }

    iterator& operator-=(const difference_type n) noexcept
    {
        current_ -= n;
        return *this;
    }

    bool operator!=(const iterator& rhs) const noexcept
    {
        return current_ != rhs.current_;
    }
};

void six::sicd::details::ComplexToAMP8IPHS8I::Impl::nearest_neighbors_par_unseq(std::span<const six::zfloat> inputs, std::span<AMP8I_PHS8I_t> results) const
{
    const auto first = inputs.begin();
    const auto last = inputs.end();
    auto dest = results.begin();

    const auto func = [&](const auto& v) {
        std::span<const six::zfloat> values(&v, 8);

        std::vector<six::AMP8I_PHS8I_t> retval(values.size());
        nearest_neighbors_unseq(values, sys::make_span(retval));
        return retval;
    };

    std::ignore = std::transform(std::execution::seq,
        const_iterator{ first }, const_iterator{ last }, iterator{ dest }, func);
}
#if SIX_sicd_ComplexToAMP8IPHS8I_unseq
std::vector<six::AMP8I_PHS8I_t> six::sicd::details::ComplexToAMP8IPHS8I::nearest_neighbors_par_unseq(
    std::span<const zfloat> inputs, const six::AmplitudeTable* pAmplitudeTable)
{
    // make a structure to quickly find the nearest neighbor
    const auto& converter = make_(pAmplitudeTable);

    std::vector<six::AMP8I_PHS8I_t> retval(inputs.size());
    converter.impl.nearest_neighbors_par_unseq(inputs, sys::make_span(retval));
    return retval;
}
#endif //  SIX_sicd_ComplexToAMP8IPHS8I_unseq

**********************************************************************/

#endif // SIX_sicd_have_VCL