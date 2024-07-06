#include <iostream>
#include <WikipediaReader.hpp>
#include <Tokeniser.hpp>

int main(int argc, char **argv) {
    if(argc != 2) {
        std::cerr << "Incorrect argument count" << std::endl;
        return 1;
    }

    char *filename = argv[1];
    std::unique_ptr<WikipediaReader> reader = std::make_unique<WikipediaReader>(filename);

    Tokeniser *tokeniser = new Tokeniser();
    tokeniser->train(*reader, 257);


    return 0;
}
