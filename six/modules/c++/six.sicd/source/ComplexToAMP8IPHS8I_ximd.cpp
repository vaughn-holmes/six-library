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
#include <array>
#include <std/span>

#include <gsl/gsl.h>
#include <math/Utilities.h>
#include <units/Angles.h>
#include <sys/Span.h>

#include "six/sicd/Utilities.h"

#if SIX_sicd_has_ximd

// Make a hacked-up "fake" SIMD implementation for development/testing
// https://en.cppreference.com/w/cpp/experimental/simd
namespace ximd
{
    constexpr size_t native_size = 4;

    template<typename T>
    struct simd final
    {
        using value_type = T;
        using reference = T&;
        using const_reference = const T&;
        std::array<value_type, native_size> value{};
        
        simd() = default;
        simd(const value_type& v) noexcept
        {
            *this = simd([&](size_t) { return v; });
        }
        template<typename U>
        simd(const simd<U>&  other) noexcept
        {
            *this = other;
        }

        // https://en.cppreference.com/w/cpp/experimental/simd/simd/simd
        template<typename G>
        explicit simd(G&& generator) noexcept
        {
            for (size_t i = 0; i < value.size(); i++)
            {
                value[i] = generator(i);
            }
        }

        reference operator[](size_t pos) noexcept
        {
            return value[pos];
        }
        const_reference operator[](size_t pos) const noexcept
        {
            return value[pos];
        }

        constexpr auto size() const noexcept
        {
            return value.size();
        }

        simd& operator++() noexcept
        {
            *this = simd([&](size_t i) { return ++value[i]; });
            return *this;
        }
        simd operator++(int) noexcept
        {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }
    };

    using floatv = simd<float>;
    using intv = simd<int>;

    using simd_mask = simd<bool>;
}
template<typename T>
inline auto operator/(const ximd::simd<T>& lhs, const ximd::simd<T>& rhs)
{
    return ximd::simd<T>([&](size_t i) { return lhs[i] / rhs[i]; });
}
template<typename T>
inline auto operator/(const ximd::simd<T>& lhs, const typename ximd::simd<T>::value_type& rhs)
{
    return lhs / ximd::simd<T>(rhs);
}

template<typename T>
inline auto operator*(const ximd::simd<T>& lhs, const ximd::simd<T>& rhs)
{
    return ximd::simd<T>([&](size_t i) { return lhs[i] * rhs[i]; });
}
template<typename T>
inline auto operator*(const ximd::simd<T>& lhs, const typename ximd::simd<T>::value_type& rhs)
{
    return lhs * ximd::simd<T>(rhs);
}

template<typename T>
inline auto operator-(const ximd::simd<T>& lhs, const ximd::simd<T>& rhs)
{
    return ximd::simd<T>([&](size_t i) { return lhs[i] - rhs[i]; });
}
inline auto& operator-=(ximd::intv& lhs, const ximd::intv& rhs)
{
    lhs = ximd::intv([&](size_t i) { lhs[i] -= rhs[i]; return lhs[i]; });
    return lhs;
}
inline auto operator-(const ximd::intv& lhs, const typename ximd::intv::value_type& rhs)
{
    return lhs - ximd::intv(rhs);
}

inline auto& operator+=(ximd::intv& lhs, const ximd::intv& rhs)
{
    lhs = ximd::intv([&](size_t i) { lhs[i] += rhs[i]; return lhs[i]; });
    return lhs;
}

template<typename T>
inline auto operator+(const ximd::simd<T>& lhs, const ximd::simd<T>& rhs)
{
    return ximd::simd<T>([&](size_t i) { return lhs[i] + rhs[i]; });
}
template<typename T>
inline auto operator+(const ximd::simd<T>& lhs, const typename ximd::simd<T>::value_type& rhs)
{
    return lhs + ximd::simd<T>(rhs);
}

inline auto operator==(const ximd::intv& lhs, const typename ximd::intv::value_type& rhs)
{
    return ximd::simd_mask([&](size_t i) { return lhs[i] == rhs; });
}
inline auto operator==(const ximd::intv& lhs, const ximd::intv& rhs)
{
    return ximd::simd_mask([&](size_t i) { return lhs[i] == rhs[i]; });
}
inline auto operator<(const ximd::floatv& lhs, const ximd::floatv& rhs)
{
    return ximd::simd_mask([&](size_t i) { return lhs[i] < rhs[i]; });
}
inline auto operator<=(const ximd::floatv& lhs, const ximd::floatv& rhs)
{
    return ximd::simd_mask([&](size_t i) { return lhs[i] <= rhs[i]; });
}
inline auto operator>(const ximd::intv& lhs, const typename ximd::intv::value_type& rhs)
{
    return ximd::simd_mask([&](size_t i) { return lhs[i] < rhs; });
}

