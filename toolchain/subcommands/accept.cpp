#pragma once
#include <stdio.h>
#include <string.h>

#include <format>

#include "../macros/color.h"
#include "../utils/compile.cpp"
#include "../utils/repo.cpp"
#include "../utils/trie.cpp"

void registerAcceptCommand(TrieTree<void (*)(const char *, TrieTree<const char *> &)> &subcommand, TrieTree<const char *> &arguments) {
    subcommand.insert("accept", strlen("accept"), [](const char *target, TrieTree<const char *> &arguments) -> void {
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

        if (repoMeta.size() < targetChapter || targetChapter == 0 || targetSection == 0 || repoMeta[targetChapter - 1].size() < targetSection) {
            puts(std::format("{}Target Chapter {}{}{}, Section {}{}{} is not existed.{}", USE_COLOR(FCOLOR_RED),
                             USE_COLOR(FCOLOR_GREEN), targetChapter, USE_COLOR(FCOLOR_RED),
                             USE_COLOR(FCOLOR_GREEN), targetSection, USE_COLOR(FCOLOR_RED),
                             USE_COLOR(FCOLOR_WHITE))
                     .c_str());
            return;
        }

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

        if (!strcmp(repoMeta[targetChapter - 1][targetSection - 1].contributors[0], name)) {
            puts(std::format("{}You cannot review your own commits.{}", USE_COLOR(FCOLOR_RED), USE_COLOR(FCOLOR_WHITE)).c_str());
            return;
        }

        repoMeta[targetChapter - 1][targetSection - 1].reviewers.push_back(name);
        repoMeta[targetChapter - 1][targetSection - 1].status = verified;
        saveRepoMeta();

        system("git add workspaceData");
        system(std::format("git commit -m \"Accept Chapter {}, Section {}, Reviewed by {}\"", targetChapter, targetSection, name).c_str());
        system("git push");
    });
}