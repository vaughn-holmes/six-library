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
#pragma once

#include <ostream>

#include <sys/Conf.h>
#include <except/Exception.h>
#include "six/Types.h"
#include "six/Data.h"
#include "six/Enums.h"
#include "six/XMLControlFactory.h"
#include "logging/Logger.h"
#include "scene/SceneGeometry.h"
#include "six/ErrorStatistics.h"
#include "six/Init.h"
#include <scene/Utilities.h>
#include <import/io.h>
#include <import/xml/lite.h>
#include <import/str.h>
#include <vector>
#include <memory>

namespace six
{

/*!
 *  Remaps angles into [0:360]
 *
 *  Others ranges are explicitly unsupported because
 *  of remapping issues, and that angles outside those
 *  ranges generally indicate errors. For instance a
 *  Latitude value outside of the [-90:90] range would
 *  be considered an error.
 *
 *  Unsupported ranges are:
 *      [0:90]
 *      [-90:90]
 *      [-180:180]
 */
inline
double remapZeroTo360(double degree)
{
    return scene::Utilities::remapZeroTo360(degree);
}


// TODO eventually replace enum toString/toType methods below directly in the
// Enums.h header - it is currently being generated so we won't change it yet

template<typename T> T toType(const std::string& s)
{
    return str::toType<T>(s);
}

std::string toString(const float& value);
std::string toString(const double& value);
std::string toString(const six::Vector3 & v);
std::string toString(const six::PolyXYZ & p);
template<> six::EarthModelType
        toType<six::EarthModelType>(const std::string& s);
template<> six::MagnificationMethod
        toType<six::MagnificationMethod>(const std::string& s);
template<> six::MagnificationMethod
        toType<six::MagnificationMethod>(const std::string& s);
template<> six::DecimationMethod
        toType<six::DecimationMethod>(const std::string& s);

template<> six::PixelType toType<six::PixelType>(const std::string& s);

template<> six::RadarModeType toType<six::RadarModeType>(const std::string& s);
std::string toString(const six::RadarModeType& value);

template<> six::DateTime toType<six::DateTime>(const std::string& s);
std::string toString(const six::DateTime& value);

std::string toString(const six::DataType& value);
inline std::ostream& operator<<(std::ostream& os, const DataType& value)
{
    os << toString(value);
    return os;
}
std::string toString(const six::EarthModelType& value);
template<> six::OrientationType
        toType<six::OrientationType>(const std::string& s);
template<> six::PolarizationSequenceType
        toType<six::PolarizationSequenceType>(const std::string& s);
template<> six::PolarizationType
        toType<six::PolarizationType>(const std::string& s);

std::string toString(const six::DualPolarizationType& value);
template<> six::DualPolarizationType
        toType<six::DualPolarizationType>(const std::string& s);
template<> six::DemodType toType<six::DemodType>(const std::string& s);

template<> six::ImageFormationType
        toType<six::ImageFormationType>(const std::string& s);
template<> six::SlowTimeBeamCompensationType toType<
        six::SlowTimeBeamCompensationType>(const std::string& s);
template<> six::ImageBeamCompensationType
toType<six::ImageBeamCompensationType>(const std::string& s);
template<> six::AutofocusType toType<six::AutofocusType>(const std::string& s);
template<> six::RMAlgoType toType<six::RMAlgoType>(const std::string& s);
std::string toString(const six::SideOfTrackType& value);

template<> six::BooleanType toType<six::BooleanType>(const std::string& s);
std::string toString(const six::BooleanType& value);

template<> six::SideOfTrackType
        toType<six::SideOfTrackType>(const std::string& s);

template<> six::ComplexImagePlaneType
        toType<six::ComplexImagePlaneType>(const std::string& s);

template<> six::ComplexImageGridType
        toType<six::ComplexImageGridType>(const std::string& s);

template<> six::FFTSign toType<six::FFTSign>(const std::string& s);
std::string toString(const six::FFTSign& value);

template<> six::AppliedType toType<six::AppliedType>(const std::string& s);
std::string toString(const six::AppliedType& value);

template<> six::CollectType toType<six::CollectType>(const std::string& s);

template<> six::FrameType toType<six::FrameType>(const std::string& s);
std::string toString(const six::FrameType& value);

std::string toString(const six::LatLonCorners& corners);

// Load the TRE plugins in the given directory
// In most cases this is not needed as XML_DATA_CONTENT is linked statically
void loadPluginDir(const std::string& pluginDir);

/*
 * Used to ensure the PluginRegistry singleton has loaded the XML_DATA_CONTENT
 * handler.  This is used internally by NITFReadControl and NITFWriteControl
 * and should not need to be called directly.
 */
void loadXmlDataContentHandler();

/*
 * Parses the XML in 'xmlStream' and converts it into a Data object
 *
 * \param xmlReg XML registry
 * \param xmlStream Input stream containing XML
 * \param dataType Complex vs. Derived.  If the resulting object is not the
 * expected type, throw.  To avoid this check, set to NOT_SET.
 * \param schemaPaths Schema path(s)
 * \param log Logger
 *
 * \return Data representation of 'xmlStream'
 */
std::unique_ptr<Data> parseData(const XMLControlRegistry& xmlReg,
                              ::io::InputStream& xmlStream, 
                              DataType dataType,
                              const std::vector<std::string>& schemaPaths,
                              logging::Logger& log);

/*
 * Parses the XML in 'xmlStream' and converts it into a Data object.  Same as
 * above but doesn't require the data type to be known in advance.
 *
 * \param xmlReg XML registry
 * \param xmlStream Input stream containing XML
 * \param schemaPaths Schema path(s)
 * \param log Logger
 *
 * \return Data representation of 'xmlStream'
 */
inline
std::unique_ptr<Data> parseData(const XMLControlRegistry& xmlReg,
                              ::io::InputStream& xmlStream,
                              const std::vector<std::string>& schemaPaths,
                              logging::Logger& log)
{
  return parseData(xmlReg, xmlStream, DataType::NOT_SET, schemaPaths, log);
}

/*
 * Parses the XML in 'pathname' and converts it into a Data object.
 *
 * \param xmlReg XML registry
 * \param pathname File containing plain text XML (not a NITF)
 * \param dataType Complex vs. Derived.  If the resulting object is not the
 * expected type, throw.  To avoid this check, set to NOT_SET.
 * \param schemaPaths Schema path(s)
 * \param log Logger
 *
 * \return Data representation of the contents of 'pathname'
 */
std::unique_ptr<Data> parseDataFromFile(const XMLControlRegistry& xmlReg,
    const std::string& pathname,
    DataType dataType,
    const std::vector<std::string>& schemaPaths,
    logging::Logger& log);

/*
 * Parses the XML in 'pathname' and converts it into a Data object.  Same as
 * above but doesn't require the data type to be known in advance.
 *
 * \param xmlReg XML registry
 * \param pathname File containing plain text XML (not a NITF)
 * \param schemaPaths Schema path(s)
 * \param log Logger
 *
 * \return Data representation of the contents of 'pathname'
 */
inline
std::unique_ptr<Data> parseDataFromFile(const XMLControlRegistry& xmlReg,
    const std::string& pathname,
    const std::vector<std::string>& schemaPaths,
    logging::Logger& log)
{
  return parseDataFromFile(xmlReg, pathname, DataType::NOT_SET, schemaPaths,
                             log);
}

/*
 * Parses the XML in 'xmlStr' and converts it into a Data object.
 *
 * \param xmlReg XML registry
 * \param xmlStr XML document as a string
 * \param dataType Complex vs. Derived.  If the resulting object is not the
 * expected type, throw.  To avoid this check, set to NOT_SET.
 * \param schemaPaths Schema path(s)
 * \param log Logger
 *
 * \return Data representation of 'xmlStr'
 */
std::unique_ptr<Data> parseDataFromString(const XMLControlRegistry& xmlReg,
    const std::string& xmlStr,
    DataType dataType,
    const std::vector<std::string>& schemaPaths,
    logging::Logger& log);

/*
 * Parses the XML in 'xmlStr' and converts it into a Data object.  Same as
 * above but doesn't require the data type to be known in advance.
 *
 * \param xmlReg XML registry
 * \param xmlStr XML document as a string
 * \param schemaPaths Schema path(s)
 * \param log Logger
 *
 * \return Data representation of 'xmlStr'
 */
inline
std::unique_ptr<Data> parseDataFromString(const XMLControlRegistry& xmlReg,
    const std::string& xmlStr,
    const std::vector<std::string>& schemaPaths,
    logging::Logger& log)
{
  return parseDataFromString(xmlReg, xmlStr, DataType::NOT_SET, schemaPaths,
                               log);
}

void getErrors(const ErrorStatistics* errorStats,
               const types::RgAz<double>& sampleSpacing,
               scene::Errors& errors);

/*
 * Try to find schema directory from given location.
 * Return empty string on failure
 *
 * \param progname name of executable
 * \return path to schema directory, or empty string
 */
std::string findSchemaPath(const std::string& progname);
}

