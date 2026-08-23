#pragma once

#include "cdrom_common.hpp"
#include "cue_parser.hpp"

#include <cstddef>
#include <expected>
#include <filesystem>
#include <fstream>
#include <vector>

namespace festation {
    class CDReader {
    public:
        auto openDiscImage(const std::filesystem::path& cuePath, const std::vector<std::filesystem::path>& binPaths) -> std::expected<void, CdFileError>;
        auto openDiscImage(const std::filesystem::path& cuePath) -> std::expected<void, CdFileError>;
        auto closeDiscImage() -> void;
        auto readCdSector(size_t lda, std::vector<std::byte>& sectorBlock) -> void;

    private:
        std::vector<std::filesystem::path> m_binPaths{};
        std::vector<std::ifstream> m_binStreams{};
        std::vector<BinFileInfo> m_binFilesInfo{};
    };
};
