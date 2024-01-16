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

#include <gsl/gsl.h>
#include <math/Utilities.h>
#include <units/Angles.h>
#include <sys/Span.h>
#include <mt/Algorithm.h>

#include "six/sicd/Utilities.h"

#undef min
#undef max

using zfloat = six::zfloat;

#if SIX_sicd_has_VCL

#define VCL_NAMESPACE vcl
#if _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100) // '...': unreferenced formal parameter
#pragma warning(disable: 4127) // conditional expression is constant
#pragma warning(disable: 4244) // '...': conversion from '...' to '...', possible loss of data
#pragma warning(disable: 4723) // potential divide by 0
#endif
#include "six/sicd/vectorclass/version2/vectorclass.h"
#include "six/sicd/vectorclass/version2/vectormath_trig.h"
#include "six/sicd/vectorclass/complex/complexvec1.h"
#if _MSC_VER
#pragma warning(pop)
#endif

using vcl_intv = vcl::Vec4i; 
using vcl_floatv = vcl::Vec4f;
constexpr auto vcl_elements_per_iteration = vcl_floatv::size();

inline int ssize(const vcl_intv& z) noexcept
{
    return z.size();
}
inline int ssize(const vcl_floatv& z) noexcept
{
    return z.size();
}

using vcl_zfloatv = vcl::Complex4f;
auto real(const vcl_zfloatv& z)
{
    return z.real();
}
auto imag(const vcl_zfloatv& z)
{
    return z.imag();
}
inline size_t size(const vcl_zfloatv& z) noexcept
{
    return z.size();
}
inline int ssize(const vcl_zfloatv& z) noexcept
{
    return z.size();
}

inline auto arg(const vcl_zfloatv& z)
{
    // https://en.cppreference.com/w/cpp/numeric/complex/arg
    // > `std::atan2(std::imag(z), std::real(z))`
    return atan2(z.imag(), z.real()); // arg()
}

template<typename T>
inline auto if_add(const T& f, const vcl_floatv& a, float b)
{
    return vcl::if_add(f, a, b);
}

inline bool any_of(const vcl_intv& m)
{
    return horizontal_or(m);
}

inline void copy_from(std::span<const float> p, vcl_floatv& result)
{
    assert(p.size() == result.size());
    result.load(p.data());
}
inline auto copy_from(std::span<const zfloat> p, vcl_zfloatv& result)
{
    // https://en.cppreference.com/w/cpp/numeric/complex
    // > For any pointer to an element of an array of `std::complex<T>` named `p` and any valid array index `i`, ...
    // > is the real part of the complex number `p[i]`, ...
    result.load(reinterpret_cast<const float*>(p.data()));
}

template<size_t N>
inline auto lookup(const vcl_intv& zindex, const std::array<zfloat, N>& phase_directions)
{
    return vcl::lookup<N>(zindex, phase_directions.data());
}
inline auto lookup(const vcl_intv& zindex, std::span<const float> magnitudes)
{
    assert(magnitudes.size() == six::AmplitudeTableSize);
    return vcl::lookup<six::AmplitudeTableSize>(zindex, magnitudes.data());
}

#endif // SIX_sicd_has_VCL

#if SIX_sicd_has_ximd
#ifndef CODA_OSS_Ximd_ENABLE
#define CODA_OSS_Ximd_ENABLE 1
#endif
#include <sys/Ximd.h>

template<typename T>
using ximd = sys::ximd::Ximd<T>;

using ximd_intv = ximd<int>;
using ximd_intv_mask = sys::ximd::ximd_mask;

using ximd_floatv = ximd<float>;
using ximd_floatv_mask = sys::ximd::ximd_mask;

constexpr auto ximd_elements_per_iteration = ximd_floatv::size();

template<typename T>
static inline auto ssize(const ximd<T>& v) noexcept
{
    return gsl::narrow<int>(v.size());
}

