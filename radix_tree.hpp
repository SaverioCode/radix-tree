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
    struct Node;

    struct Node
    {
        Node();
        Node(const Node& other) = delete;
        Node(Node&& other);
        Node(const std::string& value, const std::shared_ptr<T>& data, bool is_end);
        ~Node() noexcept = default;

        Node& operator=(const Node& other) = delete;
        Node& operator=(Node&& other);

        std::shared_ptr<T>              data;
        bool                            is_end;
        std::string                     value;
        std::unordered_map<char, Node>  map;
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
        Node _root;

        uint32_t              _compare(const std::string& value1, const std::string& value2) const noexcept;
        std::shared_ptr<T>    _find(const Node& node, const std::string& value) const noexcept;
        std::shared_ptr<T>    _findPrefix(const Node& node, const std::string& value) const noexcept;
        bool                  _insert(Node& root, Node& node, bool replace);
        bool                  _split(Node& root, Node& node, uint32_t index);

    #ifdef DEBUG
       void _printTree(const Node& node, const std::string& prefix) const noexcept;
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
    this->data = data;
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
    this->is_end = std::move(other.is_end);
    this->map    = std::move(other.map);
    this->value  = std::move(other.value);
    other.data = nullptr;
    return *this;
}

/************** RADIX-TREE *************/

template <typename T>
RadixTree<T>::RadixTree() : _root()
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
    _root = Node();
}

template <typename T>
bool RadixTree<T>::contains(const std::string& key) const noexcept
{
    return find(key) != nullptr;
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
    auto it = _root.map.find(value[0]);
    if (it == _root.map.end()) {
        return nullptr;
    }
    return _find(it->second, value);
}

template <typename T>
std::shared_ptr<T> RadixTree<T>::_find(const Node& node, const std::string& value) const noexcept
{
    uint32_t index = _compare(node.value, value);
    if (index == 0) {
        return node.is_end ? node.data : nullptr;
    }
    if (index >= value.size() || index < node.value.size()) return nullptr;
    auto it = node.map.find(value[index]);
    if (it == node.map.end()) {
        return nullptr;
    }
    return _find(it->second, &value[index]);
}

template <typename T>
std::shared_ptr<T> RadixTree<T>::findPrefix(const std::string& value) const noexcept
{
    auto it = _root.map.find(value[0]);
    if (it == _root.map.end()) {
        return _root.is_end ? _root.data : nullptr;
    }
    return _findPrefix(it->second, value);
}

template <typename T>
std::shared_ptr<T> RadixTree<T>::_findPrefix(const Node& node, const std::string& value) const noexcept
{
    uint32_t index = _compare(node.value, value);
    if (index == 0 || index >= value.size()) {
        return node.is_end ? node.data : nullptr;
    }
    if (index < node.value.size()) return nullptr;
    auto it = node.map.find(value[index]);
    if (it == node.map.end()) {
        return node.is_end ? node.data : nullptr;
    }
    std::shared_ptr<T> data = _findPrefix(it->second, &value[index]);
    if (data == nullptr) {
        return node.is_end ? node.data : nullptr;
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
void RadixTree<T>::_printTree(const Node& node, const std::string& prefix) const noexcept
{
    if (!node.value.empty()) {
        std::cout << prefix << node.value << (node.is_end ? " [END]" : "") << std::endl;
    }

    for (auto it = node.map.begin(); it != node.map.end(); it++) {
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
bool RadixTree<T>::_insert(Node& root, Node& node, bool replace)
{
    auto it = root.map.find(node.value[0]);
    if (it == root.map.end()) {
        return root.map.emplace(node.value[0], std::move(node)).second;
    }
    else {
        auto& next = it->second;
        uint32_t index = _compare(next.value, node.value);
        if (index == 0) {
            if (next.is_end && !replace) return false;

            next.data = std::move(node.data);
            next.is_end = true;
            return true;
        }
        if (index < next.value.size()) {
            return _split(next, node, index);
        }
        node.value = &node.value[index];
        return _insert(next, node, replace);
    }
}

template <typename T>
bool RadixTree<T>::_split(Node& root, Node& node, uint32_t index)
{
    Node new_node(&root.value[index], root.data, root.is_end);
    std::unordered_map<char, Node> tmp_map = std::move(root.map);

    root.data = nullptr;
    root.map = std::move(new_node.map);
    new_node.map = std::move(tmp_map);

    // Updated the value of the node to be inserted
    if (index < node.value.size()) {
        node.value = node.value.substr(index);
    }

    // Updated the value of the root node
    root.value = root.value.substr(0, index);
    root.is_end = false;

    if (_insert(root, new_node, true) && _insert(root, node, true)) {
        return true;
    }
    return false;
}

#endif
