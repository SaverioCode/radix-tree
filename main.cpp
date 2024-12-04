#include <iostream>
#include <utility>
#include <vector>

#include <radix_tree.hpp>

#define ERR_MSG_INSERT "Failed to insert value"

int main(void)
{
    RadixTree<std::string> tree;
    std::vector<std::pair<std::string, std::string*>> value_data;

    value_data.push_back({"ciao", NULL});
    value_data.push_back({"cia", NULL});
    value_data.push_back({"ciaone", NULL});
    value_data.push_back({"ciaoo", NULL});
    value_data.push_back({"ciwaua", new std::string("ciwaua data")});
    value_data.push_back({"ci", NULL});
    value_data.push_back({"cia", NULL});
    value_data.push_back({"ciavatta", NULL});
    value_data.push_back({"ciaonebellodecasa", NULL});
    value_data.push_back({"ciaonebedda", NULL});

    for (auto& p : value_data) {
        if (tree.insert(p.first, p.second) == false) {
            std::cerr << ERR_MSG_INSERT << std::endl;
        }
    }

    tree.debug(tree.getRoot());

    std::string* out = (std::string*)tree.find("ciwauaa");
    if (out != NULL) {
        std::cout << *out << std::endl;
    }
    out = (std::string*)tree.findPrefix("ciwauaaaa");
    if (out != NULL) {
        std::cout << *out << std::endl;
    }
}
