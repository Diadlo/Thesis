#ifndef _TRIE_TREE_H_
#define _TRIE_TREE_H_

#include <list>
#include <iostream>
#include <vector>
#include <algorithm>

class trie_node
{
public:
    trie_node(char letter)
        : is_leaf{false}
        , letter{letter}
    {
        global_count++;
    }

    ~trie_node()
    {
        for (auto node : children) {
            delete node;
        }
    }

    void insert(const char* str)
    {
        char c = *str;
        auto nodeIt = std::find_if(children.begin(), children.end(),
                [c](trie_node* node) { return node->letter == c; });

        trie_node* node = nullptr;
        if (nodeIt != children.end()) {
            node = *nodeIt;
        } else {
            node = new trie_node(c);
            children.push_back(node);
        }

        auto nextStr = str + 1;
        if (*nextStr == '\0') {
            node->is_leaf = true;
        } else {
            node->insert(nextStr);
        }
    }

    void get_nodes(std::vector<trie_node*>& nodes, int& id) const
    {
        this->id = id++;
        trie_node* prev = nullptr;

        for (auto it = children.begin(); it != children.end(); ++it) {
            auto node = *it;
            nodes.push_back(node);
            node->get_nodes(nodes, id);
            if (prev != nullptr) {
                prev->next_id = node->id;
            }

            prev = node;
        }

        if (prev != nullptr) {
            prev->next_id = 0;
        }

        auto bottom = children.front();
        this->bottom_id = bottom != nullptr ? bottom->id : 0;
    }

public:
    static int global_count;
    mutable int id;
    mutable int next_id;
    mutable int bottom_id;
    bool is_leaf;
    char letter;
    std::list<trie_node*> children;
};

int trie_node::global_count = 0;

class trie_tree
{
public:
    trie_tree()
        : root{new trie_node('\0')}
    {
    }

    ~trie_tree()
    {
        delete root;
    }

    void insert(const std::string& s)
    {
        root->insert(s.c_str());
    }

    std::vector<trie_node*> get_nodes() const
    {
        int startId = 0;
        auto nodes = std::vector<trie_node*>{};
        nodes.reserve(trie_node::global_count);
        nodes.push_back(root);
        root->get_nodes(nodes, startId);
        return nodes;
    }

private:
    trie_node* root;
};

#endif // _TRIE_TREE_H_
