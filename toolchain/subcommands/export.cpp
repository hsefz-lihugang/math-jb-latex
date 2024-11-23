#pragma once
#include <cJSON/cJSON.h>
#include <curl/curl.h>
#include <stdio.h>
#include <string.h>

#include <format>
#include <string>
#include <utility>
#include <vector>

#include "../macros/color.h"
#include "../utils/compile.cpp"
#include "../utils/itoa.cpp"
#include "../utils/trie.cpp"

std::string responseBuffer;

static size_t responseReceiver(const void *receiveBuffer) {
    responseBuffer.append((char *)receiveBuffer);
    return strlen((char *)receiveBuffer);
};

void registerExportCommand(TrieTree<void (*)(const char *, TrieTree<const char *> &)> &subcommand, TrieTree<const char *> &arguments) {
    arguments.insert("spacing.choice", strlen("spacing.choice"), "0pt");
    arguments.insert("spacing.cloze", strlen("spacing.cloze"), "0pt");
    arguments.insert("spacing.answerQuestion", strlen("spacing.answerQuestion"), "spacing-enabled");

    subcommand.insert("export", strlen("export"), [](const char *target, TrieTree<const char *> &arguments) -> void {
        auto lengthOfTarget = strlen(target);

        int targetChapter = 0, targetSection = 0;

        if (target[0] != 'f') {  // full
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
        }

        puts(std::format("Target chapter: {}{}{}, Target section: {}{}{}",
                         USE_COLOR(FCOLOR_YELLOW), targetChapter, USE_COLOR(FCOLOR_WHITE),
                         USE_COLOR(FCOLOR_YELLOW), targetSection, USE_COLOR(FCOLOR_WHITE),
                         USE_COLOR(FCOLOR_WHITE))
                 .c_str());

        auto requestHandle = curl_easy_init();
        if (!requestHandle) {
            puts(std::format("{}Cannot request CURL handle.{}", USE_COLOR(FCOLOR_RED), USE_COLOR(FCOLOR_WHITE)).c_str());
            return;
        }

        curl_easy_setopt(requestHandle, CURLOPT_URL, "https://api.github.com/repos/lihugang/math-jb-latex/contributors");

        struct curl_slist *requestHeaders = NULL;
        requestHeaders = curl_slist_append(requestHeaders, "User-Agent: curl 7.81.0");
        curl_easy_setopt(requestHandle, CURLOPT_HTTPHEADER, requestHeaders);

        CURLcode responseStatus;
        curl_easy_setopt(requestHandle, CURLOPT_TIMEOUT, 5);

        curl_easy_setopt(requestHandle, CURLOPT_WRITEFUNCTION, responseReceiver);
        curl_easy_setopt(requestHandle, CURLOPT_WRITEDATA,
                         responseBuffer);

        responseStatus = curl_easy_perform(requestHandle);

        curl_easy_cleanup(requestHandle);
        curl_slist_free_all(requestHeaders);

        if (responseStatus != CURLE_OK) {
            puts(std::format("{}Fail to fetch contributor info:{}{}{}", USE_COLOR(FCOLOR_RED), USE_COLOR(FCOLOR_MAGENTA), curl_easy_strerror(responseStatus), USE_COLOR(FCOLOR_WHITE)).c_str());
            return;
        }

        std::vector<std::pair<const char *, int>> contributorsInfo;

        auto root = cJSON_Parse(responseBuffer.c_str());
        auto countOfContributors = cJSON_GetArraySize(root);

        for (int i = 0; i < countOfContributors; i++) {
            auto contributorObj = cJSON_GetArrayItem(root, i);
            contributorsInfo.push_back(std::make_pair(
                cJSON_GetObjectItem(contributorObj, "login")->valuestring,
                cJSON_GetObjectItem(contributorObj, "contributions")->valueint));
        }

        cJSON_free(root);

        std::string contributorListTex = "\\begin{enumerate}\n";

        for (auto &contributor : contributorsInfo) {
            contributorListTex.append("\\item \\href{https://github.com/");
            contributorListTex.append(contributor.first);
            contributorListTex.append("}{");
            contributorListTex.append(contributor.first);
            contributorListTex.append("} 提交数：$");
            auto commitNumberStr = itoa(contributor.second);
            contributorListTex.append(commitNumberStr);
            free(commitNumberStr);
            contributorListTex.append("$\n");
        }

        contributorListTex.append("\\end{enumerate}");

        if (targetSection)
            compileSingleFile(
                targetChapter, targetSection, false,
                false,
                arguments.get("spacing.choice", strlen("spacing.choice")),
                arguments.get("spacing.cloze", strlen("spacing.cloze")),
                !!strcmp(arguments.get("spacing.answerQuestion", strlen("spacing.answerQuestion")), "spacing-disabled"),
                contributorListTex.c_str());
        else if (targetChapter)
            compileChapter(
                targetChapter, false,
                false,
                arguments.get("spacing.choice", strlen("spacing.choice")),
                arguments.get("spacing.cloze", strlen("spacing.cloze")),
                !!strcmp(arguments.get("spacing.answerQuestion", strlen("spacing.answerQuestion")), "spacing-disabled"),
                contributorListTex.c_str());
        else
            compileFullFile(
                arguments.get("spacing.choice", strlen("spacing.choice")),
                arguments.get("spacing.cloze", strlen("spacing.cloze")),
                !!strcmp(arguments.get("spacing.answerQuestion", strlen("spacing.answerQuestion")), "spacing-disabled"),
                contributorListTex.c_str());
    });
}