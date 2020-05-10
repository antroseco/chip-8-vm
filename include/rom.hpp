#pragma once

#include <cstdint>
#include <string>
#include <vector>

std::vector<std::uint8_t> LoadFile(const std::string& Path);
bool CheckROM(const std::vector<std::uint8_t>& ROM) noexcept;
