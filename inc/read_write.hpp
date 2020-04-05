#ifndef READ_WRITE_HPP
#define READ_WRITE_HPP

#include <archive.h>
#include <archive_entry.h>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <string>
#include <map>
#include <vector>
#include <boost/filesystem.hpp>
#include <numeric>
#include <exception>
#include "t_queue.h"

namespace fs = boost::filesystem;

std::vector<std::string> get_file_list(const std::string &path);

void reading_from_archive(const std::string &buffer, t_queue<std::string> *tq);

void read_from_dir(const std::vector<std::string> files, t_queue<std::string> *tq);

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

#endif