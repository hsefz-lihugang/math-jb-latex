#pragma once
#include <stdlib.h>
#include <string.h>

#include <queue>

template <typename T>
struct _trie_tree_node {
    T value;
    _trie_tree_node *map[256];
};

template <typename T>
class TrieTree {
   public:
    _trie_tree_node<T> root;

    TrieTree() { memset(root.map, 0, sizeof(root.map)); }

    void insert(const char *str, size_t len, T value) {
        _trie_tree_node<T> *cur = &root;
        for (size_t i = 0; i < len; i++) {
            if (!cur->map[(size_t)str[i]]) {
                cur->map[(size_t)str[i]] =
                    (_trie_tree_node<T> *)malloc(sizeof(_trie_tree_node<T>));
                memset(cur->map[(size_t)str[i]]->map, 0,
                       sizeof(cur->map[(size_t)str[i]]->map));
            }
            cur = cur->map[(size_t)str[i]];
        }
        cur->value = value;
    }

    T get(const char *str, size_t len) {
        _trie_tree_node<T> *cur = &root;
        for (size_t i = 0; i < len; i++)
            if (cur->map[(size_t)str[i]])
                cur = cur->map[(size_t)str[i]];
            else
                return NULL;
        return cur->value;
    }

    void clear(_trie_tree_node<T> *node, bool isFreeable) {
        for (size_t i = 0; i < 256; i++)
            if (node->map[i]) clear(node->map[i], true);
        if (isFreeable) free(node);
    }

    void clear() {
        return clear(&root, false);
    }
};