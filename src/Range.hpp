#pragma once

namespace fmri
{

    /**
     * Iterable that produces a specific range of integers.
     *
     * Useful to make an automatically typed for loop over an unknown integer type.
     *
     * @tparam T The integer type to use.
     */
    template<class T>
    class Range
    {
    private:
        T start_;
        T end_;

    public:
        /**
         * Construct a range from 0 to num, non inclusive.
         *
         * @param num
         */
        constexpr explicit Range(const T &num) : start_(0), end_(num) {};

        /**
         * Construct a range from start to end, non inclusive.
         *
         * @param start
         * @param end
         */
        constexpr Range(const T &start, const T &end) : start_(start), end_(end) {};

        class Iter
        {
        private:
            T cur_;

        public:
            constexpr explicit Iter(const T &cur) : cur_(cur)
            {};

            typedef std::bidirectional_iterator_tag iterator_category;
            typedef T value_type;
            typedef typename std::make_signed<T>::type difference_type;
            typedef T& reference;
            typedef T* pointer;

            constexpr bool operator!=(const Iter& o) { return o.cur_ != cur_; }
            constexpr Iter&operator++() { ++cur_; return *this; }
            constexpr Iter&operator--() { --cur_; return *this; }

            constexpr const T &operator*() { return cur_; }
        };

        typedef Iter const_iterator;
        typedef std::reverse_iterator<Iter> const_reverse_iterator;

        constexpr const_iterator begin() const { return Iter(start_); }
        constexpr const_iterator end() const { return Iter(end_); }
        constexpr const_reverse_iterator rbegin() const { return std::make_reverse_iterator(end()); }
        constexpr const_reverse_iterator rend() const { return std::make_reverse_iterator(begin()); }
    };
}
