#include <typeassist/TextEngine.h>

#include <iostream>

int main() {
    TextEngine engine;
    KeyEvent event{KeyType::Character, 'h'};
    TextAction action = engine.process(event);
    std::cout << action.text << '\n';
    return 0;
}