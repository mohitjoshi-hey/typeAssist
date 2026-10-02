#include <typeassist/TextEngine.h>

#include <iostream>
#include <string>
using namespace std;

// Types a string through a fresh engine and returns what the user would see.
static string simulateTyping(const std::string& input) {
    TextEngine engine;
    std::string output;

    for (char c : input) {
        KeyEvent event{c == ' ' ? KeyType::Space : KeyType::Character, c};
        TextAction action = engine.process(event);
        output += (action.type == ActionType::Replace) ? action.text : string(1, c);
    }
    return output;
}

static int failures = 0;

static void check(const std::string& input, const std::string& expected) {
    const string actual = simulateTyping(input);
    if (actual == expected) {
        std::cout << "PASS: \"" << input << "\"\n";
    } else {
        ++failures;
        cout << "FAIL: \"" << input << "\"\n" << "  expected: \"" << expected << "\"\n" << "  actual:   \"" << actual << "\"\n";
    }
}

int main() {
    // Milestone cases
    check("hello. world",         "Hello. World");
    check("hello! how are you?",  "Hello! How are you?");
    check("hello? yes.",          "Hello? Yes.");

    // Extra cases
    check("hello world",          "Hello world"); // only the first word
    check("Hello. World",         "Hello. World");// already capitalized
    check("hello... world",       "Hello... World");// multiple dots
    check("hi. ok. fine",         "Hi. Ok. Fine"); // several sentences

    if (failures == 0) {
        std::cout << "\nAll tests passed.\n";
        return 0;
    }
    std::cout << "\n" << failures << " test(s) failed.\n";
    return 1;
}