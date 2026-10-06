
# typeAssist

<p align="center">
  <strong>A lightweight, system-wide typing assistant.</strong>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17-blue?style=for-the-badge&logo=cplusplus" alt="C++17">
  <img src="https://img.shields.io/badge/CMake-3.16%2B-064F8C?style=for-the-badge&logo=cmake" alt="CMake">
  <img src="https://img.shields.io/badge/Platform-Windows-0078D4?style=for-the-badge&logo=windows" alt="Windows">
  <img src="https://img.shields.io/badge/Status-Phase%201%20%E2%80%93%20Core%20Engine-orange?style=for-the-badge" alt="Project Status">
</p>

---

## Overview

**typeAssist** is a lightweight typing-assistance project written in modern C++.

The goal is to make common typing corrections happen automatically while the user types, without requiring them to manually fix small formatting mistakes.

The project is built around a platform-independent **text-processing engine** that receives keyboard events, maintains typing state, and produces precise text actions.

The long-term goal is to make typeAssist work system-wide across desktop applications while keeping the core text-processing logic independent from Windows-specific APIs and external AI services.

---

## Features

| Feature | Status |
|---|:---:|
| Sentence capitalization | ✅ |
| Capitalization after `!` and `?` | ✅ |
| Capitalization after Enter | ✅ |
| Automatic spacing after `,` | ✅ |
| Automatic spacing after `;` | ✅ |
| Backspace handling | ✅ |
| Editing history | ✅ |
| Common abbreviation detection | ✅ |
| Decimal-number handling | ✅ |
| URL detection | 🚧 |
| Code-aware text detection | ⏳ |
| Cursor movement | ⏳ |
| Undo support | ⏳ |
| Windows global keyboard hook | ⏳ |
| AI spelling correction | ⏳ |
| Grammar assistance | ⏳ |

---

## Example

Input:

```text
hello,wrold. dr. smith paid 3.5 dollars. he left
next line
```

The engine is designed to produce behavior similar to:

```text
Hello, wrold. Dr. smith paid 3.5 dollars. He left
Next line
```

The core engine currently focuses on deterministic text processing.

AI-powered spelling and grammar correction are planned for a later phase.

---

## Demo

> A visual demonstration will be added once the Windows keyboard integration is complete.

### Planned demo

The final demo will show typeAssist working across real desktop applications:

```text
User types
     │
     ▼
"hello,world. how are you?"
     │
     ▼
typeAssist
     │
     ▼
"Hello, world. How are you?"
```

The goal is for corrections to happen **while typing**, rather than as a separate post-processing step.

---

## Architecture

The project follows a layered architecture:

```text
┌──────────────────────────────┐
│      Desktop Application     │
│ Chrome / VS Code / Word etc. │
└──────────────┬───────────────┘
               │
               ▼
┌──────────────────────────────┐
│     Windows Integration      │
│     Global Keyboard Hook     │
└──────────────┬───────────────┘
               │
               │ KeyEvent
               ▼
┌──────────────────────────────┐
│         TextEngine           │
│                              │
│  • Text state                │
│  • Capitalization            │
│  • Punctuation spacing       │
│  • Abbreviations             │
│  • Decimal handling          │
│  • Editing history           │
└──────────────┬───────────────┘
               │
               │ TextAction
               ▼
┌──────────────────────────────┐
│       Action Executor        │
│                              │
│  PassThrough / Insert /      │
│  Replace / Delete            │
└──────────────┬───────────────┘
               │
               ▼
        Updated Text
```

### Core engine

The central design is:

```text
KeyEvent → TextEngine → TextAction
```

For example, an input event can be represented as:

```cpp
KeyEvent{
    KeyType::Character,
    ','
};
```

The engine can then return an action such as:

```cpp
TextAction{
    ActionType::Insert,
    " "
};
```

The core engine does **not** directly interact with the operating system.

This separation makes the engine easier to test, maintain, and eventually reuse with different input systems.

---

## Design Philosophy

### Platform-independent core

The `TextEngine` should not depend on Windows APIs.

Windows-specific functionality belongs in the integration layer.

### Deterministic text processing

Basic typing rules should be predictable.

For example:

```text
3.5
```

must remain:

```text
3.5
```

and should not accidentally be treated as a sentence boundary.

### Small, explicit actions

The engine communicates changes through a small set of actions:

```text
PassThrough
Insert
Replace
Delete
```

This keeps the core logic independent from the application receiving the input.

### Test before integration

Text-processing rules are implemented and tested independently before introducing the global keyboard hook.

### Extensible by design

The architecture is intended to support more advanced functionality later without rewriting the core engine.

---

## Project Structure

```text
typeAssist/
│
├── include/
│   └── typeassist/
│       └── TextEngine.h
│
├── src/
│   ├── main.cpp
│   └── TextEngine.cpp
│
├── tests/
│   └── TextEngineTests.cpp
│
├── CMakeLists.txt
├── README.md
└── roadmap.md
```

### Important components

#### `include/typeassist/TextEngine.h`

Contains the core types and public interface:

- `KeyType`
- `KeyEvent`
- `ActionType`
- `TextAction`
- `TextState`
- `TextEngine`

#### `src/TextEngine.cpp`

Contains the text-processing logic.

#### `tests/TextEngineTests.cpp`

Contains automated tests for the engine's behavior.

#### `src/main.cpp`

Provides a small demonstration program for exercising the engine.

#### `roadmap.md`

Tracks the project's development phases and upcoming features.

---

## Testing

typeAssist uses CMake and CTest for automated testing.

### Build

```powershell
cmake --build build --config Debug
```

### Run tests

```powershell
ctest --test-dir build -C Debug --output-on-failure
```

The test suite currently covers functionality implemented during Phase 1, including:

- Capitalization
- Punctuation spacing
- Abbreviations
- Decimal numbers
- Enter/new-line behavior
- Backspace and editing history

---

## Building

### Requirements

- C++17 or later
- CMake 3.16+
- A supported C++ compiler
- Windows development environment for the current project setup

### Configure

```powershell
cmake -S . -B build
```

### Build

```powershell
cmake --build build --config Debug
```

### Run tests

```powershell
ctest --test-dir build -C Debug --output-on-failure
```

### Run the demo

```powershell
.\build\Debug\typeassist_demo.exe
```

---

## Tech Stack

| Technology | Purpose |
|---|---|
| **C++17** | Core implementation |
| **CMake** | Build system |
| **CTest** | Automated testing |
| **Windows API** | Planned keyboard integration |
| **AI/API** | Planned intelligent assistance |


---

The idea is simple:

**Build the engine right first. Then make it work everywhere.**

---

## License

License information will be added as the project matures.
