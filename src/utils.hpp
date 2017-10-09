#pragma once

#include <cassert>
#include <fstream>
#include <vector>
#include <string>

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

}