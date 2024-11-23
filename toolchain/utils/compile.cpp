#pragma once

#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <chrono>
#include <format>
#include <string>
#include <utility>
#include <vector>

#include "../macros/color.h"
#include "../utils/itoa.cpp"

auto loadTemplate(const char *templateName, std::vector<const char *> &arguments) -> char * {
    FILE *templateFilePtr = fopen(std::format("./template/{}.tex", templateName).c_str(), "r");
    if (templateFilePtr) {
        fseek(templateFilePtr, 0, SEEK_END);
        auto fileSize = ftell(templateFilePtr);
        fseek(templateFilePtr, 0, SEEK_SET);

        auto argumentSize = 0;
        for (auto &arg : arguments) argumentSize += strlen(arg);

        auto templateRawContent = (char *)malloc(sizeof(char) * (fileSize + 1));
        auto result = (char *)malloc(sizeof(char) * (fileSize + argumentSize + 1));

        fread(templateRawContent, 1, fileSize, templateFilePtr);
        templateRawContent[fileSize] = '\0';
        fclose(templateFilePtr);

        int rawContentPos = 0, resultPos = 0;
        while (templateRawContent[rawContentPos] != '\0') {
            if (templateRawContent[rawContentPos] == '!' && isalnum(templateRawContent[rawContentPos + 1])) {
                auto argumentId = templateRawContent[rawContentPos + 1] - '0';
                auto argumentLength = strlen(arguments[argumentId]);
                for (size_t i = 0; i < argumentLength; i++) result[resultPos++] = arguments[argumentId][i];
                rawContentPos += 2;
                continue;
            }
            result[resultPos++] = templateRawContent[rawContentPos++];
        }

        result[resultPos] = '\0';

        free(templateRawContent);
        return result;
    } else
        return NULL;
}

auto compileGraph(int chapter, int section, bool useCache = true) {
    std::vector<std::string> graphs;
    auto dirPointer = opendir(std::format("./src/{}/{}/graphs", chapter, section).c_str());
    if (dirPointer) {
        struct dirent *fileMetaPtr;

        while ((fileMetaPtr = readdir(dirPointer)) != NULL) {
            auto filename = std::format("./src/{}/{}/graphs/{}", chapter, section, fileMetaPtr->d_name);
            if (filename.find(".sha256") != filename.npos || filename.find(".tex") == filename.npos || fileMetaPtr->d_name[0] == '.') continue;
            graphs.push_back(filename);
        }
        closedir(dirPointer);
    }

    for (auto &graph : graphs) {
        char oldSha256[65] = {'\0'}, newSha256[65] = {'\0'};
        if (useCache) {
            FILE *shaFilePointer = fopen(std::format("{}{}", graph, ".sha256").c_str(), "r");
            if (shaFilePointer) {
                fscanf(shaFilePointer, "%s", oldSha256);
                fclose(shaFilePointer);

                system(std::format("sha256sum {}>{}.sha256", graph, graph).c_str());

                shaFilePointer = fopen(std::format("{}{}", graph, ".sha256").c_str(), "r");
                if (shaFilePointer) {
                    fscanf(shaFilePointer, "%s", newSha256);
                    fclose(shaFilePointer);
                }

                if (!strcmp(oldSha256, newSha256)) continue;
            }
        }

        puts(std::format("{}Compiling {}{}{}", USE_COLOR(FCOLOR_CYAN), USE_COLOR(FCOLOR_YELLOW), graph, USE_COLOR(FCOLOR_WHITE)).c_str());

        system(std::format("xelatex --output-directory=./src/{}/{}/graphs {}", chapter, section, graph).c_str());

        FILE *outputFilePtr = fopen(std::format("{}.pdf", std::string_view(graph.c_str(), graph.size() - 4)).c_str(), "r");
        if (outputFilePtr) {
            puts(std::format("{}Compile Succeeded: {}{}{}", USE_COLOR(FCOLOR_GREEN), USE_COLOR(FCOLOR_YELLOW), graph, USE_COLOR(FCOLOR_WHITE)).c_str());
            system(std::format("sha256sum {}>{}.sha256", graph, graph).c_str());
            fclose(outputFilePtr);
        } else {
            puts(std::format("{}Compile Failed: {}{}{}", USE_COLOR(FCOLOR_RED), USE_COLOR(FCOLOR_YELLOW), graph, USE_COLOR(FCOLOR_WHITE)).c_str());
            remove(std::format("{}.sha256", graph).c_str());
            return false;
        }
    }

    return true;
}

