#include "stack.h"

int &Stack::operator[](int idx) {
    int *cur = begin_ + idx;
    if (cur < begin_ || cur >= top_) { throw std::range_error { "index out of bounds" }; }
    return *cur;
}

void Stack::resize(int delta) {
    auto new_top = top_ + delta;
    if (delta < 0 && new_top < begin_) { throw std::range_error { "remove too many elements" }; }
    else if (delta > 0 && new_top > end_) { throw std::range_error { "add too many elements" }; }
    top_ = new_top;
}
