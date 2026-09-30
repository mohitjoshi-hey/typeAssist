# typeAssist Roadmap

A system-wide typing assistant in C++.

```
KeyEvent -> TextEngine -> TextAction
```

The engine is independent of the OS and of AI. Windows and AI plug in around it.

**Current stage:** Phase 0 (setup), nearly done
**Next step:** `TextEngine.h` (core data types), then the capitalization rule

Legend: `[x]` done, `[ ]` to do

---

## Phase 0: Setup and architecture

- [x] Decide architecture (Input layer -> Text Engine -> Action layer)
- [x] Create GitHub repo `typeAssist`
- [x] Write `CMakeLists.txt` (core library + demo executable)
- [ ] Add placeholder `src/` files so the project builds
- [ ] Add `README.md`
- [ ] First commit: project builds successfully

## Phase 1: Core engine (no Windows, no AI)

Core types
- [ ] `KeyEvent`, `KeyType`
- [ ] `TextAction`, `ActionType`
- [ ] `TextState` (what the engine remembers)
- [ ] `TextEngine::process()`

Rules, one at a time, each with tests
- [ ] Capitalize first letter after `.` `!` `?`
- [ ] Punctuation spacing (`hello,world` -> `hello, world`)
- [ ] Backspace / editing behavior
- [ ] Unit tests (`tests/`), enable in CMake

Edge cases to handle
- [ ] Abbreviations (`Mr. Smith`)
- [ ] Decimal numbers (`3.14`)
- [ ] URLs
- [ ] Code (don't "correct" it)
- [ ] Cursor movement
- [ ] Undo

## Phase 2: Windows integration

- [ ] Windows keyboard hook -> `KeyEvent`
- [ ] Action executor (insert / replace / delete / pass-through)
- [ ] Test in Chrome, VS Code, Word, etc.

## Phase 3: AI layer

- [ ] Spelling detection (flag suspicious words)
- [ ] AI API call for corrections
- [ ] Replace misspelled word (`goign` -> `going`)
- [ ] Grammar suggestions

---

## Feature ideas (not committed)

- [ ] Per-app enable/disable
- [ ] Hotkey to pause the assistant
- [ ] User-configurable rules
- [ ] Custom dictionary

---

## Changelog

| Date | Update |
|------|--------|
| 2026-09-30 | Repo created, architecture decided, `CMakeLists.txt` written |