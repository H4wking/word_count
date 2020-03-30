#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
    std::string conf_file;
    if (argc == 1) {
        conf_file = "config.dat";
    } else if (argc == 2) {
        conf_file = argv[1];
    } else {
        std::cerr << "Insufficient amount of arguments.\n";
        exit(1);
    }

    std::ifstream conf(conf_file, std::ios::in);

    if (!conf.is_open()) {
        std::cerr << "Could not open the configuration file.\n";
        exit(2);
    }

    std::string in, out;
    int thr;

    conf >> in;
    conf >> out;
    conf >> thr;

    return 0;
}
