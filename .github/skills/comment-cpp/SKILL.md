---
name: comment-cpp
description: "Use Documentation/README.md comment style guidelines to fix, improve, and document C/C++ code in this project."
---

# Comment C/C++ Code Using Project Guidelines

Use the project's `Documentation/README.md` comment format guide when updating C and C++ code.

When this skill is applied, follow these rules:
- Read and apply the conventions from `Documentation/README.md` for file headers, Doxygen style comments, section headers, and member-variable documentation.
- Preserve existing code behavior and make only necessary comment or style changes.
- Add or improve Doxygen-style comments for public functions, classes, methods, and types.
- Use `@file` and `@brief` in file headers, with `@details` or a second paragraph for longer descriptions.
- Document function parameters with `@param` and return values with `@return` when applicable.
- Use section headers to separate larger logical blocks of implementation.
- Prefer concise, clear comments that explain intent, not implementation details.

## Use When
- updating header or source comments in `*.h`, `*.hpp`, `*.c`, or `*.cpp` files
- converting existing comments to the project’s agreed Doxygen style
- adding documentation for public APIs or device interfaces

## Example Prompts
- "Update the comments in `src/core0/PfeifferGauge.cpp` to follow Documentation/README.md style."
- "Add Doxygen comments to `include/core0/AlicatMFC.h` using the project comment guidelines."
- "Review and fix C++ file headers and function comments in the `src` folder."
