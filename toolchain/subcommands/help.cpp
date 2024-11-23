#pragma once
#include <stdio.h>
#include <string.h>

#include <format>

#include "../macros/basic.h"
#include "../utils/trie.cpp"

void registerHelpCommand(TrieTree<void (*)(const char *, TrieTree<const char *> &)> &subcommand, TrieTree<const char *> &arguments) {
    subcommand.insert("help", strlen("help"), [](const char *object, TrieTree<const char *> &arguments) -> void {
        puts(std::format("Math\nA tool to manage math-jb-latex project.\nFor usage, please read README.md in the root.\n\nCompiled with C++ {}\nCompiled Time: {} {}\nColor enabled: {}", __cplusplus, __DATE__, __TIME__, COLOR_ENABLED).c_str());
    });
}