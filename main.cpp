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
#include <boost/algorithm/string.hpp>

static void extract_to_vector(const std::string &buffer, std::vector<std::string> *vector_txt) {
    struct archive_entry *entry;
    int r;
    off_t filesize;
    struct archive *a = archive_read_new();
    archive_read_support_format_all(a);
//    archive_read_support_filter_all(a);
    r = archive_read_open_memory(a, buffer.c_str(), buffer.size());
    r = archive_read_next_header(a, &entry);
    filesize = archive_entry_size(entry);
    std::string output(filesize, char{});
    r = archive_read_data(a, &output[0], output.size());
    vector_txt->push_back(output);
    archive_read_close(a);
//    archive_read_free(a);
}


int write_file(std::string filename, std::map<std::string, int> mp) {
    std::wofstream out;
    out.open(filename, std::ios::out | std::ios::app);
    out.imbue(std::locale("en_US.UTF-8"));
    for (auto &it : mp) {
        out << it.first.c_str() << "  " << it.second << std::endl;
    }
    out.close();
    return 0;
}

int write_file2(std::string filename, std::vector<std::pair<std::string, int>> vec) {
    std::wofstream out;
    out.open(filename, std::ios::out | std::ios::app);
    out.imbue(std::locale("en_US.UTF-8"));
    for (const auto &it : vec) {
        out << it.first.c_str() << "  " << it.second << std::endl;
    }
    out.close();
    return 0;
}

bool sortByVal(const std::pair<std::string, int> &a, const std::pair<std::string, int> &b) {
    return (a.second > b.second);
}

std::vector<std::pair<std::string, int>> sort_by_value(std::map<std::string, int> mp) {
    std::map<std::string, int>::iterator it;

    std::vector<std::pair<std::string, int>> vec;

    for (it = mp.begin(); it != mp.end(); it++) {
        vec.push_back(make_pair(it->first, it->second));
    }

    // // sort the vector by increasing order of its pair's second value
    sort(vec.begin(), vec.end(), sortByVal);

    // print the vector
    std::cout << "The map, sorted by value is: " << std::endl;
    for (int i = 0; i < vec.size(); i++) {
        std::cout << vec[i].first << ": " << vec[i].second << std::endl;
    }

    return vec;
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

    boost::algorithm::to_lower(files_txt[0]);

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

    write_file(out_a, dict);

    std::map<std::string, int>::iterator itr;
    std::cout << "\nThe map dict is: \n";
    std::cout << "\tKEY\tELEMENT\n";
    for (itr = dict.begin(); itr != dict.end(); ++itr) {
        std::cout << '\t' << itr->first
                  << '\t' << itr->second << '\n';
    }
    std::cout << std::endl;

    std::vector<std::pair<std::string, int>> vec = sort_by_value(dict);

    write_file2(out_n, vec);

    return 0;
}
