#pragma once

#include <stdlib.h>

#include <stack>

char *itoa(int num) {
    auto result = (char *)malloc(sizeof(char) * 20);
    size_t length = 0;

    std::stack<char> characters;

    while (num) {
        characters.push(num % 10 + '0');
        num /= 10;
    }

    while (characters.size()) {
        result[length++] = characters.top();
        characters.pop();
    }

    result[length] = '\0';

    return result;
}