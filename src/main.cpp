#include <typeassist/TextEngine.h>

#include <iostream>
#include <string>

using namespace std;

static KeyType keyTypeFor(char c) {
    if (c == ' ') return KeyType::Space;
    if (c == '\b') return KeyType::Backspace;
    if (c == '\n') return KeyType::Enter;
    return KeyType::Character;
}

int main() {
    TextEngine engine;

    // Typo "wrold" fixed with Backspace; "dr." and "3.5" are not sentence ends.
    const string input =
        "hello,wrold\b\b\b\borld. dr. smith paid 3.5 dollars. he left\nnext line";
    string screen;

    for (char c : input) {
        KeyEvent event{keyTypeFor(c), c};
        TextAction action = engine.process(event);

        if (event.type == KeyType::Backspace) {
            if (!screen.empty()) screen.pop_back();  // app deletes one char
        } else if (action.type == ActionType::Replace) {
            screen += action.text; // engine replaced the key
        } else {
            screen += c;// engine let the key through
        }
    }

    cout << screen << '\n';
    return 0;
}