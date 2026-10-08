#include <typeassist/TextEngine.h>

#include <algorithm>
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

    // Typo "wrold" fixed with Backspace; "dr.", "3.5", "example.com" and a
    // link with a comma in it are left alone; "good!thanks" gets its space.
    const string input =
        "hello,wrold\b\b\b\borld. dr. smith paid 3.5 dollars. he left\n"
        "see example.com or www.site.com/a,b\n"
        "all good!thanks for waiting";
    string screen;

    for (char c : input) {
        KeyEvent event{keyTypeFor(c), c};
        TextAction action = engine.process(event);

        if (event.type == KeyType::Backspace) {
            if (!screen.empty()) screen.pop_back();  // app deletes one char
        } else if (action.type == ActionType::Replace) {
            // delete `erase` characters, then type the replacement text
            screen.erase(screen.size() - min(action.erase, screen.size()));
            screen += action.text;
        } else {
            screen += c;                             // engine let the key through
        }
    }

    cout << screen << '\n';
    return 0;
}