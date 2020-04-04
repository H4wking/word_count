#include <iostream>
#include <fstream>
#include <archive.h>
#include <archive_entry.h>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <string>
#include <map>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <thread>
#include <cmath>
#include "t_queue.h"
#include <boost/filesystem.hpp>
#include <numeric>
#include <exception>


namespace bl =boost::locale::boundary;
namespace fs = boost::filesystem;

inline std::chrono::high_resolution_clock::time_point get_current_time_fenced() {
    std::atomic_thread_fence(std::memory_order_seq_cst);
    auto res_time = std::chrono::high_resolution_clock::now();
    std::atomic_thread_fence(std::memory_order_seq_cst);
    return res_time;
}

template<class D>
inline long long to_us(const D &d) {
    return std::chrono::duration_cast<std::chrono::microseconds>(d).count();
}


std::vector<std::string> get_file_list(const std::string &path) {
    std::vector<std::string> m_file_list;
    if (fs::is_directory(path) && !fs::is_empty(path)) {
        fs::path apk_path(path);
        fs::recursive_directory_iterator end;

        for (auto &p : boost::filesystem::directory_iterator(path)) {
            m_file_list.push_back(p.path().string());
        }
    }
    return m_file_list;
}


void reading_from_archive(const std::string &buffer, t_queue<std::string> *tq) {
    struct archive *a;
    struct archive_entry *entry;
    la_int64_t r;
    off_t entry_size;
    a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);
    archive_read_support_format_raw(a);
    // read from buffer, not from the file
    if ((r = archive_read_open_memory(a, buffer.c_str(), buffer.size()))) {
        exit(1);
    }
    for (;;) {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF) {
            break;
        }
        if (r < ARCHIVE_OK) {
            std::cerr << archive_error_string(a) << std::endl;
        }
        if (r < ARCHIVE_WARN) {
            std::cerr << archive_errno(a) << std::endl;
            std::cerr << archive_error_string(a) << std::endl;
            exit(1);
        }

        // Do nothing if not txt files
        if (boost::filesystem::path(archive_entry_pathname(entry)).extension() == ".txt") {
            entry_size = archive_entry_size(entry);
            std::string output(entry_size, char{});
            //write explicitly to the other buffer
            r = archive_read_data(a, &output[0], output.size());
            tq->push_back(output);
        }

        if (r < ARCHIVE_WARN) {
            exit(1);
        }
    }
    archive_read_close(a);
    archive_read_free(a);
}

void read_from_dir(const std::vector<std::string> files, t_queue<std::string> *tq) {
    for (const auto &file_name : files) {
        if (fs::exists(file_name)) {
            std::ifstream raw_file(file_name, std::ios::binary);
            std::ostringstream buffer_ss;
            buffer_ss << raw_file.rdbuf();
            std::string buffer{buffer_ss.str()};
            if (fs::path(file_name).extension() == ".txt") {
                tq->push_back(buffer);
            } else if (fs::is_directory(fs::path(file_name))) {
                read_from_dir(get_file_list(file_name), tq);
            } else {
                reading_from_archive(buffer, tq);
            }
        } else {
            std::cerr << "File: " << file_name << "is't exists" << std::endl;
            exit(1);
        }
    }

}

template<class struct_t>
int write_file(const std::string &filename, struct_t mp) {
    std::ofstream out;
    out.open(filename, std::ios::trunc | std::ios::out | std::ios::binary);
    for (auto &it : mp) {
        out << boost::format("%1% %|15t| : %|25t| %2%\n") % it.first.c_str() % it.second;
    }
    out.close();
    return 0;
}


bool sort_by_val(const std::pair<std::string, int> &a, const std::pair<std::string, int> &b) {
    return (a.second > b.second);
}

std::vector<std::pair<std::string, int>> sort_by_value(std::map<std::string, int> mp) {
    std::map<std::string, int>::iterator it;

    std::vector<std::pair<std::string, int>> vec;

    for (it = mp.begin(); it != mp.end(); it++) {
        vec.emplace_back(it->first, it->second);
    }

    // // sort the vector by increasing order of its pair's second value
    sort(vec.begin(), vec.end(), sort_by_val);

    // print the vector
//    std::cout << "The map, sorted by value is: " << std::endl;
//    for (auto &it: vec) {
//        std::cout << boost::format("%1% %|15t| : %|25t| %2%\n") % it.first.c_str() % it.second;
//    }

    return vec;
}

void count_words_thr(int from, int to, std::vector<std::string> &words, std::map<std::string, int> &dict) {
    for (int i = from; i < to; i++) {
        if (!dict.count(words[i])) {
            dict.insert(std::pair<std::string, int>(words[i], 1));
        } else {
            dict[words[i]] += 1;
        }
    }
}

int main(int argc, char *argv[]) {
    boost::locale::generator gen;
    std::locale::global(gen("en_us.UTF-8"));
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
        std::cerr << "Could not open the configuration file. Set your working directory to ..\n";
        exit(2);
    }

    std::string in, out_a, out_n;
    int thr;

    conf >> in;
    conf >> out_a;
    conf >> out_n;
    conf >> thr;
    t_queue<std::string> tq;
    std::vector<std::string> root;
    root.push_back(in);
    read_from_dir(root, &tq);


//    char *txt = extract_from_archive(buffer);
    std::string str_txt;
    while (tq.get_size()) {
        str_txt += std::string(tq.pop());
        str_txt += "\n";
    }
//    std::string str_txt(tq.pop());
    boost::algorithm::to_lower(str_txt);
//    delete[] txt;
    boost::locale::normalize(str_txt);
    boost::locale::fold_case(str_txt);

    bl::ssegment_index map(bl::word, str_txt.begin(), str_txt.end());
// Define a rule
    map.rule(bl::word_letters);
// Print all "words" -- chunks of word boundary

    auto start = get_current_time_fenced();

    std::map<std::string, int> dict;

    if (thr == 1) {
        for (bl::ssegment_index::iterator it = map.begin(), e = map.end(); it != e; ++it) {
            ++dict[*it];
        }
    } else {
        std::vector<std::string> words;
        for (bl::ssegment_index::iterator it = map.begin(), e = map.end(); it != e; ++it) {
            words.push_back(*it);
        }
        std::map<std::string, int> dicts[thr];
        std::vector<std::thread> v;
        int s = words.size();

        for (int i = 0; i < thr - 1; ++i)
            v.emplace_back(count_words_thr, (s / thr) * i, (s / thr) * (i + 1),
                           std::ref(words), std::ref(dicts[i]));

        v.emplace_back(count_words_thr, (s / thr) * (thr - 1), s,
                       std::ref(words), std::ref(dicts[thr - 1]));

        for (auto &t: v) {
            t.join();
        }


        for (const auto &d : dicts) {
            for (auto &it : d) {
                if (!dict.count(it.first)) {
                    dict.insert(std::pair<std::string, int>(it.first, it.second));
                } else {
                    dict[it.first] += it.second;
                }
            }
        }
    }

    auto finish = get_current_time_fenced();

    auto total_time = finish - start;

    std::cout << "Time in us: " << to_us(total_time) << std::endl;

    const std::size_t result = std::accumulate(std::begin(dict), std::end(dict), 0,
                                               [](const std::size_t previous,
                                                  const std::pair<const std::string, std::size_t> &p) {
                                                   return previous + p.second;
                                               });
    std::cout << "Words total: " << result << "\n";

    write_file(out_a, dict);
    std::vector<std::pair<std::string, int>> vec = sort_by_value(dict);
    write_file(out_n, vec);

    return 0;
}
