#pragma once

#include <cstddef>
#include <deque>
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

// What the engine remembers about the text typed so far.
struct TextState {
    bool capitalizeNext = true; // the next letter starts a sentence
    bool spaceNeeded = false;   // a letter typed now needs a space before it
    std::string word;// letters of the word being typed (lowercase)
    bool lastWasDigit = false;  // the previous key was a digit
    bool decimalPending = false;// a '.' right after a digit just arrived
    bool capBeforeDot = false;  // capitalizeNext as it was before that '.'
};

class TextEngine {
public:
    TextAction process(const KeyEvent& event);

private:
    // One entry per key the user typed, so Backspace can undo its effect.
    struct HistoryEntry {
        TextState before; // state before this key was processed
        bool insertedSpace = false;  //the engine added a space before this key
    };

    TextAction processCharacter(char ch, bool& insertedSpace);
    TextAction handleBackspace();
    void remember(const HistoryEntry& entry);

    TextState state_;
    std::deque<HistoryEntry> history_;
    static constexpr std::size_t kMaxHistory = 256;
};