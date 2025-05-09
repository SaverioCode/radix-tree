#ifndef RADIX_TREE_HPP
#define RADIX_TREE_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

template <typename T>
class RadixTree
{
    struct Node
    {
        typedef std::unordered_map<char, std::unique_ptr<Node>> HashMap;

        Node();
        Node(const Node& other) = delete;
        Node(Node&& other);
        Node(const std::string& value, const std::shared_ptr<T>& data, bool is_end);
        ~Node() noexcept = default;

        Node& operator=(const Node& other) = delete;
        Node& operator=(Node&& other);

        bool addNode(Node& node);

        std::shared_ptr<T>  data;
        bool                is_end;
        std::string         value;
        HashMap  			map;
    };

    public:
        RadixTree();
        RadixTree(const RadixTree& other) = delete;
        RadixTree(RadixTree&& other);
        ~RadixTree() noexcept = default;

        RadixTree& operator=(const RadixTree& other) = delete;
        RadixTree& operator=(RadixTree&& other);

        void                 clear() noexcept;
        bool                 contains(const std::string& key) const noexcept;
        std::shared_ptr<T>   find(const std::string& value) const noexcept;
        std::shared_ptr<T>   findPrefix(const std::string& value) const noexcept;
        bool                 insert(const std::string& value, const std::shared_ptr<T>& data, bool replace = false);

    #ifdef DEBUG    
        void printTree() const noexcept;
    #endif

    private:
        std::unique_ptr<Node> _root;

        uint32_t              _compare(const std::string& value1, const std::string& value2) const noexcept;
        std::shared_ptr<T>    _find(const std::unique_ptr<Node>& node, const std::string& value) const noexcept;
        std::shared_ptr<T>    _findPrefix(const std::unique_ptr<Node>& node, const std::string& value) const noexcept;
        bool                  _insert(std::unique_ptr<Node>& root, Node& node, bool replace);
        bool                  _split(std::unique_ptr<Node>& root, Node& node, uint32_t index);

    #ifdef DEBUG
       void _printTree(const std::unique_ptr<Node>& node, const std::string& prefix) const noexcept;
    #endif
};

/************** NODE *************/
template <typename T>
RadixTree<T>::Node::Node()
{
    data   = nullptr;
    is_end = false;
}

template <typename T>
RadixTree<T>::Node::Node(const std::string& value, const std::shared_ptr<T>& data, bool is_end)
{
    this->value  = value;
    this->data   = data ? std::make_shared<T>(*data) : nullptr;
    this->is_end = is_end;
}

template <typename T>
RadixTree<T>::Node::Node(Node&& other)
{
    *this = std::move(other);
}

template <typename T>
typename RadixTree<T>::Node& RadixTree<T>::Node::operator=(Node&& other)
{
    this->data   = std::move(other.data);
    this->is_end = other.is_end;
    this->map    = std::move(other.map);
    this->value  = std::move(other.value);
    other.is_end = false;
    other.data   = nullptr;
    return *this;
}

template <typename T>
bool RadixTree<T>::Node::addNode(Node& node)
{
    char key = node.value[0];

    return map.emplace(key, std::make_unique<Node>(std::move(node))).second;
}

/************** RADIX-TREE *************/

template <typename T>
RadixTree<T>::RadixTree() : _root(std::make_unique<Node>())
{
}

template <typename T>
RadixTree<T>::RadixTree(RadixTree&& other)
{
    *this = std::move(other);
}

template <typename T>
RadixTree<T>& RadixTree<T>::operator=(RadixTree&& other)
{
    _root = std::move(other._root);
    return *this;
}

template <typename T>
void RadixTree<T>::clear() noexcept
{
    _root->data.reset();
    _root->value.clear();
    _root->is_end = false;
    _root->map.clear();
}

template <typename T>
bool RadixTree<T>::contains(const std::string& key) const noexcept
{
    return find(key) != nullptr; // Todo: implement proper logic to differentiate when the T* is nullptr and when the key is not found
}

template <typename T>
bool RadixTree<T>::insert(const std::string& value, const std::shared_ptr<T>& data, bool replace)
{
    Node node(value, data, true);
    return _insert(_root, node, replace);
}

template <typename T>
std::shared_ptr<T> RadixTree<T>::find(const std::string& value) const noexcept
{
    auto it = _root->map.find(value[0]);
    if (it == _root->map.end()) {
        return nullptr;
    }
    return _find(it->second, value);
}

template <typename T>
std::shared_ptr<T> RadixTree<T>::_find(const std::unique_ptr<Node>& node, const std::string& value) const noexcept
{
    uint32_t index = _compare(node->value, value);
    if (index == 0) {
        return node->is_end ? node->data : nullptr;
    }
    if (index >= value.size() || index < node->value.size()) return nullptr;

    auto it = node->map.find(value[index]);
    if (it == node->map.end()) {
        return nullptr;
    }
    return _find(it->second, &value[index]);
}

template <typename T>
std::shared_ptr<T> RadixTree<T>::findPrefix(const std::string& value) const noexcept
{
    auto it = _root->map.find(value[0]);
    if (it == _root->map.end()) {
        return _root->is_end ? _root->data : nullptr;
    }
    return _findPrefix(it->second, value);
}

template <typename T>
std::shared_ptr<T> RadixTree<T>::_findPrefix(const std::unique_ptr<Node>& node, const std::string& value) const noexcept
{
    uint32_t index = _compare(node->value, value);
    if (index == 0 || index >= value.size()) {
        return node->is_end ? node->data : nullptr;
    }
    if (index < node->value.size()) return nullptr;

    auto it = node->map.find(value[index]);
    if (it == node->map.end()) {
        return node->is_end ? node->data : nullptr;
    }
    std::shared_ptr<T> data = _findPrefix(it->second, &value[index]);
    if (data == nullptr) {
        return node->is_end ? node->data : nullptr;
    }
    return data;
}

#ifdef DEBUG
template <typename T>
void RadixTree<T>::printTree() const noexcept
{
    _printTree(_root, " ");
}

template <typename T>
void RadixTree<T>::_printTree(const std::unique_ptr<Node>& node, const std::string& prefix) const noexcept
{
    if (!node->value.empty()) {
        std::cout << prefix << node->value << (node->is_end ? " [END]" : "") << std::endl;
    }

    for (auto it = node->map.begin(); it != node->map.end(); it++) {
        _printTree(it->second, prefix + "     ");
    }
}
#endif

template <typename T>
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

template <typename T>
bool RadixTree<T>::_insert(std::unique_ptr<Node>& root, Node& node, bool replace)
{
    auto it = root->map.find(node.value[0]);
    if (it == root->map.end()) {
        return root->addNode(node);
    }

    auto& next = it->second;
    uint32_t index = _compare(next->value, node.value);
    if (index == 0) {
        if (next->is_end && !replace) return false;

        next->data = std::move(node.data);
        next->is_end = true;
        return true;
    }
    if (index < next->value.size()) {
        return _split(next, node, index);
    }
    node.value = node.value.substr(index);
    return _insert(next, node, replace);
}

template <typename T>
bool RadixTree<T>::_split(std::unique_ptr<Node>& root, Node& node, uint32_t index)
{
    Node new_node(&root->value[index], std::move(root->data), root->is_end);

    root->is_end = false;
    root->data = nullptr;
    new_node.map = std::move(root->map);
    if (index < node.value.size()) {
        node.value = node.value.substr(index);
    }
    root->value = root->value.substr(0, index);

    return (_insert(root, new_node, true) && _insert(root, node, true));
}

#endif
