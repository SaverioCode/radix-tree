#include <iostream>
#include <utility>
#include <vector>

#include <radix_tree.hpp>

#define ERR_MSG_INSERT "Failed to insert value"

int main(void)
{
    RadixTree<std::string> tree;
    std::vector<std::pair<std::string, std::string*>> value_data;

    value_data.push_back({"ciao", nullptr});
    value_data.push_back({"cia", nullptr});
    value_data.push_back({"ciaone", nullptr});
    value_data.push_back({"ciaoo", nullptr});
    value_data.push_back({"ciwaua", new std::string("ciwaua data")});
    value_data.push_back({"ci", nullptr});
    value_data.push_back({"cia", nullptr});
    value_data.push_back({"ciavatta", nullptr});
    value_data.push_back({"ciaonebellodecasa", nullptr});
    value_data.push_back({"ciaonebedda", nullptr});

    for (auto& p : value_data) {
        if (tree.insert(p.first, p.second) == false) {
            std::cerr << ERR_MSG_INSERT << std::endl;
        }
    }

    tree.printTree(tree.getRoot());

    std::string* out = (std::string*)tree.find("ciwauaa");
    if (out != nullptr) {
        std::cout << *out << std::endl;
    }
    out = (std::string*)tree.findPrefix("ciwauaaaa");
    if (out != nullptr) {
        std::cout << *out << std::endl;
    }
}
