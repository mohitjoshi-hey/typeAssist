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

// What the engine wants to happen for one key.
//   PassThrough: let the user's key through untouched.
//   Replace:delete `erase` characters before the cursor, then type `text`  instead of the user's key (so `text` includes the key itself when the key is a Space or Enter).
struct TextAction {
    ActionType type;
    std::string text;
    std::size_t erase = 0;
};

// What the engine remembers about the text typed so far.
struct TextState {
    bool capitalizeNext = true;// the next letter starts a sentence
    bool spaceNeeded = false;// a letter typed now needs a space before it
    std::string word; // letters of the word being typed (lowercase)
    std::string token;// first characters of the current run of non-space keys
    bool inUrl = false;  // the current token is a link (http://..., www....)
    bool linkChar = false; // the current token contains @ / or \ (email, path, link)
    char prevChar = '\0'; // the previous key in the current token
    bool endPending = false;  // a . ! ? just arrived; a space must follow to confirm it
    bool capBeforeEnd = false;// capitalizeNext as it was before that . ! ?

    // Letters stuck to a . ! ? ("hello.World"): maybe a forgotten space.
    char lastPunct = '\0'; // the . ! ? just before them ('\0' = none)
    bool punctBlocked = false;// it can't be a forgotten space (link, email, number...)
    bool punctEnds = false;// it ended a sentence (false for "Mr.")
    std::string glued; // the letters stuck to it, as typed
    bool gluedSpaced = false;// true if a space was auto-inserted for this glued word
    bool tokenAutoCapitalized = false;// true if first letter was auto-capitalized at start of sentence
};

class TextEngine {
public:
    TextAction process(const KeyEvent& event);

private:
    TextAction step(const KeyEvent& event);
    TextAction processCharacter(char ch, std::size_t& charsAdded, std::size_t& charsErased);
    TextAction handleBackspace();
    void remember(const TextState& before);

    TextState state_;
    // One entry per character on screen: the state before that character,
    // so Backspace can step back exactly one character.
    std::deque<TextState> history_;
    static constexpr std::size_t kMaxHistory = 256;
};