// Manage a SIMD complex as an array of two SIMDs
using ximd_zfloatv = std::array<ximd_floatv, 2>;
inline auto& real(ximd_zfloatv& z) noexcept
{
    return z[0];
}
inline const auto& real(const ximd_zfloatv& z) noexcept
{
    return z[0];
}
inline auto& imag(ximd_zfloatv& z) noexcept
{
    return z[1];
}
inline const auto& imag(const ximd_zfloatv& z) noexcept
{
    return z[1];
}
inline size_t size(const ximd_zfloatv& z) noexcept
{
    auto retval = real(z).size();
    assert(retval == imag(z).size());
    return retval;
}
inline auto ssize(const ximd_zfloatv& z) noexcept
{
    return gsl::narrow<int>(size(z));
}

inline auto arg(const ximd_zfloatv& z)
{
    // https://en.cppreference.com/w/cpp/numeric/complex/arg
    // > `std::atan2(std::imag(z), std::real(z))`
    return atan2(imag(z), real(z));  // arg()
}

template <typename TGeneratorReal, typename TGeneratorImag>
static inline auto make_ximd_zfloatv(TGeneratorReal&& generate_real, TGeneratorImag&& generate_imag)
{
    ximd_zfloatv retval;
    for (size_t i = 0; i < size(retval); i++)
    {
        real(retval)[i] = generate_real(i);
        imag(retval)[i] = generate_imag(i);
    }
    return retval;
}

template<typename TTest, typename TResult>
static inline auto ximd_select_(const TTest& test, const TResult& t, const TResult& f)
{
    TResult retval;
    for (size_t i = 0; i < test.size(); i++)
    {
        retval[i] = test[i] ? t[i] : f[i];
    }
    return retval;
}

static inline auto copy_from(std::span<const float> p, ximd_floatv& result)
{
    assert(p.size() == result.size());
    result.copy_from(p.data());
}
static inline auto copy_from(std::span<const zfloat> p, ximd_zfloatv& result)
{
    const auto generate_real = [&](size_t i) { return p[i].real(); };
    const auto generate_imag = [&](size_t i) { return p[i].imag(); };
    result = make_ximd_zfloatv(generate_real, generate_imag);
}

static inline auto roundi(const ximd_floatv& v)  // match vcl::roundi()
{
    const auto rounded = round(v);
    const auto generate_roundi = [&](size_t i)
    { return static_cast<typename ximd_intv::value_type>(rounded[i]); };
    return ximd_intv::generate(generate_roundi);
}

static inline auto select(const ximd_floatv_mask& test, const  ximd_floatv& t, const  ximd_floatv& f)
{
    return ximd_select_(test, t, f);
}
static inline auto select(const ximd_intv_mask& test, const  ximd_intv& t, const  ximd_intv& f)
{
    return ximd_select_(test, t, f);
}

static inline auto if_add(const ximd_floatv_mask& m, const ximd_floatv& v, typename ximd_floatv::value_type c)
{
    // phase = if_add(phase < 0.0, phase, std::numbers::pi_v<float> * 2.0f); // Wrap from [0, 2PI]
    const auto generate_add = [&](size_t i) {
        return m[i] ? v[i] + c : v[i];
    };
    return ximd_floatv::generate(generate_add);
}

template<size_t N>
static inline auto lookup(const ximd_intv& zindex, const std::array<zfloat, N>& phase_directions)
{
    // It seems that the "generator" constuctor is called with SIMD instructions.
    // https://en.cppreference.com/w/cpp/experimental/simd/simd/simd
    // > The calls to `generator` are unsequenced with respect to each other.

    const auto generate_real = [&](size_t i) {
        const auto i_ = zindex[i];
        return phase_directions[i_].real();
    };
    const auto generate_imag = [&](size_t i) {
        const auto i_ = zindex[i];
        return phase_directions[i_].imag();
    };
    return make_ximd_zfloatv(generate_real, generate_imag);
}

