#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

template <class T>
class data_view
{
public:
    using value_type = T;
    using size_type = std::size_t;
    using const_reference = const value_type&;
    using const_pointer = const value_type*;
    using const_iterator = const_pointer;

private:
    const const_pointer Data;
    const size_type Size;

public:
    constexpr data_view() noexcept : Data(nullptr), Size(0){};
    constexpr data_view(const_pointer data, size_type size) noexcept : Data(data), Size(size){};
    data_view(const std::vector<value_type>& source) noexcept : Data(source.data()), Size(source.size()){};

    [[nodiscard]] constexpr const_reference operator[](size_type position) const noexcept
    {
        return Data[position];
    }

    [[nodiscard]] constexpr const_reference at(size_type position) const
    {
        if (position >= Size)
            throw std::out_of_range("data_view<T>::at(size_type) range check");

        return Data[position];
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return Size == 0;
    }

    [[nodiscard]] constexpr size_type size() const noexcept
    {
        return Size;
    }

    [[nodiscard]] constexpr const_pointer data() const noexcept
    {
        return Data;
    }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept
    {
        return Data;
    }

    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
        return Data + Size;
    }

    [[nodiscard]] constexpr const_reference front() const noexcept
    {
        assert(!empty());
        return *Data;
    }

    [[nodiscard]] constexpr const_reference back() const noexcept
    {
        assert(!empty());
        return *(Data + Size - 1);
    }
};

using byte_view = data_view<std::uint8_t>;
