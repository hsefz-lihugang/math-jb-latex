#pragma once
#include <stdio.h>
#include <string.h>

#include <format>

#include "../macros/color.h"
#include "../utils/compile.cpp"
#include "../utils/repo.cpp"
#include "../utils/trie.cpp"

void registerCommitCommand(TrieTree<void (*)(const char *, TrieTree<const char *> &)> &subcommand, TrieTree<const char *> &arguments) {
    arguments.insert("message", strlen("message"), "");
    subcommand.insert("commit", strlen("commit"), [](const char *target, TrieTree<const char *> &arguments) -> void {
        auto lengthOfTarget = strlen(target);

        if (!lengthOfTarget) {
            puts(std::format("{}Target chapter is not specified.{}", USE_COLOR(FCOLOR_RED), USE_COLOR(FCOLOR_WHITE)).c_str());
            return;
        }

        int targetChapter = 0, targetSection = 0;

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

        puts(std::format("Target chapter: {}{}{}, Target section: {}{}{}",
                         USE_COLOR(FCOLOR_YELLOW), targetChapter, USE_COLOR(FCOLOR_WHITE),
                         USE_COLOR(FCOLOR_YELLOW), targetSection, USE_COLOR(FCOLOR_WHITE))
                 .c_str());

        auto message = arguments.get("message", strlen("message"));

        if (repoMeta.size() < targetChapter) repoMeta.resize(targetChapter);
        if (repoMeta[targetChapter - 1].size() < targetSection) repoMeta[targetChapter - 1].resize(targetSection);

        system("git pull");
        system("git config user.name>./workspaceData/git.name");

        auto gitNameFp = fopen("./workspaceData/git.name", "r");
        if (gitNameFp == NULL) {
            puts(std::format("{}Cannot fetch the name registered in git.", USE_COLOR(FCOLOR_RED), USE_COLOR(FCOLOR_WHITE)).c_str());
            return;
        }
        char name[128];
        fscanf(gitNameFp, "%s", name);
        fclose(gitNameFp);

        repoMeta[targetChapter - 1][targetSection - 1].contributors.push_back(name);
        saveRepoMeta();

        system("git add src");
        system("git add workspaceData");
        system(std::format("git commit -m \"Commit Chapter {}, Section {}, by {}: {}\"", targetChapter, targetSection, name, message).c_str());
        system("git push");
    });
}