inline bool any_of(const ximd::simd_mask& m)
{
    for (size_t i = 0; i < m.size(); i++)
    {
        if (m[i])
        {
            return true;
        }
    }
    return false;
}


using floatv = ximd::floatv;
using intv = ximd::intv;
template<typename TGenerator>
inline auto make_intv(TGenerator&& generate)
{
    return intv(generate);
}
template<typename TGenerator>
inline auto make_floatv(TGenerator&& generate)
{
    return floatv(generate);
}

using zfloatv = std::array<floatv, 2>;
auto& real(zfloatv& z)
{
    return z[0];
}
const auto& real(const zfloatv& z)
{
    return z[0];
}
auto& imag(zfloatv& z)
{
    return z[1];
}
const auto& imag(const zfloatv& z)
{
    return z[1];
}
auto size(const zfloatv& z)
{
    auto retval = real(z).size();
    assert(retval == imag(z).size());
    return retval;
}

template<typename TGeneratorReal, typename TGeneratorImag>
inline auto make_zfloatv(TGeneratorReal&& generate_real, TGeneratorImag&& generate_imag)
{
    zfloatv retval;
    for (size_t i = 0; i < size(retval); i++)
    {
        real(retval)[i] = generate_real(i);
        imag(retval)[i] = generate_imag(i);
    }
    return retval;
}

static inline auto load(std::span<const six::zfloat> p)
{
    const auto generate_real = [&](size_t i) {
        return p[i].real();
    };
    const auto generate_imag = [&](size_t i) {
        return p[i].imag();
    };
    return make_zfloatv(generate_real, generate_imag);
}

// https://en.cppreference.com/w/cpp/numeric/complex/arg
// > `std::atan2(std::imag(z), std::real(z))`
inline auto arg(const floatv& real, const floatv& imag)
{
    const auto generate_atan2 = [&](size_t i) {
        return atan2(imag[i], real[i]); // arg()
    };
    return make_floatv(generate_atan2);
}
inline auto arg(const zfloatv& z)
{
    return arg(real(z), imag(z));
}

inline auto round(const floatv& v)
{
    const auto generate_round = [&](size_t i) {
        return std::round(v[i]);
    };
    return make_floatv(generate_round);
}

inline auto roundi(const floatv& v) // match vcl::roundi()
{
    const auto generate_roundi = [&](size_t i) {
        return static_cast<typename intv::value_type>(v[i]);
    };
    return make_intv(generate_roundi);
}

template<typename TTest, typename TResult>
inline auto select(const TTest& /*test_*/, const TResult& /*t*/, const TResult& /*f*/)
{
    //const auto test = test_.__cvt(); // https://github.com/VcDevel/std-simd/issues/41
    //const auto test = stdx::static_simd_cast<typename TResult::mask_type>(test_); // https://github.com/VcDevel/std-simd/issues/41

    // https://en.cppreference.com/w/cpp/experimental/simd/where_expression
    // > ... All other elements are left unchanged.
    TResult retval;
    //where(test, retval) = t;
    //where(!test, retval) = f;
    return retval;
}

static auto if_add(const ximd::simd_mask& m, const floatv& v, typename floatv::value_type c)
{
    // phase = if_add(phase < 0.0, phase, std::numbers::pi_v<float> * 2.0f); // Wrap from [0, 2PI]
    const auto generate_add = [&](size_t i) {
        return m[i] ? v[i] + c : v[i];
    };
    return make_floatv(generate_add);
}
static inline auto getPhase(const zfloatv& v, float phase_delta)
{
    // Phase is determined via arithmetic because it's equally spaced.
    // There's an intentional conversion to zero when we cast 256 -> uint8. That wrap around
    // handles cases that are close to 2PI.
    auto phase = arg(v);
    //where(phase < 0.0f, phase) += std::numbers::pi_v<float> *2.0f; // Wrap from [0, 2PI]
    phase = if_add(phase < 0.0f, phase, std::numbers::pi_v<float> * 2.0f); // Wrap from [0, 2PI]
    return roundi(phase / phase_delta);
}

