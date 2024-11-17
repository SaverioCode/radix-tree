#include <iostream>
#include <utility>

#include <radix_tree.hpp>


/************** NODE *************/

RadixTree::Node::Node()
{
    data  = NULL;
    is_end = false;
    map = new std::map<char, Node*>();
}

RadixTree::Node::Node(const std::string& value, const void* data, bool is_end)
{
    this->value = value;
    this->data = const_cast<void*>(data);
    this->is_end = is_end;
    this->map = new std::map<char, Node*>();
}

RadixTree::Node::~Node()
{
    delete data; // ToDo: needs to fucking reintroduce type, because delete is for object and having void* doesn't give enough info to understand what kind of addres is pointing to, interface can be a solution
    for (std::map<char, Node*>::iterator it = map->begin(); it != map->end(); it++) {
        delete it->second;
    }
    delete map;
}

/************** RADIX-TREE *************/

RadixTree::RadixTree()
{
    _root = new Node();
}

RadixTree::~RadixTree()
{
    delete _root;
}

bool RadixTree::insert(const std::string& value, const void* data, bool replace)
{
    return _insert(_root, new Node(value, data, true), replace);
}

void* RadixTree::find(const std::string& value)
{
    std::map<char, Node*>::iterator it = _root->map->find(value[0]);
    if (it == _root->map->end()) {
        return NULL;
    }
    return _find(it->second, value);
}

void* RadixTree::_find(const Node* node, const std::string& value)
{
    uint32_t index = _compare(node->value, value);
    if (index == 0) {
        return node->data;
    }
    std::map<char, Node*>::iterator it = node->map->find(value[index]);
    if (it == node->map->end()) {
        return NULL;
    }
    return _find(it->second, &value[index]);
}

void* RadixTree::findPrefix(const std::string& value)
{
    std::map<char, Node*>::iterator it = _root->map->find(value[0]);
    if (it == _root->map->end()) {
        std::cout << "main root fail findPrefix" << std::endl;
        return _root->is_end ? _root->data : NULL; // To change, the first node has to have the value "/", I think I should have a fallback if not setted from configile
    }
    return _findPrefix(it->second, value);
}

void* RadixTree::_findPrefix(const Node* node, const std::string& value)
{
    uint32_t index = _compare(node->value, value);
    if (index == 0) {
        return node->is_end ? node->data : NULL;
    }
    std::map<char, Node*>::iterator it = node->map->find(value[index]);
    if (it == node->map->end()) {
        return node->is_end ? node->data : NULL;
    }
    void* data = _findPrefix(it->second, &value[index]);
    if (data == NULL) {
        return node->is_end ? node->data : NULL;
    }
    return data;
}

void RadixTree::debug(Node* root, const std::string& prefix)
{
    if (root == NULL) return;

    if (!root->value.empty()) {
        std::cout << prefix << root->value << (root->is_end ? " [END]" : "") << "\n";
    }

    for (std::map<char, Node*>::iterator it = root->map->begin(); it != root->map->end(); it++) {
        debug(it->second, prefix + "     ");
    }
}

RadixTree::Node* RadixTree::getRoot() const
{
    return _root;
}

uint32_t RadixTree::_compare(const std::string& value1, const std::string& value2)
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

bool RadixTree::_insert(Node* root, Node* node, bool replace)
{
    std::map<char, Node*>::iterator it = root->map->find(node->value[0]);
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
                node->data = NULL;
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

void RadixTree::_split(Node* root, Node* node, uint32_t index)
{
    Node* new_node = new Node(); // se i == s2 -> map is the union or just the old one? se i < s2 -> map is new
    new_node->value = &root->value[index];
    new_node->data = root->data;
    new_node->is_end = root->is_end;

    if (index < node->value.size()) {
        root->value = root->value.substr(0, index);
        root->data = NULL;
        root->is_end = false;
        std::map<char, Node*>* map = new_node->map;
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

void RadixTree::_move(Node* dest, Node* src)
{
    dest->value = src->value;
    src->value.clear();
    delete dest->data;
    dest->data = src->data;
    src->data = NULL;
    dest->is_end = src->is_end;
}
