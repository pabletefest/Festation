#pragma once

#include "cdrom_common.hpp"

#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace festation {
    enum class FileTrackType {
        AUDIO,
        CDG,
        MODE1_2048,
        MODE1_2352,
        MODE2_2336,
        MODE2_2352,
        CDI_2336,
        CDI_2352
    };

    struct TrackPregapInfo {
        MSFFormat address;
    };

    struct TrackIndexInfo {
        size_t id;
        MSFFormat address;
    };

    struct FileTrackInfo {
        size_t id;
        FileTrackType type;
        std::optional<TrackPregapInfo> pregap;
        std::vector<TrackIndexInfo> indices;
    };

    struct BinFileInfo {
        std::string filename;
        std::vector<FileTrackInfo> tracks;
    };

    auto parseCueFile(const std::filesystem::path& cuePath) -> std::expected<std::vector<BinFileInfo>, CdFileError>;
};
