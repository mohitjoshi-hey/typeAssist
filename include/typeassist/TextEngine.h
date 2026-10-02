#pragma once

#include <string>

enum class KeyType {
    Character,
    Space,
    Backspace,
    Enter
};

struct KeyEvent {
    KeyType type;
    char character = '\0';
};

enum class ActionType {
    PassThrough,
    Replace,
    Insert,
    Delete
};

struct TextAction {
    ActionType type;
    std::string text;
};

class TextEngine {
public:
    TextAction process(const KeyEvent& event);

private:
    bool capitalizeNext_ = true;
    bool spaceNeeded_ = false;
};