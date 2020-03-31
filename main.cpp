#include <iostream>
#include <sstream>
#include <fstream>
#include <archive.h>
#include <archive_entry.h>
#include <boost/locale/boundary.hpp>
#include <boost/locale/generator.hpp>
#include <string>
#include <map>
#include <vector>
#include <locale>


static void extract_to_vector(const std::string &buffer, std::vector<std::string> *vector_txt){
    struct archive_entry *entry;
    int r;
    off_t filesize;
    struct archive *a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);
    r = archive_read_open_memory(a, buffer.c_str(), buffer.size());
    r = archive_read_next_header(a, &entry);
    filesize = archive_entry_size(entry);
    std::string output(filesize, char{});
    r = archive_read_data(a, &output[0], output.size());
    vector_txt->push_back(output);
    archive_read_close(a);
    archive_read_free(a);
}


int main(int argc, char *argv[]) {
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

    std::string in, out_a, out_n;
    int thr;

    conf >> in;
    conf >> out_a;
    conf >> out_n;
    conf >> thr;
    std::ifstream raw_file(in, std::ios::binary);
    std::ostringstream buffer_ss;
    buffer_ss << raw_file.rdbuf();
    std::string buffer{buffer_ss.str()};
    std::vector<std::string> files_txt;
    extract_to_vector(buffer, &files_txt);

    namespace bl =boost::locale::boundary;


    boost::locale::generator gen;
    std::locale::global(gen(""));
    bl::ssegment_index map(bl::word, files_txt[0].begin(), files_txt[0].end());

// Define a rule
    map.rule(bl::word_letter);
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
