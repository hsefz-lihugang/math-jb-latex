#pragma once
#include <stdio.h>
#include <string.h>

#include <format>

#include "../macros/color.h"
#include "../utils/compile.cpp"
#include "../utils/trie.cpp"

void registerPreviewCommand(TrieTree<void (*)(const char *, TrieTree<const char *> &)> &subcommand, TrieTree<const char *> &arguments) {
    arguments.insert("cache", strlen("cache"), "enabled");
    arguments.insert("spacing.choice", strlen("spacing.choice"), "0pt");
    arguments.insert("spacing.cloze", strlen("spacing.cloze"), "0pt");
    arguments.insert("spacing.answerQuestion", strlen("spacing.answerQuestion"), "spacing-enabled");

    subcommand.insert("preview", strlen("preview"), [](const char *target, TrieTree<const char *> &arguments) -> void {
        auto lengthOfTarget = strlen(target);

        if (!lengthOfTarget) {
            puts(std::format("{}Target chapter is not specified.{}", USE_COLOR(FCOLOR_RED), USE_COLOR(FCOLOR_WHITE)).c_str());
            return;
        }

        int targetChapter = 0, targetSection = 0;
        auto cacheEnabled = !!strcmp(arguments.get("cache", strlen("cache")), "disabled");

        bool spliterReaded = false;

        for (size_t i = 0; i < lengthOfTarget; i++) {
            if (target[i] == '.') {
                spliterReaded = true;
                continue;
            }
            if (!spliterReaded) {
                targetChapter *= 10;
                targetChapter += target[i] - '0';
            } else {
                targetSection *= 10;
                targetSection += target[i] - '0';
            }
        }

        puts(std::format("Target chapter: {}{}{}, Target section: {}{}{}, Cache Enabled: {}{}{}",
                         USE_COLOR(FCOLOR_YELLOW), targetChapter, USE_COLOR(FCOLOR_WHITE),
                         USE_COLOR(FCOLOR_YELLOW), targetSection, USE_COLOR(FCOLOR_WHITE),
                         USE_COLOR(FCOLOR_YELLOW), cacheEnabled ? "YES" : "NO", USE_COLOR(FCOLOR_WHITE))
                 .c_str());

        if (targetSection)
            compileSingleFile(
                targetChapter, targetSection, cacheEnabled,
                true,
                arguments.get("spacing.choice", strlen("spacing.choice")),
                arguments.get("spacing.cloze", strlen("spacing.cloze")),
                !!strcmp(arguments.get("spacing.answerQuestion", strlen("spacing.answerQuestion")), "spacing-disabled"));
        else
            compileChapter(
                targetChapter, cacheEnabled,
                true,
                arguments.get("spacing.choice", strlen("spacing.choice")),
                arguments.get("spacing.cloze", strlen("spacing.cloze")),
                !!strcmp(arguments.get("spacing.answerQuestion", strlen("spacing.answerQuestion")), "spacing-disabled"));
    });
}