#include <typeassist/TextEngine.h>
#include <iostream>
#include <string>

int main() {
    TextEngine engine;
    const std::string input = "hello. this is a test";
    std::string output;

    for (char c : input) {
        KeyEvent event{c == ' ' ? KeyType::Space : KeyType::Character, c};
        TextAction action = engine.process(event);

        if (action.type == ActionType::Replace) {
            output += action.text;// engine replaced the key
        } else {
            output += c; //engine let the key through
        }
    }

    std::cout << output << '\n';
    return 0;
}