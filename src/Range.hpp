#pragma once

namespace fmri
{
    template<class T>
    class Range
    {
    private:
        T start_;
        T end_;

    public:
        explicit Range(const T &num) : start_(0), end_(num) {};

        Range(const T &start, const T &end) : start_(start), end_(end) {};

        class Iter
        {
        private:
            T cur_;

        public:
            explicit Iter(const T &cur) : cur_(cur)
            {};

            bool operator!=(const Iter& o) { return o.cur_ != cur_; }
            Iter&operator++() { ++cur_; return *this; }

            T &operator*() { return cur_; }
        };

        typedef Iter const_iterator;

        const_iterator begin() const { return Iter(start_); }
        const_iterator end() const { return Iter(end_); }
    };
}