static auto lookup(const ximd_intv& zindex, std::span<const float> magnitudes)
{
    const auto generate = [&](size_t i) {
        const auto i_ = zindex[i];

        // The index may be out of range. This is expected because `i` might be "don't care."
        if ((i_ >= 0) && (i_ < std::ssize(magnitudes)))
        {
            return magnitudes[i_];
        }
        return NAN; // propogate "don't care"
    };
    return ximd_floatv::generate(generate);
}

#endif // SIX_sicd_has_ximd

#if SIX_sicd_has_simd

#include <experimental/simd>

// https://en.cppreference.com/w/cpp/experimental/simd/simd_cast
// > The TS specification is missing `simd_cast` and `static_simd_cast` overloads for `simd_mask`. ...
namespace stdx
{
    using namespace std::experimental;
    using namespace std::experimental::__proposed;
}

template<typename T>
using simd = stdx::simd<T>;

using simd_intv = simd<int>;
using simd_intv_mask = typename simd_intv::mask_type;

using simd_floatv = simd<float>;
using simd_floatv_mask = typename simd_floatv::mask_type;

constexpr auto simd_elements_per_iteration = simd_floatv::size();

template<typename T>
static inline auto ssize(const simd<T>& v) noexcept
{
    return gsl::narrow<int>(v.size());
}

// Manage a SIMD complex as an array of two SIMDs
using simd_zfloatv = std::array<simd_floatv, 2>;
inline auto& real(simd_zfloatv& z) noexcept
{
    return z[0];
}
inline const auto& real(const simd_zfloatv& z) noexcept
{
    return z[0];
}
inline auto& imag(simd_zfloatv& z) noexcept
{
    return z[1];
}
inline const auto& imag(const simd_zfloatv& z) noexcept
{
    return z[1];
}
inline size_t size(const simd_zfloatv& z) noexcept
{
    auto retval = real(z).size();
    assert(retval == imag(z).size());
    return retval;
}
inline auto ssize(const simd_zfloatv& z) noexcept
{
    return gsl::narrow<int>(size(z));
}

inline auto arg(const simd_zfloatv& z)
{
    // https://en.cppreference.com/w/cpp/numeric/complex/arg
    // > `std::atan2(std::imag(z), std::real(z))`
    return atan2(imag(z), real(z));  // arg()
}

template <typename TGeneratorReal, typename TGeneratorImag>
static inline auto make_simd_zfloatv(TGeneratorReal&& generate_real, TGeneratorImag&& generate_imag)
{
    simd_zfloatv retval;
    for (size_t i = 0; i < size(retval); i++)
    {
        real(retval)[i] = generate_real(i);
        imag(retval)[i] = generate_imag(i);
    }
    return retval;
}

template<typename TTest, typename TResult>
static inline auto simd_select_(const TTest& test, const TResult& t, const TResult& f)
{
    // https://en.cppreference.com/w/cpp/experimental/simd/where_expression
    // > ... All other elements are left unchanged.
    TResult retval;
    where(test, retval) = t;
    where(!test, retval) = f;
    return retval;
}

static inline auto copy_from(std::span<const float> p, simd_floatv& result)
{
    assert(p.size() == result.size());
    result.copy_from(p.data(), stdx::element_aligned);
}
static inline auto copy_from(std::span<const zfloat> p, simd_zfloatv& result)
{
    const auto generate_real = [&](size_t i) { return p[i].real(); };
    const auto generate_imag = [&](size_t i) { return p[i].imag(); };
    result = make_simd_zfloatv(generate_real, generate_imag);
}

static inline auto roundi(const simd_floatv& v)  // match vcl::roundi()
{
    const auto rounded = round(v);
    const auto generate_roundi = [&](size_t i)
    { return static_cast<typename simd_intv::value_type>(rounded[i]); };
    return simd_intv(generate_roundi);
}

