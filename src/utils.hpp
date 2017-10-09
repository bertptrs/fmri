#pragma once

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

namespace fmri
{

    template<class T>
    inline T identity(T t) {
        return t;
    }

    template<class T>
    inline std::vector <T> read_vector(const std::string& filename)
    {
        std::ifstream input(filename);
        assert(input.good());

        std::vector<T> res;
        std::transform(std::istream_iterator<T>(input),
                       std::istream_iterator<T>(),
                       identity<T>, std::back_inserter(res));

        return res;
    }

    template<>
    inline std::vector<std::string> read_vector<std::string>(const std::string& filename)
    {
        std::ifstream input(filename);
        assert(input.good());

        std::string v;
        std::vector<std::string> res;

        while (getline(input, v)) {
            res.push_back(v);
        }

        return res;
    }

    template<class T, class U>
    std::vector<std::pair<T, U>> combine(const std::vector<T>& a, const std::vector<U>& b)
    {
        std::vector<std::pair<T, U>> res;
        std::transform(a.begin(), a.end(), b.begin(),
                       std::back_inserter(res),
                       [] (const T& a, const U& b) -> auto { return std::make_pair(a, b); });

        return res;
    }

}