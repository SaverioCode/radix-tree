#ifndef RADIX_TREE_HPP
#define RADIX_TREE_HPP

#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>

template<typename T>
class RadixTree
{
    struct Node;

    struct Node
    {
        Node() noexcept;
        Node(const Node& other) = delete;
        Node(Node&& other) noexcept;
        Node(const std::string& value, const T* data, bool is_end) noexcept;
        ~Node() noexcept;

        Node& operator=(const Node& other) = delete;
        Node& operator=(Node&& other) noexcept;
        // Todo: implemente operator==

        T*                               data;
        bool                             is_end;
        std::unordered_map<char, Node*>* map;
        std::string                      value;
    };

    public:
        RadixTree() noexcept;
        RadixTree(const RadixTree& other) = delete;
        RadixTree(RadixTree&& other) noexcept;
        ~RadixTree() noexcept;

        RadixTree& operator=(const RadixTree& other) = delete;
        RadixTree& operator=(RadixTree&& other) noexcept;

        // Todo: remove operator==?

        T*   find(const std::string& value) const noexcept;
        T*   findPrefix(const std::string& value) const noexcept;
        bool insert(const std::string& value, const T* data, bool replace = false) noexcept;

    #ifdef DEBUG    
        void printTree(Node* root, const std::string& prefix = "") const noexcept;
        Node* getRoot() const noexcept;
    #endif

    private:
        Node* _root;

        uint32_t _compare(const std::string& value1, const std::string& value2) const noexcept;
        T*       _find(const Node* node, const std::string& value) const noexcept;
        T*       _findPrefix(const Node* node, const std::string& value) const noexcept;
        bool     _insert(Node* root, Node* node, bool replace) noexcept;
        bool     _split(Node* root, Node* node, uint32_t index) noexcept;
};

/************** NODE *************/
template<typename T>
RadixTree<T>::Node::Node() noexcept
{
    data   = nullptr;
    is_end = false;
    map    = new std::unordered_map<char, Node*>();
}

template<typename T>
RadixTree<T>::Node::Node(const std::string& value, const T* data, bool is_end) noexcept
{
    this->value  = value;
    this->data   = const_cast<T*>(data);
    this->is_end = is_end;
    this->map    = new std::unordered_map<char, Node*>();
}

template<typename T>
RadixTree<T>::Node::Node(Node&& other) noexcept
{
    *this = std::move(other);
}

template<typename T>
typename RadixTree<T>::Node& RadixTree<T>::Node::operator=(Node&& other) noexcept
{
    this->data   = std::move(other.data);
    this->is_end = std::move(other.is_end);
    this->map    = std::move(other.map);
    this->value  = std::move(other.value);
    other.data = nullptr;
    other.map  = nullptr;
}

template<typename T>
RadixTree<T>::Node::~Node() noexcept
{
    delete data;
    for (auto it = map->begin(); it != map->end(); it++) {
        delete it->second;
    }
    delete map;
}

/************** RADIX-TREE *************/

template<typename T>
RadixTree<T>::RadixTree() noexcept
{
    _root = new Node();
}

template<typename T>
RadixTree<T>::~RadixTree() noexcept
{
    delete _root;
}
        
template<typename T>
RadixTree<T>::RadixTree(RadixTree&& other) noexcept
{
    *this = std::move(other);
}

template<typename T>
RadixTree<T>& RadixTree<T>::operator=(RadixTree&& other) noexcept
{
    _root = std::move(other._root);
    other._root = new Node();
    return *this;
}

template<typename T>
bool RadixTree<T>::insert(const std::string& value, const T* data, bool replace) noexcept
{
    return _insert(_root, new Node(value, data, true), replace);
}

template<typename T>
T* RadixTree<T>::find(const std::string& value) const noexcept
{
    auto it = _root->map->find(value[0]);
    if (it == _root->map->end()) {
        return nullptr;
    }
    return _find(it->second, value);
}

template<typename T>
T* RadixTree<T>::_find(const Node* node, const std::string& value) const noexcept
{
    uint32_t index = _compare(node->value, value);
    if (index == 0) {
        return node->is_end ? node->data : nullptr;
    }
    if (index >= value.size() || index < node->value.size()) return nullptr;
    auto it = node->map->find(value[index]);
    if (it == node->map->end()) {
        return nullptr;
    }
    return _find(it->second, &value[index]);
}

template<typename T>
T* RadixTree<T>::findPrefix(const std::string& value) const noexcept
{
    auto it = _root->map->find(value[0]);
    if (it == _root->map->end()) {
        return _root->is_end ? _root->data : nullptr;
    }
    return _findPrefix(it->second, value);
}

template<typename T>
T* RadixTree<T>::_findPrefix(const Node* node, const std::string& value) const noexcept
{
    uint32_t index = _compare(node->value, value);
    if (index == 0 || index >= value.size()) {
        return node->is_end ? node->data : nullptr;
    }
    if (index < node->value.size()) return nullptr;
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
void RadixTree<T>::printTree(Node* root, const std::string& prefix) const noexcept
{
    if (root == nullptr) return;

    if (!root->value.empty()) {
        std::cout << prefix << root->value << (root->is_end ? " [END]" : "") << "\n";
    }

    for (auto it = root->map->begin(); it != root->map->end(); it++) {
        printTree(it->second, prefix + "     ");
    }
}

template<typename T>
typename RadixTree<T>::Node* RadixTree<T>::getRoot() const noexcept
{
    return _root;
}
#endif

template<typename T>
uint32_t RadixTree<T>::_compare(const std::string& value1, const std::string& value2) const noexcept
{
    uint32_t delim = value1.size() <= value2.size() ? value1.size() : value2.size();

    uint32_t i = 0;
    for (; i < delim; i++) {
        if (value1[i] != value2[i]) {
            break;
        }
    }
    return ((value1.size() == value2.size()) && (delim == i)) ? 0 : i;
}

template<typename T>
bool RadixTree<T>::_insert(Node* root, Node* node, bool replace) noexcept
{
    auto it = root->map->find(node->value[0]);
    if (it == root->map->end()) {
        return root->map->insert({node->value[0], node}).second;
    }
    else {
        root = it->second;
        uint32_t index = _compare(root->value, node->value);
        if (index == 0) {
            if (root->is_end && !replace) {
                delete node;
                return false;
            }
            delete root->data;
            root->data = node->data;
            node->data = nullptr;
            root->is_end = true;
            delete node;
            return true;
        }
        if (index < root->value.size()) {
            return _split(root, node, index);
        }
        node->value = &node->value[index];
        return _insert(root, node, replace);
    }
}

template<typename T>
bool RadixTree<T>::_split(Node* root, Node* node, uint32_t index) noexcept
{
    Node* new_node = new Node(&root->value[index], root->data, root->is_end);
    std::unordered_map<char, Node*>* tmp_map = root->map;

    root->data = nullptr;
    root->map = new_node->map;
    new_node->map = tmp_map;

    // Updated the value of the node to be inserted
    if (index < node->value.size()) {
        node->value = &node->value[index];
    }

    // Updated the value of the root node
    root->value = root->value.substr(0, index);
    root->is_end = false;

    if (_insert(root, new_node, true) && _insert(root, node, true)) {
        return true;
    }
    return false;
}

#endif
