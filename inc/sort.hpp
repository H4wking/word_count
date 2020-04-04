#ifndef WORD_COUNT_SORT_H
#define WORD_COUNT_SORT_H

#include <string>
#include <map>
#include <vector>

bool sort_by_val(const std::pair<std::string, int> &a, const std::pair<std::string, int> &b);

std::vector<std::pair<std::string, int>> sort_by_value(std::map<std::string, int> mp);

#endif //WORD_COUNT_SORT_H
