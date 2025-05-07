#ifndef RADIX_TREE_HPP
#define RADIX_TREE_HPP

#include <cstdint>
#include <string>
#include <unordered_map>

template<typename T>
class RadixTree
{
    struct Node;

    struct Node
    {
        Node();
        Node(const Node& other) = delete;
        Node(const Node&& other);
        Node(const std::string& value, const T* data, bool is_end);
        ~Node();

        Node& operator=(const Node& other) = delete; // Todo: force move?
        // Todo: implemente operator==

        T*                               data;
        bool                             is_end;
        std::unordered_map<char, Node*>* map;
        std::string                      value;
    };

    public:
        RadixTree();
        RadixTree(const RadixTree& other) = delete;
        RadixTree(const RadixTree&& other);
        ~RadixTree();

        RadixTree& operator=(const RadixTree& other) = delete;

        T*   find(const std::string& value) const noexcept;
        T*   findPrefix(const std::string& value) const noexcept;
        bool insert(const std::string& value, const T* data, bool replace = false) noexcept;

    #ifdef DEBUG    
        void printTree(Node* root, const std::string& prefix = "") const noexcept;
        Node* getRoot() const const noexcept;
    #endif

    private:
        Node* _root;

        uint32_t _compare(const std::string& value1, const std::string& value2) const noexcept;
        T*       _find(const Node* node, const std::string& value) const noexcept;
        T*       _findPrefix(const Node* node, const std::string& value) const noexcept;
        bool     _insert(Node* root, Node* node, bool replace) noexcept;
        void     _split(Node* root, Node* node, uint32_t index) noexcept;
};

/************** NODE *************/
template<typename T>
RadixTree<T>::Node::Node()
{
    data   = nullptr;
    is_end = false;
    map    = new std::unordered_map<char, Node*>();
}

template<typename T>
RadixTree<T>::Node::Node(const std::string& value, const T* data, bool is_end)
{
    this->value  = value;
    this->data   = const_cast<T*>(data);
    this->is_end = is_end;
    this->map    = new std::unordered_map<char, Node*>();
}

template<typename T>
RadixTree<T>::Node::Node(const Node&& other)
{
    this->data   = other.data;
    this->is_end = other.is_end;
    this->map    = other.map;
    this->value  = other.value;
    const_cast<Node&&>(other).data = nullptr;
    const_cast<Node&&>(other).map  = nullptr;
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
    const_cast<RadixTree&&>(other)._root = new Node();
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
        return node->data;
    }
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
void RadixTree<T>::printTree(Node* root, const std::string& prefix) const noexcept
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
typename RadixTree<T>::Node* RadixTree<T>::getRoot() const noexcept
{
    return _root;
}
#endif

template<typename T>
uint32_t RadixTree<T>::_compare(const std::string& value1, const std::string& value2) const noexcept
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
void RadixTree<T>::_split(Node* root, Node* node, uint32_t index) noexcept
{
    Node* new_node   = new Node();
    new_node->value  = &root->value[index];
    new_node->data   = root->data;
    new_node->is_end = root->is_end;

    if (index < node->value.size()) {
        root->data = nullptr;
        root->is_end = false;
        std::unordered_map<char, Node*>* map = new_node->map;
        new_node->map = root->map;
        root->map = map;
        root->value.resize(index);
        node->value = &node->value[index];
        _insert(root, node, true);
    }
    else {
        root->data = node->data;
        node->data = nullptr;
        root->value = node->value;
        root->is_end = node->is_end;
        delete node;
    }
    _insert(root, new_node, true);
}

#endif
