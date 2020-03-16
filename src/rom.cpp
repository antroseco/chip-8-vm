#include "rom.hpp"

#include <arpa/inet.h>
#include <fstream>

std::vector<uint8_t> LoadFile(const std::string& Path)
{
    /*std::ifstream File(Path, std::ios::in | std::ios::binary);

    if (!File)
        throw std::runtime_error("Unable to open " + Path);

    const std::istreambuf_iterator<char> Iterator(File);
    return std::vector<uint8_t>(Iterator, {});*/

    std::ifstream inFile(Path, std::ios_base::binary);

    inFile.seekg(0, std::ios_base::end);
    size_t length = inFile.tellg();
    inFile.seekg(0, std::ios_base::beg);

    std::vector<uint8_t> buffer;
    buffer.reserve(length);
    std::copy(std::istreambuf_iterator<char>(inFile),
              std::istreambuf_iterator<char>(),
              std::back_inserter(buffer));

    return buffer;
}

std::vector<Instruction> ParseROM(const std::vector<uint8_t>& Data)
{
    auto DataPointer = Data.data();

    std::vector<Instruction> Buffer;
    Buffer.reserve(Data.size() / 2);

    for (std::size_t i = 0; i < Data.size(); i += 2)
    {
        auto Short = reinterpret_cast<const uint16_t*>(std::next(DataPointer, i));
        Buffer.emplace_back(htons(*Short));
    }

    return Buffer;
}