auto compileSingleFile(int chapter, int section, bool useCache = true, bool isPreview = false, const char *choiceSpacing = "0pt", const char *clozeSpacing = "0pt", bool questionSpacing = true) -> bool {
    auto startTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    const char *targetType = isPreview ? "preview" : "export";

    auto compileGraphState = compileGraph(chapter, section, useCache);
    if (!compileGraphState) return false;

    std::vector<const char *> templateArguments;

    auto chapterString = itoa(chapter), sectionString = itoa(section);
    templateArguments.push_back(chapterString);
    templateArguments.push_back(sectionString);
    templateArguments.push_back(choiceSpacing);
    templateArguments.push_back(clozeSpacing);
    templateArguments.push_back(questionSpacing ? "Enabled" : "Disabled");
    templateArguments.push_back(questionSpacing ? "" : "% ");  // LaTeX comments
    templateArguments.push_back(useCache ? "Enabled" : "Disabled");

    FILE *mainTexFilePtr = fopen(std::format("./src/{}/{}/main.tex", chapter, section).c_str(), "r");
    if (!mainTexFilePtr) {
        puts(std::format("{}Cannot open source file: {}./src/{}/{}/main.tex{}", USE_COLOR(FCOLOR_RED), USE_COLOR(FCOLOR_YELLOW), chapter, section, USE_COLOR(FCOLOR_WHITE)).c_str());
        return false;
    }

    fseek(mainTexFilePtr, 0, SEEK_END);
    auto mainTexFileSize = ftell(mainTexFilePtr);
    fseek(mainTexFilePtr, 0, SEEK_SET);

    auto mainTexFileContent = (char *)malloc(sizeof(char) * (mainTexFileSize + 1));
    fread(mainTexFileContent, 1, mainTexFileSize, mainTexFilePtr);
    mainTexFileContent[mainTexFileSize] = '\0';
    fclose(mainTexFilePtr);

    templateArguments.push_back(mainTexFileContent);

    auto content = loadTemplate(targetType, templateArguments);

    free(chapterString);
    free(sectionString);
    free(mainTexFileContent);

    if (content == NULL) {
        puts(std::format("{}Cannot open template: {}./template/{}.tex{}", USE_COLOR(FCOLOR_RED), USE_COLOR(FCOLOR_YELLOW), targetType, USE_COLOR(FCOLOR_WHITE)).c_str());
        return false;
    }
    auto contentLength = strlen(content);

    mainTexFilePtr = fopen(std::format("./src/{}/{}/generated.tex", chapter, section).c_str(), "w");

    if (mainTexFilePtr == NULL) {
        puts(std::format("{}Cannot write to: {}./src/{}/{}/generated.tex{}", USE_COLOR(FCOLOR_RED), USE_COLOR(FCOLOR_YELLOW), chapter, section, USE_COLOR(FCOLOR_WHITE)).c_str());
        free(content);
        return false;
    }

    fwrite(content, 1, contentLength, mainTexFilePtr);
    fclose(mainTexFilePtr);
    free(content);

    if (useCache) {
        char oldSha256[65] = {'\0'}, newSha256[65] = {'\0'};
        FILE *shaFilePointer = fopen(std::format("./src/{}/{}/generated.tex.sha256", chapter, section).c_str(), "r");
        if (shaFilePointer) {
            fscanf(shaFilePointer, "%s", oldSha256);
            fclose(shaFilePointer);

            system(std::format("sha256sum ./src/{}/{}/generated.tex>./src/{}/{}/generated.tex.sha256", chapter, section, chapter, section).c_str());

            shaFilePointer = fopen(std::format("./src/{}/{}/generated.tex.sha256", chapter, section).c_str(), "r");
            if (shaFilePointer) {
                fscanf(shaFilePointer, "%s", newSha256);
                fclose(shaFilePointer);
            }

            if (!strcmp(oldSha256, newSha256)) {
                puts(std::format("{}Cache was hit. Please go {}./result/{}.{}.{}.pdf{} to see your file.{}", USE_COLOR(FCOLOR_GREEN), USE_COLOR(FCOLOR_YELLOW), chapter, section, targetType, USE_COLOR(FCOLOR_GREEN), USE_COLOR(FCOLOR_WHITE)).c_str());
                return true;
            }
        }
    }

    puts(std::format("{}Compiling {}./src/{}/{}/generated.tex{}", USE_COLOR(FCOLOR_CYAN), USE_COLOR(FCOLOR_YELLOW), chapter, section, USE_COLOR(FCOLOR_WHITE)).c_str());

    system(std::format("xelatex --output-directory=./src/{}/{} ./src/{}/{}/generated.tex", chapter, section, chapter, section).c_str());

    FILE *outputFilePtr = fopen(std::format("./src/{}/{}/generated.pdf", chapter, section).c_str(), "r");
    if (outputFilePtr) {
        puts(std::format("{}Compile Succeeded: {}./src/{}/{}/generated.tex{}", USE_COLOR(FCOLOR_GREEN), USE_COLOR(FCOLOR_YELLOW), chapter, section, USE_COLOR(FCOLOR_WHITE)).c_str());
        fclose(outputFilePtr);

        system(std::format("sha256sum ./src/{}/{}/generated.tex>./src/{}/{}/generated.tex.sha256", chapter, section, chapter, section).c_str());
        rename(
            std::format("./src/{}/{}/generated.pdf", chapter, section).c_str(),
            std::format("./result/{}.{}.{}.pdf", chapter, section, targetType).c_str());

        auto endTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();

        puts(std::format("{}Compile Task was finished. Taken time: {}{:.2f} seconds{}. Please go {}./result/{}.{}.{}.pdf{} to see your file.{}", USE_COLOR(FCOLOR_GREEN), USE_COLOR(FCOLOR_CYAN), (float)(endTime - startTime) / 1000, USE_COLOR(FCOLOR_GREEN), USE_COLOR(FCOLOR_YELLOW), chapter, section, targetType, USE_COLOR(FCOLOR_GREEN), USE_COLOR(FCOLOR_WHITE)).c_str());
    } else {
        puts(std::format("{}Compile Failed: {}./src/{}/{}/generated.tex{}", USE_COLOR(FCOLOR_RED), USE_COLOR(FCOLOR_YELLOW), chapter, section, USE_COLOR(FCOLOR_WHITE)).c_str());
        remove(std::format("./src/{}/{}/generated.tex.sha256", chapter, section).c_str());
        remove(std::format("./result/{}.{}.{}.pdf", chapter, section, targetType).c_str());
        return false;
    }

    return true;
}