template<typename TMask>
static inline auto select(const TMask& test_, const  simd_floatv& t, const  simd_floatv& f)
{
    //const auto test = test_.__cvt(); // https://github.com/VcDevel/std-simd/issues/41
    const auto test = stdx::static_simd_cast<simd_floatv_mask>(test_); // https://github.com/VcDevel/std-simd/issues/41
    return simd_select_(test, t, f);
}
template<typename TMask>
static inline auto select(const TMask& test_, const  simd_intv& t, const  simd_intv& f)
{
    //const auto test = test_.__cvt(); // https://github.com/VcDevel/std-simd/issues/41
    const auto test = stdx::static_simd_cast<simd_intv_mask>(test_); // https://github.com/VcDevel/std-simd/issues/41
    return simd_select_(test, t, f);
}

static inline auto if_add(const simd_floatv_mask& m, const simd_floatv& v, typename simd_floatv::value_type c)
{
    // phase = if_add(phase < 0.0, phase, std::numbers::pi_v<float> * 2.0f); // Wrap from [0, 2PI]
    const auto generate_add = [&](size_t i) {
        return m[i] ? v[i] + c : v[i];
    };
    return simd_floatv(generate_add);
}

template<size_t N>
static inline auto lookup(const simd_intv& zindex, const std::array<zfloat, N>& phase_directions)
{
    // It seems that the "generator" constuctor is called with SIMD instructions.
    // https://en.cppreference.com/w/cpp/experimental/simd/simd/simd
    // > The calls to `generator` are unsequenced with respect to each other.

    const auto generate_real = [&](size_t i) {
        const auto i_ = zindex[i];
        return phase_directions[i_].real();
    };
    const auto generate_imag = [&](size_t i) {
        const auto i_ = zindex[i];
        return phase_directions[i_].imag();
    };
    return make_simd_zfloatv(generate_real, generate_imag);
}

static auto lookup(const simd_intv& zindex, std::span<const float> magnitudes)
{
    const auto generate = [&](size_t i) {
        const auto i_ = zindex[i];

        // The index may be out of range. This is expected because `i` might be "don't care."
        if ((i_ >= 0) && (i_ < std::ssize(magnitudes)))
        {
            return magnitudes[i_];
        }
        return NAN; // propogate "don't care"
    };
    return simd_floatv(generate);
}

#endif // SIX_sicd_has_simd

/******************************************************************************************************/

template<typename ZFloatV>
static auto getPhase(const ZFloatV& v, float phase_delta)
{
    // Phase is determined via arithmetic because it's equally spaced.
    // There's an intentional conversion to zero when we cast 256 -> uint8. That wrap around
    // handles cases that are close to 2PI.
    auto phase = arg(v);
    phase = if_add(phase < 0.0f, phase, std::numbers::pi_v<float> * 2.0f); // Wrap from [0, 2PI]
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
template<typename IntV, typename FloatV>
inline auto lower_bound_(std::span<const float> magnitudes, const FloatV& v)
{
    IntV first = 0;
    const IntV last = gsl::narrow<int>(magnitudes.size());

    auto count = last - first;
    while (any_of(count > 0))
    {
        auto it = first;
        const auto step = count / 2;
        it += step;

        auto next = it; ++next; // ... ++it;
        auto advance = count; advance -= step + 1;  // ...  -= step + 1;

        const auto c = lookup(it, magnitudes); // magnituides[it]
        const auto test = c < v;
        it = select(test, next, it); // ... ++it
        first = select(test, it, first); // first = ...
        count = select(test, advance, step); // count -= step + 1 <...OR...> count = step
    }
    return first;
}
template<typename IntV, typename FloatV>
inline auto lower_bound(std::span<const float> magnitudes, const FloatV& value)
{
    auto retval = lower_bound_<IntV>(magnitudes, value);

    #if CODA_OSS_DEBUG
    for (int i = 0; i < ssize(value); i++)
    {
        const auto it = std::lower_bound(magnitudes.begin(), magnitudes.end(), value[i]);
        const auto result = gsl::narrow<int>(std::distance(magnitudes.begin(), it));
        assert(retval[i] == result);
    }
    #endif

    return retval;
}

template<typename IntV, typename FloatV>
static auto nearest(std::span<const float> magnitudes, const FloatV& value)
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
    const auto it = ::lower_bound<IntV>(magnitudes, value);
    const auto prev_it = it - 1; // const auto prev_it = std::prev(it);

    const auto v0 = value - lookup(prev_it, magnitudes); // value - *prev_it
    const auto v1 = lookup(it, magnitudes) - value; // *it - value
    //const auto nearest_it = select(v0 <= v1, prev_it, it); //  (value - *prev_it <= *it - value ? prev_it : it);
    
    const IntV end = gsl::narrow<int>(magnitudes.size());
    //const auto end_test = select(it == end, prev_it, nearest_it); // it == end ? prev_it  : ...
    const IntV zero = 0;
    auto retval = select(it == 0, zero, // if (it == begin) return 0;
        select(it == end, prev_it,  // it == end ? prev_it  : ...
            select(v0 <= v1, prev_it, it) //  (value - *prev_it <= *it - value ? prev_it : it);
        ));
    return retval;
}

