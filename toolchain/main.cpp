#include <curl/curl.h>
#include <stdio.h>
#include <string.h>

#include "./utils/repo.cpp"
#include "subcommands/accept.cpp"
#include "subcommands/commit.cpp"
#include "subcommands/export.cpp"
#include "subcommands/help.cpp"
#include "subcommands/preview.cpp"
#include "subcommands/subCommandNotFoundPrompt.cpp"
#include "utils/trie.cpp"

auto main(int argc, const char **argv) -> int {
    if (!readRepoMeta()) return 0;
    curl_global_init(CURL_GLOBAL_DEFAULT);

    TrieTree<void (*)(const char *, TrieTree<const char *> &)> subcommands;
    const char *subcommandString = argc == 1 ? "help" : argv[1];

    TrieTree<const char *> arguments;

    registerHelpCommand(subcommands, arguments);
    registerPreviewCommand(subcommands, arguments);
    registerCommitCommand(subcommands, arguments);
    registerAcceptCommand(subcommands, arguments);
    registerExportCommand(subcommands, arguments);
    registerSubCommandNotFoundError(subcommands, arguments);

    for (int i = 3 /* argv[0] is filename, argv[1] is subcommand, argv[2] is destination */; i < argc; i++) {
        auto length = strlen(argv[i]);

        if (length < 2) continue;
        if (argv[i][0] != '-' && argv[i][1] != '-') continue;

        int valuePos = -1;
        for (size_t j = 2; j < length; j++)
            if (argv[i][j] == '=') {
                valuePos = j + 1;
                break;
            }

        if (valuePos <= 0) continue;

        arguments.insert(argv[i] + 2, valuePos - 3, argv[i] + valuePos);
    }

    auto subcommandFunction = subcommands.get(subcommandString, strlen(subcommandString));
    if (subcommandFunction)
        subcommandFunction(argc < 3 ? "" : argv[2], arguments);
    else
        subcommands.get("::subCommandNotFound", strlen("::subCommandNotFound"))(argv[1], arguments);

    arguments.clear();
    subcommands.clear();
    curl_global_cleanup();
    return 0;
}