auto compileChapter(int chapter, bool useCache = true, bool isPreview = false, const char *choiceSpacing = "0pt", const char *clozeSpacing = "0pt", bool questionSpacing = true) -> bool {
    auto startTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    const char *targetType = isPreview ? "chapterPreview" : "chapterExport";

    int countOfSections = 0;
    auto chapterDirectoryPtr = opendir(std::format("./src/{}", chapter).c_str());

    if (!chapterDirectoryPtr) {
        puts(std::format("{}Cannot open directory: {}./src/{}/{}", USE_COLOR(FCOLOR_RED), USE_COLOR(FCOLOR_YELLOW), chapter, USE_COLOR(FCOLOR_WHITE)).c_str());
        return false;
    }

    struct dirent *chapterDirectoryMetaPtr;

    while ((chapterDirectoryMetaPtr = readdir(chapterDirectoryPtr)) != NULL) {
        if (isalnum(chapterDirectoryMetaPtr->d_name[0]))  // we assume a file started with number is a directory, for d_type is not in POSIX standard and MinGw on Windows does not contain it.
            countOfSections = std::max(countOfSections, atoi(chapterDirectoryMetaPtr->d_name));
    }

    closedir(chapterDirectoryPtr);

    puts(std::format("{}{} {}section{} {} scanned.{}", USE_COLOR(FCOLOR_GREEN), countOfSections, USE_COLOR(FCOLOR_CYAN), countOfSections < 2 ? "" : "s", countOfSections < 2 ? "was" : "were", USE_COLOR(FCOLOR_WHITE)).c_str());

    for (int i = 1; i <= countOfSections; i++)
        if (!compileGraph(chapter, i, useCache)) return false;

    auto includeStatements = (char *)malloc(sizeof(char) * (strlen("\\newpage\n\\section{Section 1000}\n\\newcommand{\\useImage}[1]{\\includegraphics{./src/!0/1000/graphs/#1.pdf}}\n\\input{./src/1000/1000/main.tex}\n") * countOfSections));

    size_t includeStatementsSize = 0;

    for (int i = 1; i <= countOfSections; i++)
        includeStatementsSize += sprintf(includeStatements + includeStatementsSize, "\\newpage\n\\section{Section %d}\n\\newcommand{\\useImage}[1]{\\includegraphics{./src/%d/%d/graphs/#1.pdf}}\n\\input{./src/%d/%d/main.tex}\n", i, chapter, i, chapter, i);

    std::vector<const char *> templateArguments;

    auto chapterString = itoa(chapter);

    templateArguments.push_back(chapterString);
    templateArguments.push_back(choiceSpacing);
    templateArguments.push_back(clozeSpacing);
    templateArguments.push_back(questionSpacing ? "Enabled" : "Disabled");
    templateArguments.push_back(questionSpacing ? "" : "% ");  // LaTeX comments
    templateArguments.push_back(useCache ? "Enabled" : "Disabled");
    templateArguments.push_back(includeStatements);

    auto content = loadTemplate(targetType, templateArguments);
    free(chapterString);
    free(includeStatements);

    if (content == NULL) {
        puts(std::format("{}Cannot open template: {}./template/{}.tex{}", USE_COLOR(FCOLOR_RED), USE_COLOR(FCOLOR_YELLOW), targetType, USE_COLOR(FCOLOR_WHITE)).c_str());
        return false;
    }
    auto contentLength = strlen(content);

    FILE *generatedFilePtr = fopen(std::format("./src/{}/generated.tex", chapter).c_str(), "w");

    if (generatedFilePtr == NULL) {
        puts(std::format("{}Cannot write to: {}./src/{}/generated.tex{}", USE_COLOR(FCOLOR_RED), USE_COLOR(FCOLOR_YELLOW), chapter, USE_COLOR(FCOLOR_WHITE)).c_str());
        free(content);
        return false;
    }

    fwrite(content, 1, contentLength, generatedFilePtr);
    fclose(generatedFilePtr);
    free(content);

    puts(std::format("{}Compiling {}./src/{}/generated.tex{}", USE_COLOR(FCOLOR_CYAN), USE_COLOR(FCOLOR_YELLOW), chapter, USE_COLOR(FCOLOR_WHITE)).c_str());

    system(std::format("xelatex --output-directory=./src/{} ./src/{}/generated.tex && xelatex --output-directory=./src/{} ./src/{}/generated.tex", chapter, chapter, chapter, chapter).c_str());

    FILE *outputFilePtr = fopen(std::format("./src/{}/generated.pdf", chapter).c_str(), "r");
    if (outputFilePtr) {
        puts(std::format("{}Compile Succeeded: {}./src/{}/generated.tex{}", USE_COLOR(FCOLOR_GREEN), USE_COLOR(FCOLOR_YELLOW), chapter, USE_COLOR(FCOLOR_WHITE)).c_str());
        fclose(outputFilePtr);

        rename(
            std::format("./src/{}/generated.pdf", chapter).c_str(),
            std::format("./result/{}.{}.pdf", chapter, targetType).c_str());

        auto endTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();

        puts(std::format("{}Compile Task was finished. Taken time: {}{:.2f} seconds{}. Please go {}./result/{}.{}.pdf{} to see your file.{}", USE_COLOR(FCOLOR_GREEN), USE_COLOR(FCOLOR_CYAN), (float)(endTime - startTime) / 1000, USE_COLOR(FCOLOR_GREEN), USE_COLOR(FCOLOR_YELLOW), chapter, targetType, USE_COLOR(FCOLOR_GREEN), USE_COLOR(FCOLOR_WHITE)).c_str());
    } else {
        puts(std::format("{}Compile Failed: {}./src/{}/generated.tex{}", USE_COLOR(FCOLOR_RED), USE_COLOR(FCOLOR_YELLOW), chapter, USE_COLOR(FCOLOR_WHITE)).c_str());
        remove(std::format("./result/{}.{}.pdf", chapter, targetType).c_str());
        return false;
    }
    return true;
}