#include <iostream>
#include <fstream>
#include <archive.h>
#include <archive_entry.h>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <string>
#include <map>
#include <vector>

static char *extract_from_archive(const std::string &buffer) {
    // initialize variable for libarchive
    struct archive_entry *entry;
    int r;
    int64_t entrysize;
    struct archive *a = archive_read_new();
    archive_read_support_filter_all(a);
    // all formats supporting like zip, tar ...
    archive_read_support_format_all(a);
    r = archive_read_open_memory(a, buffer.c_str(), buffer.size());
    r = archive_read_next_header(a, &entry);
    // finding size
    entrysize = archive_entry_size(entry);
    // allocating memory for out data-pointer
    char *txt_tmp = new char[entrysize];
    r = archive_read_data(a, txt_tmp, entrysize);
    archive_read_close(a);
    archive_read_free(a);
    return txt_tmp;
}


template<class struct_t>
int write_file(const std::string& filename, struct_t mp) {
    std::ofstream out;
    out.open(filename, std::ios::trunc | std::ios::out | std::ios::binary);
    for (auto &it : mp) {
        out << boost::format("%1% %|15t| : %|25t| %2%\n") % it.first.c_str() % it.second;
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
        vec.emplace_back(it->first, it->second);
    }

    // // sort the vector by increasing order of its pair's second value
    sort(vec.begin(), vec.end(), sortByVal);

    // print the vector
    std::cout << "The map, sorted by value is: " << std::endl;
    for (auto &it: mp) {
        std::cout << boost::format("%1% %|15t| : %|25t| %2%\n") % it.first.c_str() % it.second;
    }

    return vec;
}

int main(int argc, char *argv[]) {
    boost::locale::generator gen;
    std::locale::global(gen("en_US.UTF-8"));
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
    char *txt = extract_from_archive(buffer);
    std::string str_txt(txt);
    delete[] txt;
    namespace bl =boost::locale::boundary;
    boost::locale::normalize(str_txt);
    boost::locale::fold_case(str_txt);

    bl::ssegment_index map(bl::word, str_txt.begin(), str_txt.end());

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
    std::vector<std::pair<std::string, int>> vec = sort_by_value(dict);
    write_file(out_n, vec);

    return 0;
}
