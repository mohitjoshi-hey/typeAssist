#include <typeassist/TextEngine.h>

#include <cctype>

TextAction TextEngine::process(const KeyEvent& event) {
    // Only characters affect capitalization for now.
    if (event.type != KeyType::Character) {
        return {ActionType::PassThrough, ""};
    }

    const unsigned char c = static_cast<unsigned char>(event.character);

    // First letter of a sentence: capitalize it.
    if (capitalizeNext_ && std::isalpha(c)) {
        capitalizeNext_ = false;

        const unsigned char upper =
            static_cast<unsigned char>(std::toupper(c));

        if (upper == c) {
            return {ActionType::PassThrough, ""};  //already a capital
        }
        return {
            ActionType::Replace, std::string(1, static_cast<char>(upper))
        };
    }

    // a sentence ended: the next letter should be capitalized.
    if (c == '.' || c == '!' || c == '?') {
        capitalizeNext_ = true;
    }

    return {ActionType::PassThrough, ""};
}