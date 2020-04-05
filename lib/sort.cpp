#include "../inc/sort.hpp"


bool sort_by_val(const std::pair<std::string, int> &a, const std::pair<std::string, int> &b) {
    return (a.second > b.second);
}

std::vector<std::pair<std::string, int>> sort_by_value(std::map<std::string, int> mp) {
    std::map<std::string, int>::iterator it;

    std::vector<std::pair<std::string, int>> vec;

    for (it = mp.begin(); it != mp.end(); it++) {
        vec.emplace_back(it->first, it->second);
    }

    sort(vec.begin(), vec.end(), sort_by_val);

    return vec;
}
