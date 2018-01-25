#include <list>
#include <iostream>
#include <vector>
#include <algorithm>

class TrieNode
{
public:
    TrieNode(char letter)
        : isLeaf{false}
        , letter{letter}
    {
        globalCount++;
    }

    ~TrieNode()
    {
        for (auto node : children) {
            delete node;
        }
    }

    void insert(const char* str)
    {
        char c = *str;
        auto nodeIt = std::find_if(children.begin(), children.end(), 
                [c](TrieNode* node) { return node->letter == c; });

        TrieNode* node = nullptr;
        if (nodeIt != children.end()) {
            node = *nodeIt;
        } else {
            node = new TrieNode(c);
            children.push_back(node);
        }

        auto nextStr = str + 1;
        if (*nextStr == '\0') {
            node->isLeaf = true;
        } else {
            node->insert(nextStr);
        }
    }

    void getNodes(std::vector<TrieNode*>& nodes, int& id) const
    {
        this->id = id++;
        TrieNode* prev = nullptr;

        for (auto it = children.begin(); it != children.end(); ++it) {
            auto node = *it;
            nodes.push_back(node);
            node->getNodes(nodes, id);
            if (prev != nullptr) {
                prev->nextId = node->id;
            }

            prev = node;
        }

        if (prev != nullptr) {
            prev->nextId = 0;
        }

        auto bottom = children.front();
        this->bottomId = bottom != nullptr ? bottom->id : 0;
    }

public:
    static int globalCount;
    mutable int id;
    mutable int nextId;
    mutable int bottomId;
    bool isLeaf;
    char letter;
    std::list<TrieNode*> children;
};

int TrieNode::globalCount = 0;

class TrieTree
{
public:
    TrieTree()
        : root{new TrieNode('\0')}
    {
    }

    ~TrieTree()
    {
        delete root;
    }

    void insert(const std::string& s)
    {
        root->insert(s.c_str());
    }

    std::vector<TrieNode*> getNodes() const
    {
        int startId = 0;
        auto nodes = std::vector<TrieNode*>{};
        nodes.reserve(TrieNode::globalCount);
        nodes.push_back(root);
        root->getNodes(nodes, startId);
        return nodes;
    }

private:
    TrieNode* root;
};
