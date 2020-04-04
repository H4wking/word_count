#ifndef TIME_HPP
#define TIME_HPP

#include <iostream>
#include <chrono>
#include <atomic>

inline std::chrono::high_resolution_clock::time_point get_current_time_fenced();

template<class D>
inline long long to_ms(const D &d);

#endif