#include <iostream>
#include <fstream>
#include <boost/locale/boundary.hpp>
#include <boost/locale/generator.hpp>
#include <string>
#include <locale>
#include <map>

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

    std::ifstream text(in, std::ios::in);

    std::string str((std::istreambuf_iterator<char>(text)),
                    std::istreambuf_iterator<char>());

    namespace bl =boost::locale::boundary;


    boost::locale::generator gen;
    std::locale::global(gen(""));
    bl::ssegment_index map(bl::word, str.begin(), str.end());

// Define a rule
    map.rule(bl::word_any);
// Print all "words" -- chunks of word boundary

    std::map<std::string, int> dict;
    for (bl::ssegment_index::iterator it = map.begin(), e = map.end(); it != e; ++it) {
        if (!dict.count(*it)) {
            dict.insert(std::pair<std::string, int>(*it, 1));
        } else {
            dict[*it] += 1;
        }
    }

    std::map<std::string, int>::iterator itr;
    std::cout << "\nThe map dict is: \n";
    std::cout << "\tKEY\tELEMENT\n";
    for (itr = dict.begin(); itr != dict.end(); ++itr) {
        std::cout << '\t' << itr->first
             << '\t' << itr->second << '\n';
    }
    std::cout << std::endl;


    return 0;
}
