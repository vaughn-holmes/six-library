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
#include <six/NITFWriteControl.h>

#include <iomanip>
#include <sstream>
#include <string>
#include <std/bit>
#include <std/memory>
#include <algorithm>

#include <io/ByteStream.h>
#include <math/Round.h>
#include <mem/ScopedArray.h>
#include <gsl/gsl.h>

#include <six/XMLControlFactory.h>
#include <nitf/IOStreamWriter.hpp>


namespace six
{
NITFWriteControl::NITFWriteControl()
{
    mNITFHeaderCreator.reset(new six::NITFHeaderCreator());
}

NITFWriteControl::NITFWriteControl(std::shared_ptr<Container> container)
{
    mNITFHeaderCreator.reset(new six::NITFHeaderCreator(container));
}

NITFWriteControl::NITFWriteControl(const six::Options& options,
                                   std::shared_ptr<Container> container,
                                   const XMLControlRegistry* xmlRegistry)
{
    mNITFHeaderCreator.reset(new six::NITFHeaderCreator(options, container));
    if (xmlRegistry)
    {
        // Indirecting through *Impl to avoid virtual function call in ctor
        setXMLControlRegistryImpl(xmlRegistry);
    }
}

void NITFWriteControl::setXMLControlRegistryImpl(
        const XMLControlRegistry* xmlRegistry)
{
    mNITFHeaderCreator->setXMLControlRegistry(xmlRegistry);
    WriteControl::setXMLControlRegistryImpl(xmlRegistry);
}

void NITFWriteControl::initialize(const six::Options& options,
                                  std::shared_ptr<Container> container)
{
    mNITFHeaderCreator->initialize(options, container);
}

void NITFWriteControl::initialize(std::shared_ptr<Container> container)
{
    mNITFHeaderCreator->initialize(container);
}

void NITFWriteControl::setNITFHeaderCreator(
        std::unique_ptr<six::NITFHeaderCreator>&& headerCreator)
{
    mNITFHeaderCreator.reset(headerCreator.release());
}
#if !CODA_OSS_cpp17
void NITFWriteControl::setNITFHeaderCreator(
        mem::auto_ptr<six::NITFHeaderCreator> headerCreator)
{
    setNITFHeaderCreator(std::unique_ptr<six::NITFHeaderCreator>(headerCreator.release()));
}
#endif

std::string NITFWriteControl::getComplexIID(size_t segmentNum,
                                            size_t numImageSegments)
{
    return NITFHeaderCreator::getComplexIID(segmentNum, numImageSegments);
}

std::string NITFWriteControl::getDerivedIID(size_t segmentNum,
                                            size_t productNum)
{
    return NITFHeaderCreator::getDerivedIID(segmentNum, productNum);
}

std::string NITFWriteControl::getIID(DataType dataType,
                                     size_t segmentNum,
                                     size_t numImageSegments,
                                     size_t productNum)
{
    return NITFHeaderCreator::getIID(dataType,
                                     segmentNum,
                                     numImageSegments,
                                     productNum);
}

void NITFWriteControl::setBlocking(const std::string& imode,
                                   const types::RowCol<size_t>& segmentDims,
                                   nitf::ImageSubheader& subheader)
{
    mNITFHeaderCreator->setBlocking(imode, segmentDims, subheader);
}

void NITFWriteControl::setImageSecurity(const six::Classification& c,
                                        nitf::ImageSubheader& subheader)
{
    mNITFHeaderCreator->setImageSecurity(c, subheader);
}

void NITFWriteControl::setDESecurity(const six::Classification& c,
                                     nitf::DESubheader& subheader)
{
    mNITFHeaderCreator->setDESecurity(c, subheader);
}

void NITFWriteControl::setSecurity(const six::Classification& c,
                                   nitf::FileSecurity security,
                                   const std::string& prefix)
{
    mNITFHeaderCreator->setSecurity(c, security, prefix);
}

std::string NITFWriteControl::getNITFClassification(const std::string& level)
{
    return NITFHeaderCreator::getNITFClassification(level);
}

void NITFWriteControl::updateFileHeaderSecurity()
{
    mNITFHeaderCreator->updateFileHeaderSecurity();
}

void NITFWriteControl::save(const SourceList& imageData,
                            const std::string& outputFile,
                            const std::vector<std::string>& schemaPaths)
{
    const size_t bufferSize = getOptions().getParameter(
            six::WriteControl::OPT_BUFFER_SIZE,
            Parameter(NITFHeaderCreator::DEFAULT_BUFFER_SIZE));

    nitf::BufferedWriter bufferedIO(outputFile, bufferSize);

    save(imageData, bufferedIO, schemaPaths);
    bufferedIO.close();
}

bool NITFWriteControl::shouldByteSwap() const
{
    const int byteSwapping = (int)getOptions().getParameter(
            six::WriteControl::OPT_BYTE_SWAP,
            Parameter(static_cast<int>(ByteSwapping::SWAP_AUTO)));

    if (byteSwapping == ByteSwapping::SWAP_AUTO)
    {
        // Have to if it's not a BE machine
        static auto endianness = std::endian::native;
        return endianness == std::endian::little;
    }
    else
    {
        // Do what they say.  You really shouldn't do this
        // unless you know what you're doing anyway!
        return byteSwapping ? true : false;
    }
}

template<typename TImageData>
static inline std::shared_ptr<NewMemoryWriteHandler> makeWriteHandler(NITFSegmentInfo segmentInfo,
    TImageData& imageData, const Data& data, bool doByteSwap)
{
    // this bypasses the normal NITF ImageWriter and streams directly to the output
    return std::make_shared<NewMemoryWriteHandler>(segmentInfo,
            imageData, segmentInfo.getFirstRow(), data, doByteSwap);
}
static std::shared_ptr<StreamWriteHandler> makeWriteHandler(NITFSegmentInfo segmentInfo,
    io::InputStream* imageData, const Data& data, bool doByteSwap)
{
    //! TODO: This section of code (unlike the memory section above)
    //        does not account for blocked writing or J2K compression.
    //        CODA ticket #443 will update support for this.
    return std::make_shared<StreamWriteHandler>(segmentInfo, imageData, data, doByteSwap);
}

template<typename TImageData>
void NITFWriteControl::writeWithoutNitro(TImageData&& imageData,
    const std::vector<NITFSegmentInfo>& imageSegments, size_t startIndex, const Data& data, bool doByteSwap)
{
    for (size_t j = 0; j < imageSegments.size(); ++j)
    {
        auto writeHandler = makeWriteHandler(imageSegments[j], std::forward<TImageData>(imageData), data, doByteSwap);
        mWriter.setImageWriteHandler(static_cast<int>(startIndex + j), writeHandler);
    }
}

bool NITFWriteControl::prepareIO(size_t imageDataSize, nitf::IOInterface& outputFile)
{
    const auto& infos = getInfos();
    if (infos.size() != imageDataSize)
    {
        std::ostringstream ostr;
        ostr << "Require " << infos.size() << " images, received " << imageDataSize;
        throw except::Exception(Ctxt(ostr.str()));
    }

    nitf::Record& record = getRecord();
    mWriter.prepareIO(outputFile, record);
    return shouldByteSwap();
}
bool NITFWriteControl::prepareIO(std::span<const std::byte* const> imageData, nitf::IOInterface& outputFile)
{
    return prepareIO(imageData.size(), outputFile);
}
bool NITFWriteControl::prepareIO(std::span<const std::complex<float>>, nitf::IOInterface& outputFile)
{
    return prepareIO(1 /*imageDataSize*/, outputFile);
}
bool NITFWriteControl::prepareIO(std::span<const std::pair<uint8_t, uint8_t>>, nitf::IOInterface& outputFile)
{
    return prepareIO(1 /*imageDataSize*/, outputFile);
}

void NITFWriteControl::save(const SourceList& imageData,
                            nitf::IOInterface& outputFile,
                            const std::vector<std::string>& schemaPaths)
{
    const bool doByteSwap = prepareIO(imageData.size(), outputFile);
    const auto& infos = getInfos();

    //! TODO: This section of code (unlike the memory section below)
    //        does not account for blocked writing or J2K compression.
    //        CODA ticket #443 will update support for this.
    for (size_t i = 0; i < infos.size(); ++i)
    {
        const NITFImageInfo& info = *(infos[i]);
        const std::vector<NITFSegmentInfo> imageSegments = info.getImageSegments();
        const auto startIndex = info.getStartIndex();
        const six::Data* const pData = info.getData();

        writeWithoutNitro(imageData[i], imageSegments, startIndex, *pData, doByteSwap);
    }

    addDataAndWrite(schemaPaths);
}

static void writeWithNitro_(const std::byte* const pImageData, const NITFSegmentInfo& segmentInfo, const Data& data, nitf::ImageWriter& iWriter)
{
    const auto numChannels = data.getNumChannels();
    const auto pixelSize = data.getNumBytesPerPixel() / numChannels;
    const auto numCols = data.getNumCols();

    // We will use the ImageWriter provided by NITRO so that we can
    // take advantage of the built-in compression capabilities
    iWriter.setWriteCaching(1);

    nitf::ImageSource iSource;
    const size_t bandSize = pixelSize * numCols * segmentInfo.getNumRows();

    for (size_t chan = 0; chan < numChannels; ++chan)
    {
        // Assume that the bands are interleaved in memory.  This
        // makes sense for 24-bit 3-color data.
        const auto pData = pImageData + pixelSize * segmentInfo.getFirstRow() * numCols;
        const auto start = gsl::narrow<nitf::Off>(chan);
        const auto numBytesPerPixel = gsl::narrow<int>(pixelSize);
        const auto pixelSkip = gsl::narrow<int>(numChannels - 1);
        nitf::MemorySource ms(pData, bandSize, start, numBytesPerPixel, pixelSkip);
        iSource.addBand(ms);
    }
    iWriter.attachSource(iSource);
}
static void writeWithNitro_(std::span<const std::complex<float>> imageData, const NITFSegmentInfo& segmentInfo, const Data& data, nitf::ImageWriter& iWriter)
{
    const void* pImageData = imageData.data();
    writeWithNitro_(static_cast<const std::byte*>(pImageData), segmentInfo, data, iWriter);
}
static void writeWithNitro_(std::span<const  std::pair<uint8_t, uint8_t>> imageData, const NITFSegmentInfo& segmentInfo, const Data& data, nitf::ImageWriter& iWriter)
{
    const void* pImageData = imageData.data();
    writeWithNitro_(static_cast<const std::byte*>(pImageData), segmentInfo, data, iWriter);
}
template<typename TImageData>
void NITFWriteControl::writeWithNitro(TImageData&& imageData,
    const std::vector<NITFSegmentInfo>& imageSegments, size_t startIndex, const Data& data, bool /*unused_*/)
{
    for (size_t jj = 0; jj < imageSegments.size(); ++jj)
    {
        // We will use the ImageWriter provided by NITRO so that we can
        // take advantage of the built-in compression capabilities
        nitf::ImageWriter iWriter = mWriter.newImageWriter(static_cast<int>(startIndex + jj), mCompressionOptions);
        writeWithNitro_(std::forward<TImageData>(imageData), imageSegments[jj], data, iWriter);
    }
}

static const Legend* getLegend(const six::Container* container, size_t i)
{
    const auto legend = container->getLegend(i);
    if (legend)
    {
        if (legend->mDims.row * legend->mDims.col != legend->mImage.size())
        {
            throw except::Exception(Ctxt("Legend dimensions don't match"));
        }

        if (legend->mImage.empty())
        {
            throw except::Exception(Ctxt("Empty legend"));
        }
    }
    return legend;
}
void NITFWriteControl::addLegend(const Legend& legend, int imageNumber)
{
    nitf::ImageSource iSource;
    nitf::MemorySource memSource(legend.mImage, 0 /*start*/, 0 /*pixelSkip*/);
    iSource.addBand(memSource);

    nitf::ImageWriter iWriter = mWriter.newImageWriter(imageNumber);
    iWriter.setWriteCaching(1);
    iWriter.attachSource(iSource);
}

inline const std::byte* const imageData_i(std::span<const std::byte* const> imageData, size_t i)
{
    return i < imageData.size() ? imageData[i] : nullptr;
}
inline std::span<const std::complex<float>> imageData_i(std::span<const std::complex<float>> imageData, size_t)
{
    return imageData;
}
inline std::span<const std::pair<uint8_t, uint8_t>> imageData_i(std::span<const std::pair<uint8_t, uint8_t>> imageData, size_t)
{
    return imageData;
}

template<typename T>
void NITFWriteControl::save_T(T&& imageData,
    nitf::IOInterface& outputFile,
    const std::vector<std::string>& schemaPaths)
{
    const bool doByteSwap = prepareIO(std::forward<T>(imageData), outputFile);

    // check to see if J2K compression is enabled
    double j2kCompression = (double)getOptions().getParameter(
        NITFHeaderCreator::OPT_J2K_COMPRESSION_BYTERATE, Parameter(0));

    bool enableJ2K = (getContainer()->getDataType() != DataType::COMPLEX) &&
        (j2kCompression <= 1.0) && j2kCompression > 0.0001;

    // TODO maybe we need to see if the compression plug-in is even available

    createCompressionOptions(mCompressionOptions);
    size_t numImages = getInfos().size();
    for (size_t i = 0; i < numImages; ++i)
    {
        const auto pInfo = getInfo(i);
        const std::vector<NITFSegmentInfo> imageSegments = pInfo->getImageSegments();
        const six::Data* const pData = pInfo->getData();

        const auto startIndex = pInfo->getStartIndex();
        nitf::ImageSegment imageSegment = getRecord().getImages()[startIndex];
        nitf::ImageSubheader subheader = imageSegment.getSubheader();

        const bool isBlocking = subheader.numBlocksPerRow() > 1 || subheader.numBlocksPerCol() > 1;

        // The SIDD spec requires that a J2K compressed SIDDs be only a
        // single image segment. However this functionality remains untested.
        const auto numIS = imageSegments.size();
        const auto imageData_ = imageData_i(std::forward<T>(imageData), i);
        if (isBlocking || (enableJ2K && numIS == 1) || !mCompressionOptions.empty())
        {
            if ((isBlocking || (enableJ2K && numIS == 1)) && (pData->getDataType() == six::DataType::COMPLEX))
            {
                throw except::Exception(Ctxt("SICD does not support blocked or J2K compressed output"));
            }

            writeWithNitro(imageData_, imageSegments, startIndex, *pData);
        }
        else
        {
            writeWithoutNitro(imageData_, imageSegments, startIndex, *pData, doByteSwap);
        }

        const Legend* const legend = getLegend(getContainer().get(), i);
        if (legend)
        {
            addLegend(*legend, static_cast<int>(startIndex + numIS));
        }
    }

    addDataAndWrite(schemaPaths);
}
void NITFWriteControl::save_buffer_list(std::span<const std::byte* const> imageData,
                            nitf::IOInterface& outputFile,
                            const std::vector<std::string>& schemaPaths)
{
    save_T(imageData, outputFile, schemaPaths);
}
template<>
void NITFWriteControl::save_image(std::span<const std::complex<float>> imageData,
                            nitf::IOInterface& outputFile,
                            const std::vector<std::string>& schemaPaths)
{
    save_T(imageData, outputFile, schemaPaths);
}
template<>
void NITFWriteControl::save_image(std::span<const std::pair<uint8_t, uint8_t>> imageData,
                            nitf::IOInterface& outputFile,
                            const std::vector<std::string>& schemaPaths)
{
    save_T(imageData, outputFile, schemaPaths);
}

void NITFWriteControl::addDataAndWrite(const std::vector<std::string>& schemaPaths)
{
    const auto numDES = getContainer()->size();

    // These must stick around until mWriter.write() is called since the
    // SegmentMemorySource's will be pointing to them
    const auto desStrs = std::make_unique<std::string[]>(numDES);

    for (size_t ii = 0; ii < numDES; ++ii)
    {
        const Data* data = getContainer()->getData(ii);

        desStrs[ii] = six::toValidXMLString(data, schemaPaths, mLog, mXMLRegistry);
        nitf::SegmentWriter deWriter = mWriter.newDEWriter(gsl::narrow<int>(ii));
        nitf::SegmentMemorySource segSource(desStrs[ii], 0, 0, false);
        deWriter.attachSource(segSource);
    }

    auto deWriterIndex = gsl::narrow<int>(numDES);
    for (auto segmentWriter : getSegmentWriters())
    {
        mWriter.setDEWriteHandler(deWriterIndex++, segmentWriter);
    }
    mWriter.write();
}

std::string NITFWriteControl::getDesTypeID(const six::Data& data)
{
    return NITFHeaderCreator::getDesTypeID(data);
}

bool NITFWriteControl::needUserDefinedSubheader(const six::Data& data)
{
    return NITFHeaderCreator::needUserDefinedSubheader(data);
}

void NITFWriteControl::setOrganizationId(const std::string& organizationId)
{
    mNITFHeaderCreator->setOrganizationId(organizationId);
}

void NITFWriteControl::setLocationIdentifier(
        const std::string& locationId, const std::string& locationIdNamespace)
{
    mNITFHeaderCreator->setLocationIdentifier(locationId, locationIdNamespace);
}

void NITFWriteControl::setAbstract(const std::string& abstract)
{
    mNITFHeaderCreator->setAbstract(abstract);
}

void NITFWriteControl::addUserDefinedSubheader(
        const six::Data& data, nitf::DESubheader& subheader) const
{
    mNITFHeaderCreator->addUserDefinedSubheader(data, subheader);
}

void NITFWriteControl::addAdditionalDES(
       std::shared_ptr<nitf::SegmentWriter> segmentWriter)
{
    mNITFHeaderCreator->addAdditionalDES(segmentWriter);
}
}

