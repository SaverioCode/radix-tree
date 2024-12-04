#ifndef RADIX_TREE_HPP
#define RADIX_TREE_HPP

#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>

template<typename T>
class RadixTree
{
    struct Node;

    struct Node
    {
        Node();
        Node(const std::string& value, const T* data, bool is_end);
        ~Node();

        std::string                      value;
        T*                               data;
        bool                             is_end;
        std::unordered_map<char, Node*>* map;
    };

    public:
        RadixTree();
        RadixTree(const RadixTree& other) = delete;
        RadixTree(const RadixTree&& other);
        ~RadixTree();

        RadixTree& operator=(const RadixTree& other) = delete;

        bool insert(const std::string& value, const T* data, bool replace = false);
        T*   findPrefix(const std::string& value);      
        T*   find(const std::string& value);

    #ifdef DEBUG    
        void debug(Node* root, const std::string& prefix = "");
        Node* getRoot() const;
    #endif

    private:
        Node* _root;

        T*       _findPrefix(const Node* node, const std::string& value);
        T*       _find(const Node* node, const std::string& value);
        uint32_t _compare(const std::string& value1, const std::string& value2);
        bool     _insert(Node* root, Node* node, bool replace);
        void     _split(Node* root, Node* node, uint32_t index);
        void     _move(Node* dest, Node* src);
};

/************** NODE *************/
template<typename T>
RadixTree<T>::Node::Node()
{
    data  = nullptr;
    is_end = false;
    map = new std::unordered_map<char, Node*>();
}

template<typename T>
RadixTree<T>::Node::Node(const std::string& value, const T* data, bool is_end)
{
    this->value = value;
    this->data = const_cast<T*>(data);
    this->is_end = is_end;
    this->map = new std::unordered_map<char, Node*>();
}

template<typename T>
RadixTree<T>::Node::~Node()
{
    delete data;
    for (auto it = map->begin(); it != map->end(); it++) {
        delete it->second;
    }
    delete map;
}

/************** RADIX-TREE *************/

template<typename T>
RadixTree<T>::RadixTree()
{
    _root = new Node();
}

template<typename T>
RadixTree<T>::~RadixTree()
{
    delete _root;
}
        
template<typename T>
RadixTree<T>::RadixTree(const RadixTree&& other)
{
    _root = other._root;
    cast_const<RadixTree&&>(other)._root = new Node();
}

template<typename T>
bool RadixTree<T>::insert(const std::string& value, const T* data, bool replace)
{
    return _insert(_root, new Node(value, data, true), replace);
}

template<typename T>
T* RadixTree<T>::find(const std::string& value)
{
    auto it = _root->map->find(value[0]);
    if (it == _root->map->end()) {
        return nullptr;
    }
    return _find(it->second, value);
}

template<typename T>
T* RadixTree<T>::_find(const Node* node, const std::string& value)
{
    uint32_t index = _compare(node->value, value);
    if (index == 0) {
        return node->data;
    }
    auto it = node->map->find(value[index]);
    if (it == node->map->end()) {
        return nullptr;
    }
    return _find(it->second, &value[index]);
}

template<typename T>
T* RadixTree<T>::findPrefix(const std::string& value)
{
    auto it = _root->map->find(value[0]);
    if (it == _root->map->end()) {
        std::cout << "main root fail findPrefix" << std::endl;
        return _root->is_end ? _root->data : nullptr;
    }
    return _findPrefix(it->second, value);
}

template<typename T>
T* RadixTree<T>::_findPrefix(const Node* node, const std::string& value)
{
    uint32_t index = _compare(node->value, value);
    if (index == 0) {
        return node->is_end ? node->data : nullptr;
    }
    auto it = node->map->find(value[index]);
    if (it == node->map->end()) {
        return node->is_end ? node->data : nullptr;
    }
    T* data = _findPrefix(it->second, &value[index]);
    if (data == nullptr) {
        return node->is_end ? node->data : nullptr;
    }
    return data;
}

#ifdef DEBUG
template<typename T>
void RadixTree<T>::debug(Node* root, const std::string& prefix)
{
    if (root == nullptr) return;

    if (!root->value.empty()) {
        std::cout << prefix << root->value << (root->is_end ? " [END]" : "") << "\n";
    }

    for (auto it = root->map->begin(); it != root->map->end(); it++) {
        debug(it->second, prefix + "     ");
    }
}

template<typename T>
RadixTree<T>::Node* RadixTree<T>::getRoot() const
{
    return _root;
}
#endif

template<typename T>
uint32_t RadixTree<T>::_compare(const std::string& value1, const std::string& value2)
{
    uint32_t i = 0;
    while (i < value1.size() && i < value2.size()) {
        if (value1[i] != value2[i]) {
            return i;
        }
        i++;
    }
    return value1.size() == value2.size() ? 0 : i;
}

template<typename T>
bool RadixTree<T>::_insert(Node* root, Node* node, bool replace)
{
    auto it = root->map->find(node->value[0]);
    if (it == root->map->end()) {
        return root->map->insert(std::pair<char, Node*>(node->value[0], node)).second;
    }
    else {
        root = it->second;
        uint32_t index = _compare(root->value, node->value);
        if (index == 0) {
            bool ret = !replace && root->is_end ? false : true;
            if (ret) {
                root->data = node->data;
                root->is_end = node->is_end;
                node->data = nullptr;
            }
            delete node;
            return ret;
        }
        if (index < root->value.size()) {
            _split(root, node, index);
            return true;
        }
        node->value = &node->value[index];
        return _insert(root, node, replace);
    }
}

template<typename T>
void RadixTree<T>::_split(Node* root, Node* node, uint32_t index)
{
    Node* new_node = new Node();
    new_node->value = &root->value[index];
    new_node->data = root->data;
    new_node->is_end = root->is_end;

    if (index < node->value.size()) {
        root->value = root->value.substr(0, index);
        root->data = nullptr;
        root->is_end = false;
        std::unordered_map<char, Node*>* map = new_node->map;
        new_node->map = root->map;
        root->map = map;
        node->value = &node->value[index];
        _insert(root, new_node, true);
        _insert(root, node, true);
    }
    else {
        _move(root, node);
        delete node;
        _insert(root, new_node, true);
    }
}

template<typename T>
void RadixTree<T>::_move(Node* dest, Node* src)
{
    dest->value = src->value;
    src->value.clear();
    delete dest->data;
    dest->data = src->data;
    src->data = nullptr;
    dest->is_end = src->is_end;
}

#endif
