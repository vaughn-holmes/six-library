/* =========================================================================
 * This file is part of cphd-c++
 * =========================================================================
 *
 * (C) Copyright 2004 - 2019, MDA Information Systems LLC
 *
 * cphd-c++ is free software; you can redistribute it and/or modify
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
#ifndef SIX_cphd_CPHDWriter_h_INCLUDED_
#define SIX_cphd_CPHDWriter_h_INCLUDED_

#include <string>
#include <vector>
#include <std/span>
#include <std/cstddef>
#include <functional>

#include <types/RowCol.h>
#include <io/FileOutputStream.h>
#include <sys/OS.h>
#include <sys/Span.h>

#include <scene/sys_Conf.h>

#include <cphd/Types.h>
#include <cphd/FileHeader.h>
#include <cphd/Metadata.h>
#include <cphd/PVP.h>
#include <cphd/PVPBlock.h>
#include <cphd/DataWriter.h>

namespace cphd
{

/*
 *  \class CPHDWriter
 *  \brief CPHD write handler
 *
 *  Used to write a CPHD file. You must be able to provide the
 *  appropriate metadata and vector based metadata.
 */
struct CPHDWriter final
{
    /*
     *  \func Constructor
     *  \brief Sets up the internal structure of the CPHDWriter
     *
     *  The default argument for numThreads should be std::thread::hardware_concurrency().
     *  However, SWIG doesn't seem to like that.
     *  As a workaround, we pass in 0 for the default, and the ctor sets the
     *  number of threads to the number of CPUs if this happens.
     *
     *  \param metadata A filled out metadata struct for the file that will be
     *         written. The data.arraySize and data.numCPHDChannels will be
     *         filled in internally. All other data must be provided.
     *  \param stream Seekable output stream to be written to
     *  \param schemaPaths (Optional) A vector of XML schema paths for validation
     *  \param numThreads (Optional) The number of threads to use for processing.
     *  \param scratchSpaceSize (Optional) The maximum size of internal scratch space
     *         that may be used if byte swapping is necessary.
     *         Default is 4 MB
     */
    CPHDWriter(
            const Metadata& metadata,
            std::shared_ptr<io::SeekableOutputStream> stream,
            const std::vector<std::string>& schemaPaths = std::vector<std::string>(),
            size_t numThreads = 0,
            size_t scratchSpaceSize = 4 * 1024 * 1024);

    /*
     *  \func Constructor
     *  \brief Sets up the internal structure of the CPHDWriter
     *
     *  The default argument for numThreads should be std::thread::hardware_concurrency().
     *  However, SWIG doesn't seem to like that.
     *  As a workaround, we pass in 0 for the default, and the ctor sets the
     *  number of threads to the number of CPUs if this happens.
     *
     *  \param metadata A filled out metadata struct for the file that will be
     *         written. The data.arraySize and data.numCPHDChannels will be
     *         filled in internally. All other data must be provided.
     *  \param pathname The file path to be written to
     *  \param schemaPaths (Optional) A vector of XML schema paths for validation
     *  \param numThreads (Optional) The number of threads to use for processing.
     *  \param scratchSpaceSize (Optional) The maximum size of internal scratch space
     *         that may be used if byte swapping is necessary.
     *         Default is 4 MB
     */
    CPHDWriter(
            const Metadata& metadata,
            const std::string& pathname,
            const std::vector<std::string>& schemaPaths = std::vector<std::string>(),
            size_t numThreads = 0,
            size_t scratchSpaceSize = 4 * 1024 * 1024);

    CPHDWriter() = delete;
    CPHDWriter(const CPHDWriter&) = delete;
    CPHDWriter& operator=(const CPHDWriter&) = delete;
    CPHDWriter(CPHDWriter&&) = delete;
    CPHDWriter& operator=(CPHDWriter&&) = delete;
    ~CPHDWriter() = default;

    /*
     *  \brief The type of "write" function.
     *
     * While the signature is the same, the meaning of `size_t` varies for different
     * functions; for example, it is an `index` for `writePVPData()`.
     */
    using WriteImplFunc_t = std::function<void(const std::byte*, size_t)>;

    std::unique_ptr<DataWriter> make_DataWriter() const;

    /*
     *  \func write
     *  \brief Writes the complete CPHD into the file.
     *
     *  This only works with valid CPHDWriter data types:
     *      std:: ubyte*  (for compressed data)
     *      cphd::zfloat
     *      chpd::zint16_t
     *      cphd::zint8_t
     *
     *  \param pvpBlock The vector based metadata to write.
     *  \param widebandData .The wideband data to write to disk
     *  \param supportData (Optional) The support array data to write to disk.
     */
    template<typename T>
    void write(const PVPBlock& pvpBlock, const T* widebandData,
        std::span<const std::byte> supportData);
    template<typename T>
    void write(const PVPBlock& pvpBlock, const T* widebandData)
    {
        write(pvpBlock, widebandData, std::span<const std::byte>());
    }

    /*
     *  \func writeMetadata
     *  \brief Writes the header, and metadata into the file.
     *
     *  This should be used in situations where you need to write out the CPHD
     *  data in chunks.
     *
     *  \param pvpBlock The vector based metadata to write.
     */
    void writeMetadata(const PVPBlock& pvpBlock);

