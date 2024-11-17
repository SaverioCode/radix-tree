#ifndef RADIX_TREE_HPP
#define RADIX_TREE_HPP

extern "C" {
#include <stdint.h>
}

#include <map>
#include <string>

class RadixTree
{
    struct Node;

    struct Node
    {
        Node();
        Node(const std::string& value, const void* data, bool is_end);
        ~Node();

        std::string            value;
        void*                  data;  // Todo: change type in BaseConfig
        bool                   is_end;
        std::map<char, Node*>* map; // Todo: it should be an hash-map, C++98 doesn't have built-in hash-map (introduced in C++11)
    };

    public:
        RadixTree();
        RadixTree(const RadixTree& other);
        ~RadixTree();

        RadixTree& operator=(const RadixTree& other);

        bool  insert(const std::string& value, const void* data, bool replace = false);
        void* findPrefix(const std::string& value);      
        void* find(const std::string& value);
        void  debug(Node* root, const std::string& prefix = "");

        Node* getRoot() const;

    private:
        Node* _root;

        void*       _findPrefix(const Node* node, const std::string& value);
        void*       _find(const Node* node, const std::string& value);
        uint32_t    _compare(const std::string& value1, const std::string& value2);
        bool        _insert(Node* root, Node* node, bool replace);
        void        _split(Node* root, Node* node, uint32_t index);
        void        _move(Node* dest, Node* src);
};

#endif
