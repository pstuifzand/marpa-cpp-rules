#ifndef UTIL_H
#define UTIL_H

template <class I, class N>
std::pair<I, N> parse_digit(I first, I last, N base, typename std::iterator_traits<I>::value_type zero)
{
    if (first == last) return make_pair(last, 0);

    N number = N(0);

    while (first != last && isdigit(*first)) {
        number *= base;
        if (*first >= zero && *first < zero + base) {
            number += *first - zero;
            first++;
        }
    }

    return make_pair(first, number);
}

#endif
