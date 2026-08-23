#include "cd_reader.hpp"
#include "cdrom_common.hpp"

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <ios>
#include <system_error>

static constexpr size_t CD_SECTOR_SIZE = 2352ULL;

auto festation::CDReader::openDiscImage(const std::filesystem::path& cuePath, const std::vector<std::filesystem::path>& binPaths) -> std::expected<void, CdFileError>
{
    if (binPaths.empty()) {
        return std::unexpected(CdFileError::MissingBinFileError);
    }

    m_binStreams.resize(binPaths.size());

    for (size_t fileId = 0; fileId < binPaths.size(); fileId++) {
        const auto& binPath = binPaths[fileId];

        if (!std::filesystem::exists(binPath)) {
            return std::unexpected(CdFileError::FileExistsError);
        } 

        auto& binStream = m_binStreams[fileId];
        binStream.open(binPath);

        if (!binStream.is_open()) {
            return std::unexpected(CdFileError::FileOpeningError);
        }
    }

    auto result = parseCueFile(cuePath);

    if (!result) {
        closeDiscImage();
        return std::unexpected(result.error());
    }

    m_binFilesInfo = result.value();

    m_binFilesInfo[0].ldaStartOffset = 0;

    for (size_t i = 1; i < m_binFilesInfo.size(); i++) {
        std::error_code ec;
        uintmax_t fileSize = std::filesystem::file_size(binPaths[i - 1], ec);

        if (ec) {
            return std::unexpected(CdFileError::FileSizeError);
        }

        m_binFilesInfo[i].ldaStartOffset = (fileSize + m_binFilesInfo[i - 1].ldaStartOffset) / CD_SECTOR_SIZE;
    }

    return {};
}

auto festation::CDReader::openDiscImage(const std::filesystem::path& cuePath) -> std::expected<void, CdFileError>
{
    auto result = parseCueFile(cuePath);

    if (!result) {
        closeDiscImage();
        return std::unexpected(result.error());
    }

    m_binFilesInfo = result.value();

    m_binFilesInfo[0].ldaStartOffset = 0;

    for (size_t i = 1; i < m_binFilesInfo.size(); i++) {
        std::error_code ec;
        uintmax_t fileSize = std::filesystem::file_size(cuePath.parent_path() / m_binFilesInfo[i - 1].filename, ec);

        if (ec) {
            return std::unexpected(CdFileError::FileSizeError);
        }

        m_binFilesInfo[i].ldaStartOffset = (fileSize + m_binFilesInfo[i - 1].ldaStartOffset) / CD_SECTOR_SIZE;
    }

    m_binStreams.resize(m_binFilesInfo.size());

    for(size_t i = 0; i < m_binStreams.size(); i++) {
        const auto& binPath = cuePath.parent_path() / m_binFilesInfo[i].filename;

        auto& binStream = m_binStreams[i];
        binStream.open(binPath);

        if (!binStream.is_open()) {
            return std::unexpected(CdFileError::FileOpeningError);
        }
    }

    return {};
}

auto festation::CDReader::closeDiscImage() -> void
{
    for (auto& binStream : m_binStreams) {
        if (binStream.is_open()) {
            binStream.close();
        }
    }
}

auto festation::CDReader::readCdSector(size_t lda, std::vector<std::byte>& sectorBlock) -> void
{
    size_t fileIndex = m_binFilesInfo.size() - 1;
    size_t startOffset{};

    while (fileIndex >= 0) {
        startOffset = m_binFilesInfo[fileIndex].ldaStartOffset;

        if (lda > startOffset) {
            break;
        }

        fileIndex--;
    }

    auto& binStream = m_binStreams[fileIndex];
    binStream.seekg(lda - startOffset, std::ios_base::beg);
    binStream.read(reinterpret_cast<char*>(sectorBlock.data()), static_cast<std::streamsize>(RAW_SECTOR_SIZE));

    assert(binStream.gcount() == static_cast<std::streamsize>(RAW_SECTOR_SIZE));
}
