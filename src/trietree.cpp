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

    std::vector<TrieNode*> getNodes(int& id) const
    {
        this->id = id++;
        auto nodes = std::vector<TrieNode*>{};
        TrieNode* prev = nullptr;

        for (auto it = children.begin(); it != children.end(); ++it) {
            auto node = *it;
            auto childNodes = node->getNodes(id);
            if (prev != nullptr) {
                prev->nextId = node->id;
            }

            nodes.push_back(node);
            nodes.insert(nodes.end(), childNodes.begin(), childNodes.end());
            prev = node;
        }

        if (prev != nullptr) {
            prev->nextId = 0;
        }

        auto bottom = children.front();
        this->bottomId = bottom != nullptr ? bottom->id : 0;

        return nodes;
    }

public:
    mutable int id;
    mutable int nextId;
    mutable int bottomId;
    bool isLeaf;
    char letter;
    std::list<TrieNode*> children;
};

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
        return root->getNodes(startId);
    }

private:
    TrieNode* root;
};

int main()
{
    TrieTree t;
    t.insert("abc");
    t.insert("abq");
    t.insert("asd");
    for (auto node : t.getNodes()) {
        std::cout 
            << node->letter << " " 
            << "id="   << node->id << " " 
            << "next=" << node->nextId << " " 
            << "bott=" << node->bottomId << std::endl;
    }

    return 0;
}
