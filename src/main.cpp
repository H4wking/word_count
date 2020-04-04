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
#include "../inc/t_queue.h"
#include <boost/filesystem.hpp>
#include <numeric>
#include <exception>
#include "../inc/time.hpp"
#include "../inc/read_write.hpp"
#include "../inc/sort.hpp"



namespace bl = boost::locale::boundary;


void count_words_thr(int from, int to, std::vector<std::string> &words, std::map<std::string, int> &dict) {
    for (int i = from; i < to; i++) {
        ++dict[words[i]];
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

    auto start_load = get_current_time_fenced();

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

    auto start_count = get_current_time_fenced();

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
                dict[it.first] += it.second;
            }
        }
    }

    auto finish = get_current_time_fenced();

    auto load_time = start_count - start_load;
    auto count_time = finish - start_count;

    std::cout << "Loading: " << static_cast<float>(to_ms(load_time)) / 1000 << std::endl;
    std::cout << "Analyzing: " << static_cast<float>(to_ms(count_time)) / 1000 << std::endl;
    std::cout << "Total: " << static_cast<float>(to_ms(load_time + count_time)) / 1000 << std::endl;

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
