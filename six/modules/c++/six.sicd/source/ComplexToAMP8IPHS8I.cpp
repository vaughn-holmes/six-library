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
#include <stdexcept>

#include <mt/Algorithm.h>
#include <gsl/gsl.h>
#include <math/Utilities.h>
#include <units/Angles.h>

#include "six/sicd/Utilities.h"
#include "six/sicd/Exports.h"

#undef min
#undef max

// https://github.com/ngageoint/six-library/pull/537#issuecomment-1026453353
/*
I can add more detail, but be warned that my powerpoint skills aren't amazing and this is best drawn on a whiteboard or graph paper.
The 8-bit amplitude and phase values that we're mapping to have a systematic distribution in the complex domain. Here's what I mean
by systematic distribution:
![radial_plot](https://user-images.githubusercontent.com/16496326/151909984-f0b0cd4f-6607-4c05-9d98-038e54118daf.png)

A couple things to point out:
- The graph only shows 16 phase angles and 7 magnitudes per angle. A more realistic plot would have 256 phase angles and 256 magnitudes per angle.
- The `phase_direction[]` array contains a unit vector for each possible phase direction. Just the second vector is labeled in the plot.
- The `phase_delta` is the angular difference between each `phase_direction`.
- The `magnitudes` are the ordered set of possible magnitudes and they're identical for each phase angle.

Given any complex value `V`, we can determine the nearest phase angle by computing `round(phase(V) / phase_delta)`.
Determining the nearest magnitude value for `V` is slighly more complex:
![dot_product](https://user-images.githubusercontent.com/16496326/151910329-bede78ff-e4cb-4d6f-93f8-fb72b66cb361.png)

The complex value `V` is projected onto the nearest `phase_direction` via the dot product.
The resulting green point is then what's used to find the nearest magnitude via binary search (`std::lower_bound`).
*/

/*!
    * Get the phase of a complex value.
    * @param v complex value
    * @return phase between [0, 2PI]
    */

inline auto GetPhase(std::complex<double> v)
{
    // There's an intentional conversion to zero when we cast 256 -> uint8. That wrap around
    // handles cases that are close to 2PI.
    double phase = std::arg(v);
    if (phase < 0.0) phase += std::numbers::pi * 2.0; // Wrap from [0, 2PI]
    return phase;
}
uint8_t six::sicd::details::ComplexToAMP8IPHS8I::getPhase(six::zfloat v) const
{
    // Phase is determined via arithmetic because it's equally spaced.
    const auto phase = GetPhase(v);
    return gsl::narrow_cast<uint8_t>(std::round(phase / phase_delta));
}

template<typename TToComplexFunc>
static auto make_magnitudes_(TToComplexFunc toComplex)
{
    std::vector<float> result;
    result.reserve(six::AmplitudeTableSize);
    for (const auto amplitude : six::sicd::Utilities::iota_0_256())
    {
        // AmpPhase -> Complex
        const auto phase = amplitude;
        const auto complex = toComplex(amplitude, phase);
        result.push_back(std::abs(complex));
    }

    // I don't know if we can guarantee that the amplitude table is non-decreasing.
    // Check to verify property at runtime.
    if (!std::is_sorted(result.begin(), result.end()))
    {
        throw std::runtime_error("magnitudes must be sorted");
    }

    std::array<float, six::AmplitudeTableSize> retval;
    std::copy(result.begin(), result.end(), retval.begin());
    return retval;
}
static inline auto make_magnitudes(const six::AmplitudeTable& amplitudeTable)
{
    const auto toComplex = [&](auto amplitude, auto phase)
    {
        return six::sicd::Utilities::toComplex(amplitude, phase, amplitudeTable);
    };
    return make_magnitudes_(toComplex);
}
static inline auto make_magnitudes()
{
    static const auto toComplex = [](auto amplitude, auto phase)
    {
        return six::sicd::Utilities::toComplex(amplitude, phase);
    };
    return make_magnitudes_(toComplex);
}

