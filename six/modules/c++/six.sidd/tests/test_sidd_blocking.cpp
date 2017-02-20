/* =========================================================================
* This file is part of six.sidd-c++
* =========================================================================
*
* (C) Copyright 2004 - 2017, MDA Information Systems LLC
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

#include <iostream>
#include <iterator>
#include <io/TempFile.h>
#include <str/Manip.h>
#include <sys/Exec.h>
#include <sys/Path.h>
#include <import/six/sidd.h>
#include "six/NITFWriteControl.h"
#include "six/Types.h"

namespace
{
class TempFileWithExtension
{
public:
    TempFileWithExtension(std::string extension)
    {
        if (!str::startsWith(extension, "."))
        {
            extension = "." + extension;
        }
        const io::TempFile tempFile;
        mPathname = tempFile.pathname() + extension;
    }

    inline std::string pathname() const
    {
        return mPathname;
    }

    ~TempFileWithExtension()
    {
        try
        {
            sys::OS os;
            if (os.exists(mPathname))
            {
                os.remove(mPathname);
            }
        }
        catch (...)
        {
            // Do nothing
        }
    }
private:
    std::string mPathname;
};

std::string getProgramPathname(const std::string& installPathname,
        const std::string& programName)
{
    const sys::Path testPath = sys::Path(installPathname).join("bin").
            join(programName);
    const std::string testPathname = testPath.exists() ? testPath.getPath() :
            testPath.getPath() + ".exe";

    if (!sys::OS().exists(testPathname))
    {
        throw except::Exception(Ctxt("Unable to find " + testPathname));
    }
    return testPathname;
}

void getMultiImageSIDD(const std::string& installPathname,
        const std::string& siddPathname)
{
    const std::string siddCreator =
            getProgramPathname(installPathname, "test_create_sidd_from_mem");
    sys::Exec createSiddCommand(siddCreator + " --lut Mono --multipleSegments "
            + siddPathname);
    createSiddCommand.run();
}

void roundTripAndBlock(const std::string& installPathname,
        const std::string& inputPathname,
        const std::string& outputPathname)
{
    const std::string roundTripProgram =
            getProgramPathname(installPathname, "round_trip_six");
    sys::Exec roundTripCommand(roundTripProgram + " --retainDateTime" +
            " --size 5000 --rowsPerBlock 10 --colsPerBlock 10 " +
            inputPathname + " " + outputPathname);
    roundTripCommand.run();
}

bool checkBlocking(const std::string& originalPathname,
        const std::string& convertedPathname)
{
    six::NITFReadControl originalReader;
    originalReader.load(originalPathname);
    six::NITFReadControl convertedReader;
    convertedReader.load(convertedPathname);

    mem::SharedPtr<six::Container> originalContainer =
            originalReader.getContainer();

    six::Region originalRegion;
    originalRegion.setStartRow(0);
    originalRegion.setStartCol(0);

    six::Region convertedRegion;
    convertedRegion.setStartRow(0);
    convertedRegion.setStartCol(0);

    const size_t numRows = originalContainer->getData(0)->getNumRows();
    const size_t numCols = originalContainer->getData(0)->getNumCols();
    const size_t bytesPerPixel =
            originalContainer->getData(0)->getNumBytesPerPixel();
    const size_t bufferSize = numRows * numCols * bytesPerPixel;

    mem::ScopedArray<six::UByte> originalBuffer(
            originalReader.interleaved(originalRegion, 0));
    mem::ScopedArray<six::UByte> convertedBuffer(
            convertedReader.interleaved(convertedRegion, 0));
    for (size_t jj = 0; jj < bufferSize; ++jj)
    {
        if (originalBuffer[jj] != convertedBuffer[jj])
        {
            std::cerr << "Error with round trip\n";
            return false;
        }
    }
    std::cerr << "Round trip succeeded\n";
    return true;
}
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <installPathname>\n";
        return 1;
    }
    try
    {
        six::XMLControlFactory::getInstance().addCreator(
            six::DataType::DERIVED,
            new six::XMLControlCreatorT<
            six::sidd::DerivedXMLControl>());

        const std::string installPathname(argv[1]);
        TempFileWithExtension multiImageSidd(".nitf");
        TempFileWithExtension roundTrippedSidd(".nitf");
        getMultiImageSIDD(installPathname, multiImageSidd.pathname());
        roundTripAndBlock(installPathname,
                multiImageSidd.pathname(), roundTrippedSidd.pathname());

        if (checkBlocking(multiImageSidd.pathname(),
                roundTrippedSidd.pathname()))
        {
            return 0;
        }
        return 1;
    }
    catch (const except::Exception& ex)
    {
        std::cerr << ex.toString() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown exception\n";
    }

    return 1;
}