template<size_t N>
static inline auto lookup(const intv& zindex, const std::array<six::zfloat, N>& phase_directions)
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
    return make_zfloatv(generate_real, generate_imag);
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
template<size_t N>
static inline auto lookup(const intv& zindex, std::span<const float> magnitudes)
{
    // It seems that the "generator" constuctor is called with SIMD instructions.
    // https://en.cppreference.com/w/cpp/experimental/simd/simd/simd
    // > The calls to `generator` are unsequenced with respect to each other.

    const auto generate = [&](size_t i) {
        const auto i_ = zindex[i];
        return magnitudes[i_];
    };
    return make_floatv(generate);
}
inline auto lower_bound_(std::span<const float> magnitudes, const floatv& v)
{
    intv first = 0;
    const intv last = gsl::narrow<int>(magnitudes.size());

    auto count = last - first;
    while (any_of(count > 0))
    {
        auto it = first;
        const auto step = count / 2;
        it += step;

        auto next = it; ++next; // ... ++it;
        auto advance = count; advance -= step + 1;  // ...  -= step + 1;

        const auto c = lookup<six::AmplitudeTableSize>(it, magnitudes); // magnituides[it]

        const auto test = c < v; // (c < v).__cvt(); // https://github.com/VcDevel/std-simd/issues/41

        //where(test, it) = next; // ... ++it
        //where(test, first) = it; // first = ...
        //// count -= step + 1 <...OR...> count = step
        //where(test, count) = advance;
        //where(!test, count) = step;
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
    for (size_t i = 0; i < value.size(); i++)
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

    const auto v0 = value - lookup<six::AmplitudeTableSize>(prev_it, magnitudes); // value - *prev_it
    const auto v1 = lookup<six::AmplitudeTableSize>(it, magnitudes) - value; // *it - value
    //const auto nearest_it = select(v0 <= v1, prev_it, it); //  (value - *prev_it <= *it - value ? prev_it : it);
    
    const intv end = gsl::narrow<int>(magnitudes.size());
    //const auto end_test = select(it == end, prev_it, nearest_it); // it == end ? prev_it  : ...
    const intv zero = 0;
    auto retval = select(it == 0, zero, // if (it == begin) return 0;
        select(it == end, prev_it,  // it == end ? prev_it  : ...
            select(v0 <= v1, prev_it, it) //  (value - *prev_it <= *it - value ? prev_it : it);
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
    return find_nearest(magnitudes, real(phase_direction), imag(phase_direction), v);
}

void six::sicd::details::ComplexToAMP8IPHS8I::Impl::nearest_neighbors_unseq_(std::span<const six::zfloat> p, std::span<AMP8I_PHS8I_t> results) const
{
    const auto v = load(p);
    #if CODA_OSS_DEBUG
    for (size_t i = 0; i < size(v); i++)
    {
        const auto z = p[i];
        assert(real(v)[i] == z.real());
        assert(imag(v)[i] == z.imag());
    }
    #endif

    const auto phase = ::getPhase(v, phase_delta);
    #if CODA_OSS_DEBUG
    for (size_t i = 0; i < phase.size(); i++)
    {
        const auto phase_ = getPhase(p[i]);
        assert(static_cast<uint8_t>(phase[i]) == phase_);
    }
    #endif

    const auto phase_direction = lookup<six::AmplitudeTableSize>(phase, phase_directions);
    #if CODA_OSS_DEBUG
    for (size_t i = 0; i < phase.size(); i++)
    {
        const auto pd = phase_directions[phase[i]];
        assert(pd.real() == real(phase_direction)[i]);
        assert(pd.imag() == imag(phase_direction)[i]);
    }
    #endif

    const auto amplitude = ::find_nearest(magnitudes, phase_direction, v);
    #if CODA_OSS_DEBUG
    for (size_t i = 0; i < amplitude.size(); i++)
    {
        const auto i_ = phase[i];
        const auto a = find_nearest(phase_directions[i_], p[i]);
        assert(a == amplitude[i]);
    }
    #endif

    auto dest = results.begin();
    for (size_t i = 0; i < v.size(); i++)
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
    constexpr auto elements_per_iteration = ximd::native_size;

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
std::vector<six::AMP8I_PHS8I_t> six::sicd::details::ComplexToAMP8IPHS8I::nearest_neighbors_unseq_ximd(
    std::span<const zfloat> inputs, const six::AmplitudeTable* pAmplitudeTable)
{
    // make a structure to quickly find the nearest neighbor
    const auto& converter = make_(pAmplitudeTable);

    std::vector<six::AMP8I_PHS8I_t> retval(inputs.size());
    converter.impl.nearest_neighbors_unseq(inputs, sys::make_span(retval));
    return retval;
}

#endif // SIX_sicd_has_ximd