    /*
     *  \func writeSupportData
     *  \brief Writes the specified support Array to the file
     *
     *  Does not include padding.
     *
     *  \param data A pointer to the start of the support array that
     *        will be written to file
     *  \param id The unique identifier of the support array
     */
    template <typename T>
    void writeSupportData(const T* data_,
        const std::string& id)
    {
        const auto size = mMetadata.data.getSupportArrayById(id).size_bytes();
        const auto data = sys::make_span<const std::byte>(data_, size);
        writeSupportDataImpl(data, mMetadata.data.getSupportArrayById(id).bytesPerElement);
    }

    /*
     *  \func writeSupportData
     *  \brief Writes all of the support Arrays to the file
     *
     *  Includes padding
     *
     *  \param data A pointer to the start of the support array data block
     */
    static void writeSupportData(io::SeekableOutputStream&,
        const FileHeader&, std::function<void(std::span<const std::byte>, size_t)> writeSupportDataImpl,
        const std::unordered_map<std::string, cphd::Data::SupportArray>&, std::span<const std::byte>);
    void writeSupportData(std::span<const std::byte>);
    template <typename T>
    void writeSupportData(std::span<const T> data)
    {
        writeSupportData(std::as_bytes(data));
    }
    template <typename T>
    void writeSupportData(const std::vector<T>& data)
    {
        writeSupportData(sys::make_span(data));
    }

    /*
     *  \func getWriteSupportDataImpl
     *  \brief Returns the writeSupportDataImpl() function.
     * 
     * Returns the writeSupportDataImpl() function; use for writing
     * the data in pieces.  Note that `this` must remain in-scope.
     */
    std::function<void(std::span<const std::byte>, size_t)> getWriteSupportDataImpl();

    /*
     *  \func writePVPData
     *  \brief Writes the PVP to the file
     *
     *  \param PVPBlock A populated PVPBlock object that will be written
     *  to the file as a block of data
     */
    void writePVPData(const PVPBlock& PVPBlock);
    static void writePVPData(io::SeekableOutputStream&, const FileHeader&,
        const cphd::Data&, std::function<void(std::span<const std::byte>, size_t)> writePVPData,
        const PVPBlock&);

    /*
     *  \func getWritePVPData
     *  \brief Returns the writePVPData() function.
     *
     * Returns the writePVPData() function; use for writing
     * the data in pieces.  Note that `this` must remain in-scope.
     */
    std::function<void(std::span<const std::byte>, size_t)> getWritePVPData();

    /*
     *  \func writeCPHDData
     *  \brief Writes a chunk of CPHD data to disk. To create a proper
     *  CPHD file you must call writeMetadata and writePVPData before
     *  using this method. This only works with
     *  valid CPHDWriter data types:
     *      std:: ubyte*  (for compressed data)
     *      cphd::zfloat
     *      chpd::zint16_t
     *      cphd::zint8_t
     *
     *  \param data The data to write to disk.
     *  \param numElements The number of elements in data. Treat the data
     *  as complex when computing the size (do not multiply by 2
     *  for correct byte swapping this is done internally).
     *  \param channel For selecting channel of compressed signal block
     */
    template <typename T>
    void writeCPHDData(const T* data,
                       size_t numElements,
                       size_t channel = 1);

    /*
     *  \func getWriteCPHDDataImpl
     *  \brief Returns the writeCPHDDataImpl() function.
     *
     * Returns the writeCPHDDataImpl() function; use for writing
     * the data in pieces.  Note that `this` must remain in-scope.
     */
    WriteImplFunc_t getWriteCPHDDataImpl();
    WriteImplFunc_t getWriteCompressedCPHDDataImpl();

    std::shared_ptr<io::SeekableOutputStream> getStream() const
    {
        return mStream;
    }
    void close()
    {
        getStream()->close();
    }

private:
    /*
     *  Write metadata helper
     */
    void writeMetadata(
        size_t supportSize, // Optional
        size_t pvpSize,
        size_t cphdSize);

    /*
     *  Write pvp helper
     *  Implementation of write wideband
     *  Implementation of write compressed wideband
     *  Implementation of write support data
     */
    void writePVPData(std::span<const std::byte> pvpBlock, size_t index);
    void writeCPHDDataImpl(const std::byte* data, size_t size);
    void writeCompressedCPHDDataImpl(const std::byte* data, size_t channel);
    void writeSupportDataImpl(std::span<const std::byte>, size_t elementSize);

    // Book-keeping element
    //! metadata information
    const Metadata& mMetadata;
    //! size of each element in signal block
    const size_t mElementSize;
    //! size of scratch space for byte swapping
    const size_t mScratchSpaceSize;
    //! number of threads for parallelism
    const size_t mNumThreads;
    //! schemas for XML validation
    const std::vector<std::string> mSchemaPaths;
    //! Output stream contains CPHD file
    std::shared_ptr<io::SeekableOutputStream> mStream;
    //! header information
    FileHeader mHeader;
};
}

#endif // SIX_cphd_CPHDWriter_h_INCLUDED_
