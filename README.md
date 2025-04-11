# Text Mode NES Tetris
A recreation of NES Tetris written in kernel-style C23 targeting text mode Linux terminals.

## Motivation
This project is mostly made to practice exercising good code quality and best practices for C programming. It follows the [Linux Kernel coding style](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/Documentation/process/coding-style.rst). Some inspiration is also taken from ["Writing C for curl"](https://daniel.haxx.se/blog/2025/04/07/writing-c-for-curl/), though the [Linux Kernel coding style](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/Documentation/process/coding-style.rst) always takes precedent, for example in the case of indentation. Another notable exception is the use of C23 as opposed to C89 which `curl` restricts itself to.

Due to this focus on **best practices** and **maintainability**, issues or pull requests relating solely to functionality, e.g. furthering platform support, will have a very low priority and may be ignored.

## What this project is not
* **This project is not the most faithful possible adaptation of NES Tetris.** The main reason for this project to mention specifically NES Tetris is the simplicity of its mechanics and thus easier implementation. It also borrows from NES Tetris in some necessary game design choices such as scoring and gravity speeds, but it does not replicate aesthetic or unusual properties of NES Tetris such as the specific colour schemes or the rebirth mechanic. If you want the most faithful experience possible, you should probably just use an NES emulator.
* **This project is (probably) not the Tetris clone you're looking for.** Support for other platforms than Linux is not something that's planned to ever be implemented, which depending on your use case may already be a dealbreaker. Furthermore, this project does not use any text-user interface libraries whatsoever, meaning it might not handle all terminals or terminal emulators elegantly.

## What this project is
* **A learning opportunity.** First and foremost, for me. You might however also find it a useful reference for improving your own C code.
* **A project management playground**. The nature of this project as a small and, being written in C, having very few dependencies makes it the perfect base for trying out different ways to configure a workspace. Currently, the project is compiled solely with `gcc`. In the future, I might add a CMake configuration to build the project that way. There's much more to try here than just configuring the build system; try [Dev Containers](https://containers.dev/), try whatever you want!

## Building the project
> [!NOTE]
> The project is still a work in progress. Build instructions will be added at a later point in time.