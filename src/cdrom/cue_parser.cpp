#include "cue_parser.hpp"
#include "cdrom_common.hpp"

#include <array>
#include <cassert>
#include <charconv>
#include <fstream>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

static constexpr std::string_view FILE_TOKEN = "FILE";
static constexpr std::string_view TRACK_TOKEN = "TRACK";
static constexpr std::string_view PREGAP_TOKEN = "PREGAP";
static constexpr std::string_view INDEX_TOKEN = "INDEX";

static constexpr std::string_view AUDIO_TRACK_TYPE = "AUDIO";
static constexpr std::string_view CDG_TRACK_TYPE = "CDG";
static constexpr std::string_view MODE1_2048_TRACK_TYPE = "MODE1/2048";
static constexpr std::string_view MODE1_2352_TRACK_TYPE = "MODE1/2352";
static constexpr std::string_view MODE2_2336_TRACK_TYPE = "MODE2/2336";
static constexpr std::string_view MODE2_2352_TRACK_TYPE = "MODE2/2352";
static constexpr std::string_view CDI_2336_TRACK_TYPE = "CDI/2336";
static constexpr std::string_view CDI_2352_TRACK_TYPE = "CDI/2352";

static auto operator>>(std::istream& iss, festation::FileTrackType& trackType) -> std::istream& {
    std::string type;
    iss >> type;

    if (type == AUDIO_TRACK_TYPE) {
        trackType = festation::FileTrackType::AUDIO;
    }
    else if (type == CDG_TRACK_TYPE) {
        trackType = festation::FileTrackType::CDG;
    }
    else if (type == MODE1_2048_TRACK_TYPE) {
        trackType = festation::FileTrackType::MODE1_2048;
    }
    else if (type == MODE1_2352_TRACK_TYPE) {
        trackType = festation::FileTrackType::MODE1_2352;
    }
    else if (type == MODE2_2336_TRACK_TYPE) {
        trackType = festation::FileTrackType::MODE2_2336;
    }
    else if (type == MODE2_2352_TRACK_TYPE) {
        trackType = festation::FileTrackType::MODE2_2352;
    }
    else if (type == CDI_2336_TRACK_TYPE) {
        trackType = festation::FileTrackType::CDI_2336;
    }
    else if (type == CDI_2352_TRACK_TYPE) {
        trackType = festation::FileTrackType::CDI_2352;
    }
    else {
        std::unreachable();
    }

    return iss;
}

static auto ltrim(std::string& str) -> void
{
    auto it = std::ranges::find_if(str, [](unsigned char chr) {
        return !std::isspace(chr);
    });

    str.erase(str.begin(), it);
}

auto festation::parseCueFile(const std::filesystem::path& cuePath) -> std::expected<std::vector<BinFileInfo>, CdFileError>
{
    if (!std::filesystem::exists(cuePath)) {
        return std::unexpected(CdFileError::FileExistsError);
    }

    std::ifstream cueStream(cuePath);

    if (!cueStream.is_open()) {
        return std::unexpected(CdFileError::FileOpeningError);
    }

    std::vector<BinFileInfo> filesInfo{};
    BinFileInfo* binInfoPtr{};
    FileTrackInfo* trackInfoPtr{};
    std::string line{};

    while (std::getline(cueStream, line)) {
        ltrim(line);
        
        if (line.starts_with(FILE_TOKEN)) {
            BinFileInfo binInfo{};

            std::istringstream iss(line);
            std::string _fileToken{};
            std::string fileName{};
            std::string _fileType{};

            iss >> _fileToken >> std::quoted(fileName) >> _fileType;

            binInfo.filename = fileName;
            
            filesInfo.push_back(binInfo);
            binInfoPtr = &filesInfo.back();
        }

        if (line.starts_with(TRACK_TOKEN)) {
            FileTrackInfo trackInfo{};
            std::istringstream iss(line);
            std::string _trackToken{};
            size_t id{};
            FileTrackType trackType{};

            iss >> _trackToken >> id >> trackType;

            trackInfo.id = id;
            trackInfo.type = trackType;

            assert(binInfoPtr);
            binInfoPtr->tracks.push_back(trackInfo);
            trackInfoPtr = &binInfoPtr->tracks.back();
        }

        if (line.starts_with(PREGAP_TOKEN)) {
            std::istringstream iss(line);
            std::string _pregapToken{};
            std::string pregap{};

            iss >> _pregapToken >> pregap;

            constexpr size_t msfElementsCount{3};
            size_t msfIndex{};
            std::array<uint8_t, msfElementsCount> msf{};

            auto msfElements = std::views::split(std::string_view{pregap}, ':');
            assert(std::ranges::distance(msfElements) == msfElementsCount);

            for (const auto& value : msfElements) {
                auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), msf[msfIndex++]);

                if (ec == std::errc::invalid_argument) {
                    return std::unexpected(CdFileError::CueParsingError);
                }
            }

            assert(trackInfoPtr);
            trackInfoPtr->pregap = {
                .address = {
                    .minutes = msf[0],
                    .seconds = msf[1],
                    .sector = msf[2],
                },
                .lda = convertMSFtoLDA(msf[0], msf[1], msf[2]),
            };
        }

        if (line.starts_with(INDEX_TOKEN)) {
            TrackIndexInfo indexInfo{};
            std::istringstream iss(line);
            std::string _indexToken{};
            size_t id;
            std::string location{};

            iss >> _indexToken >> id >> location;

            indexInfo.id = id;

            constexpr size_t msfElementsCount{3};
            size_t msfIndex{};
            std::array<uint8_t, msfElementsCount> msf{};

            auto msfElements = std::views::split(std::string_view{location}, ':');
            assert(std::ranges::distance(msfElements) == msfElementsCount);

            for (const auto& value : msfElements) {
                auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), msf[msfIndex++]);

                if (ec == std::errc::invalid_argument) {
                    return std::unexpected(CdFileError::CueParsingError);
                }
            }

            indexInfo.address = {
                .minutes = msf[0],
                .seconds = msf[1],
                .sector = msf[2],
            };

            indexInfo.lda = convertMSFtoLDA(indexInfo.address);

            assert(trackInfoPtr);
            trackInfoPtr->indices.push_back(indexInfo);
        }
    }

    if (filesInfo.empty()) {
        return std::unexpected(CdFileError::CueParsingError);
    }

    return filesInfo;
}
