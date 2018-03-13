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
#include <ratio>
#include <chrono>

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

    /**
     * Get the current animation offset for a particular animation.
     *
     * @tparam Duration Duration type of length. Should be a specialisation of std::chrono::duration
     * @param length The length of the animation.
     * @return
     */
    template<class Duration>
    float getAnimationStep(const Duration &length) {
        using namespace std::chrono;

        static auto startingPoint = steady_clock::now();
        const auto modified_length = duration_cast<steady_clock::duration>(length);

        auto step = (steady_clock::now() - startingPoint) % modified_length;

        return static_cast<float>(step.count()) / static_cast<float>(modified_length.count());
    }

    /**
     * Perform an argsort partitioning on the first n elements.
     *
     * @tparam Iter
     * @tparam Compare
     * @param first First element
     * @param middle Sorting limit
     * @param last Past end iterator for range
     * @param compare Comparison function to use
     * @return A vector of the indices before the partitioning cut-off.
     */
    template<class Iter, class Compare>
    std::vector<std::size_t> arg_nth_element(Iter first, Iter middle, Iter last, Compare compare)
    {
        using namespace std;

        const auto n = static_cast<size_t>(distance(first, middle));
        const auto total = static_cast<size_t>(distance(first, last));

        vector<size_t> indices(total);
        iota(indices.begin(), indices.end(), 0u);

        nth_element(indices.begin(), indices.begin() + n, indices.end(), [=](size_t a, size_t b) {
            return compare(*(first + a), *(first + b));
        });

        indices.resize(n);

        return indices;
    }

    /**
     * Fix non-normal floating point values in a range.
     *
     * @tparam It
     * @param first Start of range iterator
     * @param last Past the end of range iterator
     * @param normalValue Value to assign to non-normal values. Default 1.
     */
    template<class It>
    inline void normalize(It first, It last, typename std::iterator_traits<It>::value_type normalValue = 1)
    {
        for (; first != last; ++first) {
            if (!std::isnormal(*first)) {
                *first = normalValue;
            }
        }
    }
}
