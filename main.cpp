#include <iostream>

#include <radix_tree.hpp>

int main(void)
{
    RadixTree tree;

    if (tree.insert("ciao", NULL) == false) {
        std::cerr << "fucking not working" << std::endl;
    }
    if (tree.insert("cia", NULL) == false) {
        std::cerr << "fucking not working" << std::endl;
    }
    if (tree.insert("ciaone", NULL) == false) {
        std::cerr << "fucking not working" << std::endl;
    }
    if (tree.insert("ciaoo", NULL) == false) {
        std::cerr << "fucking not working" << std::endl;
    }
    std::string* str = new std::string("ciwaua data");
    if (tree.insert("ciwaua", str) == false) {
        std::cerr << "fucking not working" << std::endl;
    }
    if (tree.insert("ci", NULL) == false) {
        std::cerr << "fucking not working" << std::endl;
    }
    if (tree.insert("cia", NULL) == false) {
        std::cerr << "fucking not working" << std::endl;
    }
    if (tree.insert("ciavatta", NULL) == false) {
        std::cerr << "fucking not working" << std::endl;
    }
    if (tree.insert("ciaonebellodecasa", NULL) == false) {
        std::cerr << "fucking not working" << std::endl;
    }
    if (tree.insert("ciaonebedda", NULL) == false) {
        std::cerr << "fucking not working" << std::endl;
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
