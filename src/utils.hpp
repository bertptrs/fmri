#pragma once

#include <cassert>
#include <fstream>
#include <vector>
#include <string>
#include <utility>

namespace fmri
{

    template<class T>
    inline std::vector <T> read_vector(const std::string& filename)
    {
        std::ifstream input(filename);
        assert(input.good());

        T t;
        std::vector<T> res;

        while (input >> t) {
            res.push_back(t);
        }

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
        assert(a.size() == b.size());
        std::vector<std::pair<T, U>> res;

        for (size_t i = 0; i < a.size(); ++i) {
            res.emplace_back(a[i], b[i]);
        }

        return res;
    }

}