#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

class byte_view
{
    const std::uint8_t* const Data;
    const std::size_t Size;

public:
    constexpr byte_view() noexcept : Data(nullptr), Size(0){};
    constexpr byte_view(const std::uint8_t* data, std::size_t size) noexcept : Data(data), Size(size){};
    byte_view(const std::vector<std::uint8_t>& source) noexcept : Data(source.data()), Size(source.size()){};

    [[nodiscard]] constexpr const std::uint8_t& operator[](std::size_t position) const noexcept
    {
        return Data[position];
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return Size == 0;
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        return Size;
    }

    [[nodiscard]] constexpr const std::uint8_t* data() const noexcept
    {
        return Data;
    }
};
