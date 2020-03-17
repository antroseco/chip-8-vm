#pragma once

#include <string>
#include <vector>

std::vector<uint8_t> LoadFile(const std::string& Path);
bool CheckROM(const std::vector<uint8_t>& ROM) noexcept;
