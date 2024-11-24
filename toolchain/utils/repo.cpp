#pragma once

#include <cJSON/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <format>
#include <vector>

#include "../macros/color.h"

typedef enum _chapterStatus {
    unverified = 0,
    verified = 1
} chapterStatus;

typedef struct _chapterData {
    std::vector<const char *> contributors;
    std::vector<const char *> reviewers;
    chapterStatus status = unverified;
} chapterData;

std::vector<std::vector<chapterData>> repoMeta;

auto readRepoMeta() -> bool {
    repoMeta.resize(0);

    FILE *metaFileFp = fopen("./workspaceData/repo.json", "r");
    if (metaFileFp == NULL) {
        puts(std::format("{}Cannot open file: {}./workspaceData/repo.json{}. Project meta file is corrupted, please re-pull the repo.{}", USE_COLOR(FCOLOR_RED), USE_COLOR(FCOLOR_YELLOW), USE_COLOR(FCOLOR_RED), USE_COLOR(FCOLOR_WHITE)).c_str());
        return false;
    }

    fseek(metaFileFp, 0, SEEK_END);
    auto metaFileLength = ftell(metaFileFp);
    fseek(metaFileFp, 0, SEEK_SET);

    auto metaFile = (char *)malloc(sizeof(char) * (metaFileLength + 10));

    fread(metaFile, 1, metaFileLength, metaFileFp);
    metaFile[metaFileLength] = '\0';

    fclose(metaFileFp);

    auto root = cJSON_Parse(metaFile);

    auto countOfChapter = cJSON_GetArraySize(root);
    repoMeta.resize(countOfChapter);

    for (auto i = 0; i < countOfChapter; i++) {
        auto chapterObj = cJSON_GetArrayItem(root, i);
        auto countOfSection = cJSON_GetArraySize(chapterObj);
        repoMeta[i].resize(countOfSection);

        for (auto j = 0; j < countOfSection; j++) {
            auto sectionObj = cJSON_GetArrayItem(chapterObj, j);

            auto contributorObj = cJSON_GetObjectItem(sectionObj, "contributors");
            auto reviewerObj = cJSON_GetObjectItem(sectionObj, "reviewers");
            auto status = (chapterStatus)cJSON_GetObjectItem(sectionObj, "status")->valueint;

            repoMeta[i][j].status = status;

            auto countOfContributors = cJSON_GetArraySize(contributorObj);
            auto countOfReviewers = cJSON_GetArraySize(reviewerObj);

            for (auto k = 0; k < countOfContributors; k++) repoMeta[i][j].contributors.push_back(cJSON_GetArrayItem(contributorObj, k)->valuestring);
            for (auto k = 0; k < countOfReviewers; k++) repoMeta[i][j].reviewers.push_back(cJSON_GetArrayItem(reviewerObj, k)->valuestring);
        }
    }

    cJSON_free(root);
    free(metaFile);
    return true;
}

auto saveRepoMeta() -> void {
    FILE *metaFileFp = fopen("./workspaceData/repo.json", "w");
    if (metaFileFp == NULL) {
        puts(std::format("{}Cannot write: {}./workspaceData/repo.json{}", USE_COLOR(FCOLOR_RED), USE_COLOR(FCOLOR_YELLOW), USE_COLOR(FCOLOR_WHITE)).c_str());
        return;
    }

    cJSON *root = cJSON_CreateArray();

    for (auto &chapter : repoMeta) {
        cJSON *chapterObj = cJSON_CreateArray();
        for (auto &section : chapter) {
            cJSON *sectionObj = cJSON_CreateObject();
            cJSON *contributorObj = cJSON_CreateArray();
            cJSON *reviewerObj = cJSON_CreateArray();
            for (auto &contributor : section.contributors) cJSON_AddItemToArray(contributorObj, cJSON_CreateString(contributor));
            for (auto &reviewer : section.reviewers) cJSON_AddItemToArray(reviewerObj, cJSON_CreateString(reviewer));
            cJSON_AddItemToObject(sectionObj, "contributors", contributorObj);
            cJSON_AddItemToObject(sectionObj, "reviewers", reviewerObj);
            cJSON_AddItemToObject(sectionObj, "status", cJSON_CreateNumber(section.status));

            cJSON_AddItemToArray(chapterObj, sectionObj);
        }
        cJSON_AddItemToArray(root, chapterObj);
    }

    auto result = cJSON_Print(root);
    cJSON_free(root);

    auto resultLength = strlen(result);
    fwrite(result, 1, resultLength, metaFileFp);

    fclose(metaFileFp);
}

auto checkSectionVerified(int chapter, int section) -> bool {
    if ((size_t)chapter >= repoMeta.size() || (size_t)section >= repoMeta[chapter - 1].size()) return false;
    return repoMeta[chapter - 1][section - 1].status == verified;
}