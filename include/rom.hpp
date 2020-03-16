#pragma once

#include "instruction.hpp"

#include <string>
#include <vector>

std::vector<uint8_t> LoadFile(const std::string& Path);
std::vector<Instruction> ParseROM(const std::vector<uint8_t>& Data);
