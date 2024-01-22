// 8AMPI_PHSI.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>

#include <assert.h>

#include <iostream>
#include <std/span>
#include <algorithm>
#include <vector>
#include <tuple>
#include <chrono>

#include "six/AmplitudeTable.h"
#include "six/sicd/Utilities.h"

using namespace six;

static const six::sicd::ImageData imageData;
static std::vector<AMP8I_PHS8I_t> fromComplex_seq(std::span<const six::zfloat> inputs)
{
    return imageData.fromComplex(six::execution_policy::seq, inputs);
}
static std::vector<AMP8I_PHS8I_t> fromComplex_par(std::span<const six::zfloat> inputs)
{
    return imageData.fromComplex(six::execution_policy::par, inputs);
}
static std::vector<AMP8I_PHS8I_t> fromComplex_unseq(std::span<const six::zfloat> inputs)
{
    return imageData.fromComplex(six::execution_policy::unseq, inputs);
}
static std::vector<AMP8I_PHS8I_t> fromComplex_par_unseq(std::span<const six::zfloat> inputs)
{
    return imageData.fromComplex(six::execution_policy::par_unseq, inputs);
}

static auto make_cxinputs(size_t count)
{
    std::vector<zfloat> retval;
    retval.reserve(count);
    for (size_t i = 0; i < count; i++)
    {
        float f = static_cast<float>(i);
        retval.emplace_back(f, f);
        retval.emplace_back(-f, f);
        retval.emplace_back(f, -f);
        retval.emplace_back(-f, -f);
    }
    return retval;
}

#ifdef NDEBUG
constexpr auto iterations = 10;
#else
constexpr auto iterations = 1;
#endif
template<typename TFunc>
static std::chrono::duration<double> test(TFunc func, const std::vector<six::zfloat>& inputs_)
{
    const auto inputs = sys::make_span(inputs_);

    std::ignore = func(inputs);
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++)
    {
        std::ignore = func(inputs);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return end - start;
}

#define TEST(name) diff = test(name, inputs); \
std::cout << "Time (" #name "): " << std::setw(9) << diff.count() << "\n"

int main()
{
    assert(imageData.amplitudeTable.get() == nullptr);

    #ifdef NDEBUG
    constexpr auto inputs_size = 1'000'000;
    #else
    constexpr auto inputs_size = 100;
    #endif
    const auto inputs = make_cxinputs(inputs_size * 4);

    /*********************************************************************************/
    std::chrono::duration<double> diff;

    TEST(fromComplex_seq);
    TEST(fromComplex_par);
    TEST(fromComplex_unseq);
    TEST(fromComplex_par_unseq);
}
