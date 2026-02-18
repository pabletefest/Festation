#include "file_reader.hpp"

auto festation::readFileAsString(std::filesystem::path filepath) -> std::string
{
        std::ifstream fileStream{ filepath, std::ios::binary };
        return { std::istreambuf_iterator{fileStream}, {} };
}
