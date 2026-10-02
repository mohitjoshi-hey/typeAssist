#include <typeassist/TextEngine.h>
#include <iostream>
#include <string>

using namespace std;

int main() {
    TextEngine engine;
    const string input = "hello,world. this is a test";
    string output;

    for (char c : input) {
        KeyEvent event{c == ' ' ? KeyType::Space : KeyType::Character, c};
        TextAction action = engine.process(event);

        if (action.type == ActionType::Replace) {
            output += action.text;
        } else {
            output += c;
        }
    }

    cout << output << '\n';
    return 0;
}