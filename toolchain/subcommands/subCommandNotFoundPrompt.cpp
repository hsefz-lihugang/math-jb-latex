#pragma once
#include <stdio.h>
#include <string.h>

#include <format>

#include "../macros/basic.h"
#include "../utils/trie.cpp"

void registerSubCommandNotFoundError(TrieTree<void (*)(const char *, TrieTree<const char *> &)> &subcommand, TrieTree<const char *> &arguments) {
    subcommand.insert("::subCommandNotFound", strlen("::subCommandNotFound"), [](const char *object, TrieTree<const char *> &arguments) -> void {
        puts(std::format("Subcommand {} was not declared.\nPlease see README.md for usage.", object).c_str());
    });
}