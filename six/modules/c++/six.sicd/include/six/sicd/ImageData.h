/* =========================================================================
 * This file is part of six.sicd-c++
 * =========================================================================
 *
 * (C) Copyright 2004 - 2014, MDA Information Systems LLC
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
#ifndef __SIX_IMAGE_DATA_H__
#define __SIX_IMAGE_DATA_H__

#include <assert.h>

#include <array>
#include <memory>
#include <std/span>
#include <utility>
#include <future>

#include "logging/Logger.h"
#include "types/Complex.h"

#include "six/Types.h"
#include "six/Init.h"
#include "six/Parameter.h"

namespace six
{
namespace sicd
{
using cx_float = six::zfloat;

class GeoData;
/*!
 *  \struct ImageData
 *  \brief SICD ImageData parameters
 *
 *  This block describes image pixel data
 */
struct ImageData
{
    //!  Everything is undefined at this time
    ImageData() = default;

    /*!
     *  Indicates the pixel type and binary format of the data.
\     *
     */
    PixelType pixelType = PixelType::NOT_SET;

    /*!
     *  SICD AmpTable parameter.  If the data is AMP8I_PHS8I (see above)
     *  this could be initialized, and could store a double precision
     *  LUT (256 entries) that the phase portion signifies.
     *
     */
    mem::ScopedCloneablePtr<AmplitudeTable> amplitudeTable;

    //!  Number of rows in the product, including zero-filled pixels
    size_t numRows = Init::undefined<size_t>();

    //!  Number of cols in the product, including zero-filled pixels
    size_t numCols = Init::undefined<size_t>();

    //!  Global row index (assuming this is an ROI)
    size_t firstRow = 0;

    //!  Global col index (assuming this is an ROI)
    size_t firstCol = 0;

    //!  Global row/col size (assuming this is an ROI)
    RowColInt fullImage;

    //!  Scene center (in global scene)
    RowColInt scpPixel;

    // If this doesn't have at least 3 vertices, it's not going to get written
    std::vector<RowColInt> validData;

    //! Equality operators
    bool operator==(const ImageData& rhs) const;
    bool operator!=(const ImageData& rhs) const
    {
        return !(*this == rhs);
    }

    bool validate(const GeoData& geoData, logging::Logger& log) const;

    static std::vector<AMP8I_PHS8I_t> testing_fromComplex_(std::span<const six::zfloat>); // for unit-tests

    static void toComplex(const six::Amp8iPhs8iLookup_t& lookup, std::span<const AMP8I_PHS8I_t>, std::span<six::zfloat>);
    std::vector<six::zfloat> toComplex(std::span<const AMP8I_PHS8I_t>) const;
    void fromComplex(std::span<const six::zfloat>, std::span<AMP8I_PHS8I_t>) const;
    std::vector<AMP8I_PHS8I_t> fromComplex(std::span<const six::zfloat>) const;

    /*!
     * Create a lookup table for converting from AMP8I_PHS8I to complex.
     * @param pAmplitudeTable Input amplitude table. May be nullptr if no amplitude table is defined.
     * @return reference to the output lookup table.
     */
    static const six::Amp8iPhs8iLookup_t& getLookup(const six::AmplitudeTable* pAmplitudeTable);
};

}
}

#endif