static std::span<const float> get_magnitudes(const six::AmplitudeTable* pAmplitudeTable,
    std::array<float, six::AmplitudeTableSize>& uncached_magnitudes)
{
    if (pAmplitudeTable == nullptr)
    {
        static const auto magnitudes = make_magnitudes(); // OK to cache, won't change
        return sys::make_span(magnitudes);
    }
    
    uncached_magnitudes = make_magnitudes(*pAmplitudeTable);
    return sys::make_const_span(uncached_magnitudes);
}


six::sicd::details::ComplexToAMP8IPHS8I::ComplexToAMP8IPHS8I(const six::AmplitudeTable *pAmplitudeTable)
{
    magnitudes = get_magnitudes(pAmplitudeTable, uncached_magnitudes);

    const auto p0 = GetPhase(Utilities::toComplex(1, 0, pAmplitudeTable));
    const auto p1 = GetPhase(Utilities::toComplex(1, 1, pAmplitudeTable));
    assert(p0 == 0.0);
    assert(p1 > p0);
    phase_delta = gsl::narrow_cast<float>(p1 - p0);
    for(size_t i = 0; i < 256; i++)
    {
        const units::Radians<float> angle{ static_cast<float>(p0) + i * phase_delta };
        float y, x;
        SinCos(angle, y, x);
        phase_directions[i] = { x, y };

        // Only need the parallel array when using the "vectorclass" library.
        #ifdef SIX_sicd_has_VCL
        phase_directions_real[i] = phase_directions[i].real();
        phase_directions_imag[i] = phase_directions[i].imag();
        #endif
    }
}

/*!
 * Find the nearest element given an iterator range.
 * @param value query value
 * @return index of nearest value within the iterator range.
 */
static uint8_t nearest(std::span<const float> magnitudes, float value)
{
    const auto begin = magnitudes.begin();
    const auto end = magnitudes.end();

    const auto it = std::lower_bound(begin, end, value);
    if (it == begin) return 0;

    const auto prev_it = std::prev(it);
    const auto nearest_it = it == end ? prev_it  :
        (value - *prev_it <= *it - value ? prev_it : it);
    const auto distance = std::distance(begin, nearest_it);
    assert(distance <= std::numeric_limits<uint8_t>::max());
    return gsl::narrow<uint8_t>(distance);
}
uint8_t six::sicd::details::ComplexToAMP8IPHS8I::find_nearest(six::zfloat phase_direction, six::zfloat v) const
{
    // We have to do a 1D nearest neighbor search for magnitude.
    // But it's not the magnitude of the input complex value - it's the projection of
    // the complex value onto the ray of candidate magnitudes at the selected phase.
    // i.e. dot product.
    const auto projection = (phase_direction.real() * v.real()) + (phase_direction.imag() * v.imag());
    //assert(std::abs(projection - std::abs(v)) < 1e-5); // TODO ???
    return nearest(magnitudes, projection);
}
six::AMP8I_PHS8I_t six::sicd::details::ComplexToAMP8IPHS8I::nearest_neighbor_(const six::zfloat &v) const
{
    six::AMP8I_PHS8I_t retval;
    retval.phase = getPhase(v);

    auto&& phase_direction = phase_directions[retval.phase];
    retval.amplitude = find_nearest(phase_direction, v);
    return retval;
}
void six::sicd::details::ComplexToAMP8IPHS8I::nearest_neighbors_seq(std::span<const six::zfloat> inputs, std::span<AMP8I_PHS8I_t> results) const
{
    std::transform(inputs.begin(), inputs.end(), results.begin(),
        [&](const auto& v) { return nearest_neighbor_(v); });
}
std::vector<six::AMP8I_PHS8I_t> six::sicd::details::ComplexToAMP8IPHS8I::nearest_neighbors_seq(
    std::span<const zfloat> inputs, const six::AmplitudeTable* pAmplitudeTable)
{
    // make a structure to quickly find the nearest neighbor
    const auto& converter = make_(pAmplitudeTable);

    std::vector<six::AMP8I_PHS8I_t> retval(inputs.size());
    converter.nearest_neighbors_seq(inputs, retval);
    return retval;
}

six::AMP8I_PHS8I_t six::sicd::nearest_neighbor(const details::ComplexToAMP8IPHS8I& i, const six::zfloat& v)
{
    return i.nearest_neighbor_(v);
}