template<typename IntV, typename FloatV, typename ZFloatV>
static auto find_nearest(std::span<const float> magnitudes,
    const FloatV& phase_direction_real, const FloatV& phase_direction_imag,
    const ZFloatV& v)
{
    // We have to do a 1D nearest neighbor search for magnitude.
    // But it's not the magnitude of the input complex value - it's the projection of
    // the complex value onto the ray of candidate magnitudes at the selected phase.
    // i.e. dot product.
    const auto projection = (phase_direction_real * real(v)) + (phase_direction_imag * imag(v));
    //assert(std::abs(projection - std::abs(v)) < 1e-5); // TODO ???
    return nearest<IntV>(magnitudes, projection);
}

#if SIX_sicd_has_VCL
static auto lookup_and_find_nearest(const six::sicd::details::ComplexToAMP8IPHS8I& converter,
    const vcl_intv& phase, const  vcl_zfloatv& v)
{
    const auto& impl = converter.impl;

    const auto phase_direction_real = lookup(phase, impl.phase_directions_real);
    const auto phase_direction_imag = lookup(phase, impl.phase_directions_imag);
    return ::find_nearest<vcl_intv>(impl.magnitudes, phase_direction_real, phase_direction_imag, v);
}
#endif

#if SIX_sicd_has_ximd
static auto lookup_and_find_nearest(const six::sicd::details::ComplexToAMP8IPHS8I& converter,
    const ximd_intv& phase, const  ximd_zfloatv& v)
{
    const auto& impl = converter.impl;

    const auto phase_direction = lookup(phase, impl.phase_directions);
    return ::find_nearest<ximd_intv>(impl.magnitudes, real(phase_direction), imag(phase_direction), v);
}
#endif

#if SIX_sicd_has_simd
static auto lookup_and_find_nearest(const six::sicd::details::ComplexToAMP8IPHS8I& converter,
    const simd_intv& phase, const  simd_zfloatv& v)
{
    const auto& impl = converter.impl;

    const auto phase_direction = lookup(phase, impl.phase_directions);
    return ::find_nearest<simd_intv>(impl.magnitudes, real(phase_direction), imag(phase_direction), v);
}
#endif

