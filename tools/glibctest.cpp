#include <iostream>

int main(int argc, char *argv[]) {
    std::cout << "Hello from C++!" << std::endl << "printing args" << std::endl;
    int *x = new int;
    for (int i = 0; i < argc; i++) {
        *x = i;
        std::cout << "argv[" << *x << "] -> " << argv[i] << std::endl;
    }
    delete x;
    return 0;
}
