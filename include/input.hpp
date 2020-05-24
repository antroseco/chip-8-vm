#include <SFML/Window.hpp>

#include <array>
#include <atomic>
#include <optional>

class Keyboard
{
    std::array<std::atomic_bool, 16> keys = {};

public:
    Keyboard() = delete;
    Keyboard(sf::Window& window);

    void register_keypress(sf::Event::KeyEvent event, bool state) noexcept;
    bool query_key(int key) const noexcept;
    std::optional<int> query_any() const noexcept;
};