template<typename ZFloatV>
void six::sicd::details::ComplexToAMP8IPHS8I::Impl::nearest_neighbors_unseq_T(std::span<const six::zfloat> p, std::span<AMP8I_PHS8I_t> results) const
{
    ZFloatV v;
    assert(p.size() == size(v));

    copy_from(p, v);
    #if CODA_OSS_DEBUG
    for (int i = 0; i < ssize(v); i++)
    {
        //const auto z = p[i];
        //assert(real(v)[i] == z.real());
        //assert(imag(v)[i] == z.imag());
    }
    #endif

    const auto phase = ::getPhase(v, phase_delta);
    #if CODA_OSS_DEBUG
    for (int i = 0; i < ssize(phase); i++)
    {
        const auto phase_ = getPhase(p[i]);
        assert(static_cast<uint8_t>(phase[i]) == phase_);
    }
    #endif

    const auto amplitude = lookup_and_find_nearest(converter, phase, v);
    #if CODA_OSS_DEBUG
    for (int i = 0; i < ssize(amplitude); i++)
    {
        const auto i_ = phase[i];
        const auto a = find_nearest(phase_directions[i_], p[i]);
        assert(a == amplitude[i]);
    }
    #endif

    // interleave() and store() is slower than an explicit loop.
    auto dest = results.begin();
    for (int i = 0; i < ssize(v); i++)
    {
        dest->phase = gsl::narrow_cast<uint8_t>(phase[i]);
        dest->amplitude = gsl::narrow_cast<uint8_t>(amplitude[i]);

        ++dest;
    }
}

template<typename ZFloatV, int elements_per_iteration>
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

    // Can do these checks one-time outside of the loop
    const auto distance = std::distance(first, last);

    // First, do multiples of 8
    const auto distance_ = distance - (distance % elements_per_iteration);
    const auto last_ = first + distance_;
    for (; first != last_; first += elements_per_iteration, dest += elements_per_iteration)
    {
        const auto f = sys::make_span(&(*first), elements_per_iteration);
        const auto d = sys::make_span(&(*dest), elements_per_iteration);

        nearest_neighbors_unseq_T<ZFloatV>(f, d);
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

#if SIX_sicd_has_VCL
std::vector<six::AMP8I_PHS8I_t> six::sicd::details::ComplexToAMP8IPHS8I::nearest_neighbors_unseq_vcl(
    std::span<const zfloat> inputs, const six::AmplitudeTable* pAmplitudeTable)
{
    // make a structure to quickly find the nearest neighbor
    const auto& converter = make_(pAmplitudeTable);

    std::vector<AMP8I_PHS8I_t> retval(inputs.size());
    converter.impl.nearest_neighbors_unseq<vcl_zfloatv, vcl_elements_per_iteration>(inputs, sys::make_span(retval));
    return retval;
}
#endif // SIX_sicd_has_VCL

#if SIX_sicd_has_ximd
std::vector<six::AMP8I_PHS8I_t> six::sicd::details::ComplexToAMP8IPHS8I::nearest_neighbors_unseq_ximd(
    std::span<const zfloat> inputs, const six::AmplitudeTable* pAmplitudeTable)
{
    // make a structure to quickly find the nearest neighbor
    const auto& converter = make_(pAmplitudeTable);

    std::vector<AMP8I_PHS8I_t> retval(inputs.size());
    converter.impl.nearest_neighbors_unseq<ximd_zfloatv, ximd_elements_per_iteration>(inputs, sys::make_span(retval));
    return retval;
}
#endif // SIX_sicd_has_ximd

#if SIX_sicd_has_simd
std::vector<AMP8I_PHS8I_t> six::sicd::details::ComplexToAMP8IPHS8I::nearest_neighbors_unseq_simd(
    std::span<const zfloat> inputs, const six::AmplitudeTable* pAmplitudeTable)
{
    // make a structure to quickly find the nearest neighbor
    const auto& converter = make_(pAmplitudeTable);

    std::vector<AMP8I_PHS8I_t> retval(inputs.size());
    converter.impl.nearest_neighbors_unseq<simd_zfloatv, simd_elements_per_iteration>(inputs, sys::make_span(retval));
    return retval;
}
#endif // SIX_sicd_has_simd
