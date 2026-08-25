#pragma once

#include <array>

namespace festation {
    static constexpr std::array<size_t, 2> CD_SECTOR_SIZES = { 0x800, 0x924 };
    static constexpr size_t RAW_SECTOR_SIZE = 0x930;
    
    enum class CdFileError {
        FileExistsError,
        FileOpeningError,
        MissingBinFileError,
        CueParsingError,
        FileSizeError,
    };

    struct MSFFormat {
        uint8_t minutes;
        uint8_t seconds;
        uint8_t sector;
    };

    union CdSectorData {
        struct Mode2 {
            std::byte sync[0x0C];
            std::byte header[0x04];
            std::byte subHeader[0x04];
            std::byte subHeaderCopy[0x04];

            union {
                struct Form1 {
                    std::byte data[0x800];
                    std::byte edc[0x04];
                    std::byte ecc [0x114];
                } form1;

                struct Form2 {
                    std::byte data[0x914];
                    std::byte edc[0x04];
                } form2;
            };
        } mode2;

        std::byte raw[0x930];
    };

    inline constexpr auto convertBCDtoBinary(uint8_t numberBCD) -> uint8_t
    {
        return (numberBCD & 0xF) + ((numberBCD >> 4) & 0xF) * 10;
    }

    inline constexpr auto isValidBCD(uint8_t numberBCD) -> bool
    {
        return ((numberBCD & 0xF) <= 9) && (((numberBCD >> 4) & 0xF) <= 9);
    }

    inline constexpr auto convertMSFtoLDA(uint8_t minutes, uint8_t seconds, uint8_t sector) -> size_t
    {
        return (((minutes * 60) + seconds) * 75 + sector);
    }

    inline constexpr auto convertMSFtoLDA(const MSFFormat& msf) -> size_t
    {
        return convertMSFtoLDA(msf.minutes, msf.seconds, msf.sector);
    }
};
