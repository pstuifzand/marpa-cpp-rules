#ifndef UTIL_H
#define UTIL_H

template <class I, class N>
    // I models ForwardIterator
    // N models Integer
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

template <typename I>
I parse_ident(I first, I last)
{
    if (first == last) return first;
    if (!(isalpha(*first) || *first == '_'))  return first;
    ++first;
    while (isalnum(*first) || *first == '_') {
        ++first;
    }
    return first;
}

template <typename I>
I match(I first, I last, I s_first, I s_last) {
    auto p = std::mismatch(first, last, s_first);
    if (p.second == s_last) {
        return p.first;
    }
    return first;
}

template <typename I, typename P>
// requires ForwardIterator(I)
I skip(I first, I last, P pred) {
    return std::find_if_not(first, last, pred);
}

template <typename I, typename T>
I discard_until_after(I first, I last, const T& value)
{
    first = std::find(first, last, value);
    if (first != last) ++first;
    return first;
}

#endif
