#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

template <typename T>
class indexed_table {
    public:
        typedef typename std::vector<T>::iterator iterator;
        typedef typename std::vector<T>::const_iterator const_iterator;
    public:
        int add(const T& v) {
            auto it = std::find(symbols.begin(), symbols.end(), v);
            if (it == symbols.end()) {
                symbols.push_back(v);
                return symbols.size();
            }
            return std::distance(symbols.begin(), it) + 1;
        }
        const T& operator[](int idx) const {
            return symbols[idx-1];
        }

        iterator begin() { return symbols.begin(); }
        iterator end() { return symbols.end(); }

        const_iterator begin() const { return symbols.begin(); }
        const_iterator end() const { return symbols.end(); }

        void clear() { symbols.clear(); }
        size_t size() const { return symbols.size(); }
    private:
        std::vector<T> symbols;
};

#endif
