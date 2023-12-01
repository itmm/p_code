#pragma once

#include <cassert>
#include <functional>
#include <stdexcept>

class Stack {
        int *begin_;
        int *top_;
        const int *end_;

        void assert_not_empty() const {
            if (empty()) { throw std::range_error { "stack underflow" }; }
        }

    public:
        Stack(int *begin, const int *end): begin_ { begin }, top_ { begin }, end_ { end } {
            assert(begin && begin <= end);
        }

        [[nodiscard]] bool full() const { return top_ >= end_; }
        [[nodiscard]] bool empty() const { return top_ <= begin_; }
        [[nodiscard]] int size() const { return static_cast<int>(top_ - begin_); }

        void push(int value) {
            if (full()) { throw std::range_error { "stack overflow" }; }
            *top_++ = value;
        }

        int pop() {
            assert_not_empty();
            return *--top_;
        }

        int &operator[](int idx);

        void operator()(const std::function<int(int)>& fn) {
            assert_not_empty();
            top_[-1] = fn(top_[-1]);
        }

        void operator()(const std::function<int(int, int)>& fn) {
            if (top_ - begin_ < 2) { throw std::range_error { "not two arguments for binary" }; }
            int first { *--top_ };
            top_[-1] = fn(first, top_[-1]);
        }

        void resize(int delta);
};
