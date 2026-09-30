#include <typeassist/TextEngine.h>

TextAction TextEngine::process(const KeyEvent& event) {
    // Placeholder: let everything through unchanged.
    return {ActionType::PassThrough, std::string(1, event.character)};
}