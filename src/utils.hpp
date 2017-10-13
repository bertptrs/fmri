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
	typedef float DType;

    /**
     * Identity function that simply returns whatever is put in.
     *
     * @tparam T The type of the function
     * @param t The value to return.
     * @return The original value.
     */
    template<class T>
    inline T identity(T t) {
        return t;
    }

    /**
     * Read a file into a vector of given type.
     * @tparam T The type to read.
     * @param filename the file to read
     * @return A vector of type T.
     */
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

    /**
     * String specialisation of read_vector.
     *
     * @param filename The filename to load.
     * @return A vector of the lines in the source file.
     */
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

    /**
     * Create a vector of pairs.
     *
     * @tparam T The first type
     * @tparam U The second type
     * @param a First vector
     * @param b Second vector
     * @return A vector of pair<U, V>
     */
    template<class T, class U>
    std::vector<std::pair<T, U>> combine(const std::vector<T>& a, const std::vector<U>& b)
    {
        std::vector<std::pair<T, U>> res;
        std::transform(a.begin(), a.end(), b.begin(),
                       std::back_inserter(res),
                       [] (const T& a, const U& b) -> auto { return std::make_pair(a, b); });

        return res;
    }

    template<class It>
    void clamp(It begin, It end,
               typename std::iterator_traits<It>::value_type minimum,
               typename std::iterator_traits<It>::value_type maximum)
    {
        const auto[minElem, maxElem] = std::minmax(begin, end);
        const auto diff = *maxElem - *minElem;

        const auto offset = minimum - *minElem;
        const auto scaling = (maximum - minimum) / diff;

        std::for_each(begin, end, [offset, scaling] (typename std::iterator_traits<It>::reference v) { v = (v + offset) * scaling;});
    }

}
