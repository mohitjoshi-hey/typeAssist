#include <typeassist/TextEngine.h>

#include <cctype>
#include <string>

using namespace std;

TextAction TextEngine::process(const KeyEvent& event) {
    // Space, Enter, Backspace: the user handled spacing themselves.
    if (event.type != KeyType::Character) {
        spaceNeeded_ = false;
        return {ActionType::PassThrough, ""};
    }

    const unsigned char c = static_cast<unsigned char>(event.character);

    // rule 2: a letter right after , or ; needs a space before it.
    string prefix;
    if (spaceNeeded_) {
        spaceNeeded_ = false;
        if (isalpha(c)) {
            prefix = " ";
        }
    }

    //rule 1: capitalize the first letter of a sentence.
    string text(1, event.character);
    if (capitalizeNext_ && isalpha(c)) {
        capitalizeNext_ = false;
        text[0] = static_cast<char>(toupper(c));
    } else if (c == '.' || c == '!' || c == '?') {
        capitalizeNext_ = true;
    }

    // remember that a space is now expected after this punctuation.
    if (c == ',' || c == ';') {
        spaceNeeded_ = true;
    }

    // Nothing changed: let the user's key through untouched.
    if (prefix.empty() && text[0] == event.character) {
        return {ActionType::PassThrough, ""};
    }
    return {ActionType::Replace, prefix + text};
}