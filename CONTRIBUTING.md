# Contributing to SOLAN

Thanks for taking a look at this project. It started as a personal C++ learning exercise, so the code and the process here are both still fairly informal. That said, contributions, bug reports, and suggestions are genuinely welcome.

## Before you start

This project has no automated test suite yet. Every feature so far has been verified by hand: building on both Windows and Linux, running a real transfer between two machines, and checking the output. If you're contributing code, please do the same before opening a pull request. Describe what you tested and on what platforms in the PR description, since there's no CI test job to catch regressions for you.

CI (GitHub Actions) currently only confirms the project *builds* on `ubuntu-latest` and `windows-latest`. A green check mark means it compiles, not that it works correctly.

## Setting up

Build instructions are in the [README](README.md). Short version:

```bash
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Dependencies (Asio, PicoSHA2) are fetched automatically through CMake's `FetchContent`, so there's nothing to install by hand.

## Code style

- C++17, no newer standard features yet.
- Header/source pairs, one class or one logical unit per file.
- Prefer `throw`ing exceptions on failure over returning error codes or printing and continuing. There's a single top-level `try/catch` in `main()` and in the shell's mode handlers that's meant to catch everything.
- Keep new networking code inside `src/net/`, and anything purely about terminal output inside `src/ui/`. `main.cpp` and `shell.cpp` are the only files that should know about both.

## Making changes

1. Fork the repo and create a branch for your change.
2. Keep pull requests focused. One feature or one fix per PR, rather than several bundled together, makes it much easier to review and to figure out what broke if something does.
3. Write a clear commit message describing what changed and why, not just what files were touched.
4. Test your change on at least one platform yourself before opening the PR, and mention which one(s) in the description.
5. Open the PR against `main`.

## Reporting bugs

Open an issue with:
- What you ran (exact command or shell sequence)
- What you expected to happen
- What actually happened, including the exact error text if there was one
- Your OS and how you built the project

If you're not sure whether something is a bug or expected behavior, open an issue anyway. It's easier to close a non-issue than to lose a real one.

## Known limitations, if you're looking for something to work on

- No automated tests
- No encryption on the wire
- Peer discovery doesn't currently show MAC addresses
- No installer yet for either platform
- Receive mode can only be interrupted between transfers, not while idle and waiting for a connection (this is a real limitation of using blocking sockets rather than async I/O)

Any of these would be a solid, well-scoped first contribution.
