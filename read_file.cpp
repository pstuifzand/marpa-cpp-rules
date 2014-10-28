#include <fstream>
#include <string>

void read_file(const std::string& filename, std::string& input) {
    std::ifstream in(filename);
    ssize_t size = 8*1024;
    char buffer[size];
    while (in) {
        in.read(buffer, size);
        input.append(buffer, in.gcount());
    }
}

std::string read_file(const std::string& filename) {
    std::string input;
    read_file(filename, input);
    return input;
}

