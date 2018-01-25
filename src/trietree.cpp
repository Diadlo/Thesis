#include <list>
#include <iostream>
#include <vector>
#include <algorithm>

class TrieNode
{
public:
    TrieNode(char letter)
        : id{idCount++}
        , isLeaf{false}
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

    std::vector<TrieNode*> getNodes() const
    {
        auto nodes = std::vector<TrieNode*>(children.begin(), children.end());
        for (auto node : children) {
            auto childNodes = node->getNodes();
            nodes.insert(nodes.end(), childNodes.begin(), childNodes.end());
        }

        return nodes;
    }

public:
    static int idCount;
    int id;
    bool isLeaf;
    char letter;
    std::list<TrieNode*> children;
};

int TrieNode::idCount = 0;

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
        return root->getNodes();
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
        std::cout << node->letter << " " << node->isLeaf << std::endl;
    }

    return 0;
}
