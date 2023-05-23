/* =========================================================================
 * This file is part of six.sidd-c++
 * =========================================================================
 *
 * (C) Copyright 2004 - 2014, MDA Information Systems LLC
 *
 * six.sidd-c++ is free software; you can redistribute it and/or modify
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
#ifndef __SIX_DERIVED_XML_CONTROL_H__
#define __SIX_DERIVED_XML_CONTROL_H__

#include <std/optional>

#include <six/XMLControl.h>
#include <six/Enums.h>

#include <six/sidd/DerivedXMLParser.h>

namespace six
{

// Emphasize that this is for SIDD 3.0.0
namespace sidd300
{
    // We have to support two ISM versions with SIDD 3.0 :-(
    enum class ISMVersion
    {
        v201609, // the "newer" version; default
        v13, // the "original" version

        current = v201609
    };
    ISMVersion getISMVersion(ISMVersion defaultIfNotSet = ISMVersion::current);
    std::optional<ISMVersion> setISMVersion(ISMVersion); // returns previous value, if any
    std::optional<ISMVersion> clearISMVersion(); // returns previous value, if any
}

namespace sidd
{
/*!
 *  \class DerivedXMLControl
 *  \brief Turns an DerivedData object into XML and vice versa
 *
 *  Derived XMLControl object for reading and writing DerivedData*
 *  The XMLFactory should be used to create this object, and only
 *  if necessary.  A best practice is to use the six::toXMLCharArray
 *  and six::toXMLString functions to turn Data* objects into XML
 */
struct DerivedXMLControl : public XMLControl
{
    DerivedXMLControl(logging::Logger* log = nullptr, bool ownLog = false);
    DerivedXMLControl(std::unique_ptr<logging::Logger>&&);
    DerivedXMLControl(logging::Logger&);
    DerivedXMLControl(const DerivedXMLControl&) = delete;
    DerivedXMLControl& operator=(const DerivedXMLControl&) = delete;
    DerivedXMLControl(DerivedXMLControl&&) = delete;
    DerivedXMLControl& operator=(DerivedXMLControl&&) = delete;

    static const six::DataType dataType;

    static std::unique_ptr<DerivedXMLParser> getParser_(const std::string& strVersion); // for unit-testing

    six::sidd300::ISMVersion getISMVersion() const;
    void setISMVersion(six::sidd300::ISMVersion);

protected:
    /*!
     *  Returns a new allocated DOM document, created from the DerivedData*
     */
    virtual xml::lite::Document* toXMLImpl(const Data* data);
    virtual std::unique_ptr<xml::lite::Document> toXMLImpl(const Data&) const override;
    /*!
     *  Returns a new allocated DerivedData*, created from the DOM Document*
     *
     */
    virtual Data* fromXMLImpl(const xml::lite::Document* doc);
    virtual std::unique_ptr<Data> fromXMLImpl(const xml::lite::Document&) const override;

private:
    std::unique_ptr<DerivedXMLParser>
    getParser(const std::string& strVersion) const;

    // Keep this "private" ... pass the value to DerivedXMLParser300
    six::sidd300::ISMVersion mISMVersion = six::sidd300::ISMVersion::current;
};
}
}

#endif