void six::sicd::details::ComplexToAMP8IPHS8I::nearest_neighbors_par(std::span<const six::zfloat> inputs, std::span<AMP8I_PHS8I_t> results) const
{
    const auto nearest_neighbor = [&](const auto& v)
    {
        return nearest_neighbor_(v);
    };
    std::ignore = mt::Transform_par(inputs.begin(), inputs.end(), results.begin(), nearest_neighbor);
}
std::vector<six::AMP8I_PHS8I_t> six::sicd::details::ComplexToAMP8IPHS8I::nearest_neighbors_par(
    std::span<const zfloat> inputs, const six::AmplitudeTable* pAmplitudeTable)
{
    // make a structure to quickly find the nearest neighbor
    const auto& converter = make_(pAmplitudeTable);

    std::vector<six::AMP8I_PHS8I_t> retval(inputs.size());
    converter.nearest_neighbors_par(inputs, sys::make_span(retval));
    return retval;
}

std::vector<six::AMP8I_PHS8I_t> six::sicd::details::ComplexToAMP8IPHS8I::nearest_neighbors(
    std::span<const zfloat> inputs, const six::AmplitudeTable* pAmplitudeTable)
{
    // TODO: there could be more complicated logic here to decide between
    // _seq, _par, _unseq, and _par_unseq
    #if SIX_sicd_ComplexToAMP8IPHS8I_unseq

    // None of our SIMD implementations are dramatically slower when
    // running unittests; that's mostly because there's lots of IO.  So that
    // the more interesting code-path is tested, use UNSEQ in debugging.
    #if CODA_OSS_DEBUG
    return nearest_neighbors_unseq(inputs, pAmplitudeTable); // TODO:
    #else
    return nearest_neighbors_par(inputs, pAmplitudeTable);
    #endif

    #else
    return nearest_neighbors_par(inputs, pAmplitudeTable);

#endif
}
std::vector<six::AMP8I_PHS8I_t> six::sicd::details::ComplexToAMP8IPHS8I::nearest_neighbors(execution_policy policy,
    std::span<const zfloat> inputs, const six::AmplitudeTable* pAmplitudeTable)
{
    #if SIX_sicd_ComplexToAMP8IPHS8I_unseq    
    if (policy == execution_policy::unseq)
    {
        return nearest_neighbors_unseq(inputs, pAmplitudeTable);
    }
    #endif

    if (policy == execution_policy::par)
    {
        return nearest_neighbors_par(inputs, pAmplitudeTable);
    }
    if (policy == execution_policy::seq)
    {
        return nearest_neighbors_seq(inputs, pAmplitudeTable);
    }

    // https://en.cppreference.com/w/cpp/algorithm/execution_policy_tag_t
    // > If the implementation cannot parallelize or vectorize (e.g. due to lack of resources),
    // > all standard execution policies can fall back to sequential execution.
    return nearest_neighbors(inputs, pAmplitudeTable); // no policy specified, "default policy"
}

const six::sicd::details::ComplexToAMP8IPHS8I& six::sicd::details::ComplexToAMP8IPHS8I::make_(const six::AmplitudeTable* pAmplitudeTable)
{
    if (pAmplitudeTable == nullptr)
    {
        // this won't change, so OK to cache
        static const six::sicd::details::ComplexToAMP8IPHS8I tree;
        return tree;
    }
    else
    {
        // Might have already "cached" this on the AmplitudeTable instance.
        auto pFromComplex = pAmplitudeTable->getFromComplex();
        if (pFromComplex != nullptr)
        {
            return *pFromComplex;
        }

        // constructor is private, can't use make_unique
        std::unique_ptr<sicd::details::ComplexToAMP8IPHS8I> pTree(new six::sicd::details::ComplexToAMP8IPHS8I(pAmplitudeTable));
        pAmplitudeTable->cacheFromComplex_(std::move(pTree));
        return *(pAmplitudeTable->getFromComplex());
    }
}
const six::sicd::details::ComplexToAMP8IPHS8I& six::sicd::make_ComplexToAMP8IPHS8I(const six::AmplitudeTable* pAmplitudeTable)
{
    return six::sicd::details::ComplexToAMP8IPHS8I::make_(pAmplitudeTable);
}