#include "rom.hpp"

#include <arpa/inet.h>
#include <fstream>

std::vector<uint8_t> LoadFile(const std::string& Path)
{
    std::ifstream File(Path, std::ios::in | std::ios::binary);

    if (!File)
        throw std::runtime_error("Unable to open " + Path);

    const std::istreambuf_iterator<char> Iterator(File);
    return std::vector<uint8_t>(Iterator, {});
}

bool CheckROM(const std::vector<uint8_t>& ROM) noexcept
{
    return ROM.size() < 0xDFF;
}
