
# typeAssist

<p align="center">
  <strong>A lightweight, system-wide typing assistant built in modern C++.</strong>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17-blue?style=for-the-badge&logo=cplusplus" alt="C++17">
  <img src="https://img.shields.io/badge/CMake-3.16%2B-064F8C?style=for-the-badge&logo=cmake" alt="CMake">
  <img src="https://img.shields.io/badge/Platform-Windows-0078D4?style=for-the-badge&logo=windows" alt="Windows">
  <img src="https://img.shields.io/badge/Status-Phase%201%20%E2%80%93%20Core%20Engine-orange?style=for-the-badge" alt="Project Status">
</p>

<p align="center">
  <em>Type naturally. Let typeAssist handle the small things.</em>
</p>

---

## 🚀 Overview

**typeAssist** is a lightweight typing-assistance project written in modern C++.

The goal is to make common typing corrections happen automatically while the user types, without requiring them to manually fix small formatting mistakes.

The project is built around a platform-independent **text-processing engine** that receives keyboard events, maintains typing state, and produces precise text actions.

The long-term goal is to make typeAssist work system-wide across desktop applications while keeping the core text-processing logic independent from Windows-specific APIs and external AI services.

---

## ✨ Features

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

> **Legend:**  
> ✅ Implemented  
> 🚧 Current focus  
> ⏳ Planned

---

## 🎯 Example

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

## 🖥️ Demo

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

## 🏗️ Architecture

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

## 🧠 Design Philosophy

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

## 📁 Project Structure

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

## 🧪 Testing

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

## 🔨 Building

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

## 🗺️ Roadmap

typeAssist is being developed incrementally.

### Phase 0 — Foundation

- [x] Project architecture
- [x] GitHub repository
- [x] CMake build system
- [x] Initial project structure
- [x] First working build

### Phase 1 — Core Text Engine

- [x] `KeyEvent`
- [x] `TextAction`
- [x] `TextState`
- [x] Sentence capitalization
- [x] `,` spacing
- [x] `;` spacing
- [x] Backspace handling
- [x] Editing history
- [x] Common abbreviation handling
- [x] Decimal-number handling
- [x] Enter/new-line capitalization
- [x] Unit tests
- [ ] URL detection
- [ ] Code-aware text detection
- [ ] Initials and dotted abbreviations
- [ ] Cursor movement
- [ ] Undo support

### Phase 2 — Windows Integration

- [ ] Windows global keyboard hook
- [ ] Convert keyboard input into `KeyEvent`
- [ ] Execute `TextAction`
- [ ] Test across desktop applications
- [ ] Test with Chrome
- [ ] Test with VS Code
- [ ] Test with Microsoft Word

### Phase 3 — AI Assistance

- [ ] AI-powered spelling detection
- [ ] Context-aware correction
- [ ] Minimal-context API requests
- [ ] Replacement handling
- [ ] Grammar assistance
- [ ] Privacy-conscious request handling

---

## 🔭 Current Focus

### URL Detection

The next major feature is URL detection.

The engine needs to recognize URLs such as:

```text
example.com
www.example.com
https://example.com
http://example.com
```

and avoid treating punctuation inside URLs as normal sentence punctuation.

For example:

```text
Visit https://example.com
```

should remain unchanged.

This feature is important before expanding punctuation handling to characters such as:

```text
.
!
?
:
```

because those characters can legitimately appear inside URLs.

---

## 🤖 Future AI Integration

AI functionality will remain separate from the deterministic core engine.

The planned architecture is:

```text
Keyboard Input
      │
      ▼
Windows Integration
      │
      ▼
   TextEngine
      │
      ├──────────────► Normal TextAction
      │
      ▼
 AI Assistance Layer
      │
      ▼
Correction / Suggestion
```

The goal is not to send an entire document to an AI service.

Instead, the future system should send only the relevant word or surrounding context needed for a correction.

This keeps the system efficient and makes privacy an important part of the design.

---

## 🛠️ Technology

| Technology | Purpose |
|---|---|
| **C++17** | Core implementation |
| **CMake** | Build system |
| **CTest** | Automated testing |
| **Windows API** | Planned keyboard integration |
| **AI/API** | Planned intelligent assistance |

---

## 📊 Development Status

```text
Phase 0
Foundation
████████████████████ 100%

Phase 1
Core Engine
████████████████░░░░  ~80%

Phase 2
Windows Integration
░░░░░░░░░░░░░░░░░░░░   0%

Phase 3
AI Assistance
░░░░░░░░░░░░░░░░░░░░   0%
```

> Progress percentages are approximate and represent development direction rather than strict task completion.

---

## 📌 Project Philosophy

typeAssist is being built from the inside out.

Instead of starting with a Windows keyboard hook and putting all the logic inside it, the project first builds a reliable and testable text-processing engine.

```text
Reliable Core
      ↓
Edge Cases
      ↓
OS Integration
      ↓
Real-world Testing
      ↓
AI Assistance
```

The idea is simple:

**Build the engine right first. Then make it work everywhere.**

---

## 📄 License

License information will be added as the project matures.
```

One important change from the earlier README: I’ve **not marked URL detection as completed**. Your current roadmap explicitly has it as the next Phase 1 task, so the README should accurately reflect the repository rather than oversell it.

You can paste that entire block directly into `README.md`.