#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iterator>
#include <random>
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

    /**
     * @brief Scales a range of values into a fixed range of values.
     *
     * This method traverses the range twice, once to determine maximum
     * and minimum, and once again to modify all the values.
     *
     * @tparam It Iterator type. Will be used to determine value type.
     * @param first The start of the range to scale
     * @param last The end of the range to scale
     * @param minimum The desired minimum of the range.
     * @param maximum The desired maximum of the range.
     */
    template<class It>
    void rescale(const It first, const It last,
                 typename std::iterator_traits<It>::value_type minimum,
                 typename std::iterator_traits<It>::value_type maximum)
    {
        const auto[minElem, maxElem] = std::minmax_element(first, last);
        const auto rangeWidth = maximum - minimum;
        const auto valWidth = *maxElem - *minElem;

        if (valWidth == 0) {
            // Just fill the range with the minimum value, since
            std::fill(first, last, minimum);
        } else {
            const auto scaling = rangeWidth / valWidth;

            const auto minVal = *minElem;

            std::for_each(first, last, [=](typename std::iterator_traits<It>::reference v) {
                v = std::clamp((v - minVal) * scaling + minimum, minimum, maximum);
            });
        }
    }

    template<class T>
    constexpr inline T deg2rad(T val) {
        return val / 180 * M_PI;
    }

    /**
     * Compute the ideal number of columns for dividing up a range into a rectangle.
     *
     * @tparam T
     * @param elems number of elements to space out
     * @return the ideal number of columns
     */
    template<class T>
    inline T numCols(const T elems) {
        auto cols = static_cast<T>(ceil(sqrt(elems)));

        while (elems % cols) {
            ++cols;// TODO: this should probably be done analytically
        }

        return cols;
    }

    /**
     * Get a globally initialized random number generator.
     *
     * This RNG should always be used as a reference, to make sure the state actually updates.
     *
     * @return A reference to the global RNG.
     */
    std::default_random_engine& rng();

}
