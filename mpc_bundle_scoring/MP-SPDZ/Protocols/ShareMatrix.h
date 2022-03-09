/*
 * ShareMatrix.h
 *
 */

#ifndef PROTOCOLS_SHAREMATRIX_H_
#define PROTOCOLS_SHAREMATRIX_H_

#include <vector>
using namespace std;

#include "Share.h"
#include "FHE/AddableVector.h"

template<class T> class MatrixMC;

template<class T>
class ValueMatrix : public ValueInterface
{
    typedef ValueMatrix This;

public:
    int n_rows, n_cols;
    AddableVector<T> entries;

    static DataFieldType field_type()
    {
        return T::field_type();
    }

    ValueMatrix(int n_rows = 0, int n_cols = 0) :
            n_rows(n_rows), n_cols(n_cols), entries(n_rows * n_cols)
    {
        check();
    }

    template<class U>
    ValueMatrix(const ValueMatrix<U>& other) :
            n_rows(other.n_rows), n_cols(other.n_cols), entries(other.entries)
    {
        check();
    }

    void check() const
    {
        assert(entries.size() == size_t(n_rows * n_cols));
    }

    T& operator[](const pair<int, int>& indices)
    {
        assert(indices.first < n_rows);
        assert(indices.second < n_cols);
        return entries.at(indices.first * n_cols + indices.second);
    }

    const T& operator[](const pair<int, int>& indices) const
    {
        assert(indices.first < n_rows);
        assert(indices.second < n_cols);
        return entries.at(indices.first * n_cols + indices.second);
    }

    This& operator+=(const This& other)
    {
        entries += other.entries;
        check();
        return *this;
    }

    This operator-(const This& other) const
    {
        assert(entries.size() == other.entries.size());
        This res(n_rows, n_cols);
        res.entries = entries - other.entries;
        res.check();
        return res;
    }

    This operator*(const This& other) const
    {
        assert(n_cols == other.n_rows);
        This res(n_rows, other.n_cols);
        for (int i = 0; i < n_rows; i++)
            for (int j = 0; j < other.n_cols; j++)
                for (int k = 0; k < n_cols; k++)
                    res[{i, j}] += (*this)[{i, k}] * other[{k, j}];
        res.check();
        return res;
    }

    bool operator!=(const This& other) const
    {
        if (n_rows != other.n_rows or n_cols != other.n_cols)
            return true;
        return entries != other.entries;
    }

    void randomize(PRNG& G)
    {
        entries.randomize(G);
    }

    ValueMatrix transpose() const
    {
        ValueMatrix res(this->n_cols, this->n_rows);
        for (int i = 0; i < this->n_rows; i++)
            for (int j = 0; j < this->n_cols; j++)
                res[{j, i}] = (*this)[{i, j}];
        return res;
    }

    friend ostream& operator<<(ostream& o, const This&)
    {
        return o;
    }
};

template<class T>
class ShareMatrix : public ValueMatrix<T>, public ShareInterface
{
    typedef ShareMatrix This;
    typedef ValueMatrix<T> super;

public:
    typedef MatrixMC<T> MAC_Check;
    typedef Beaver<ShareMatrix> Protocol;
    typedef ::Input<This> Input;

    typedef ValueMatrix<typename T::clear> clear;
    typedef clear open_type;
    typedef typename T::clear mac_key_type;

    static string type_string()
    {
        return "matrix";
    }

    static This constant(const clear& other, int my_num, mac_key_type key)
    {
        This res(other.n_rows, other.n_cols);
        for (size_t i = 0; i < other.entries.size(); i++)
            res.entries[i] = T::constant(other.entries[i], my_num, key);
        res.check();
        return res;
    }

    ShareMatrix(int n_rows = 0, int n_cols = 0) :
            ValueMatrix<T>(n_rows, n_cols)
    {
    }

    template<class U>
    ShareMatrix(const U& other) :
        super(other)
    {
    }

    ShareMatrix from_row(int start, int size) const
    {
        ShareMatrix res(min(size, this->n_rows - start), this->n_cols);
        for (int i = 0; i < res.n_rows; i++)
            for (int j = 0; j < res.n_cols; j++)
                res[{i, j}] = (*this)[{start + i, j}];
        return res;
    }

    ShareMatrix from_col(int start, int size) const
    {
        ShareMatrix res(this->n_rows, min(size, this->n_cols - start));
        for (int i = 0; i < res.n_rows; i++)
            for (int j = 0; j < res.n_cols; j++)
                res[{i, j}] = (*this)[{i, start + j}];
        return res;
    }

    ShareMatrix from(int start_row, int start_col, int* sizes) const
    {
        ShareMatrix res(min(sizes[0], this->n_rows - start_row),
                min(sizes[1], this->n_cols - start_col));
        for (int i = 0; i < res.n_rows; i++)
            for (int j = 0; j < res.n_cols; j++)
                res[{i, j}] = (*this)[{start_row + i, start_col + j}];
        return res;
    }

    void add_from_col(int start, const ShareMatrix& other)
    {
        for (int i = 0; i < this->n_rows; i++)
            for (int j = 0; j < other.n_cols; j++)
                (*this)[{i, start + j}] += other[{i, j}];
    }
};

template<class T>
ShareMatrix<T> operator*(const ValueMatrix<typename T::clear>& a,
        const ShareMatrix<T>& b)
{
    assert(a.n_cols == b.n_rows);
    ShareMatrix<T> res(a.n_rows, b.n_cols);
    for (int i = 0; i < a.n_rows; i++)
        for (int j = 0; j < b.n_cols; j++)
            for (int k = 0; k < a.n_cols; k++)
                res[{i, j}] += a[{i, k}] * b[{k, j}];
    res.check();
    return res;
}

template<class T>
class MatrixMC : public MAC_Check_Base<ShareMatrix<T>>
{
    typename T::MAC_Check inner;

public:
    void exchange(const Player& P)
    {
        inner.init_open(P);
        for (auto& share : this->secrets)
        {
            share.check();
            for (auto& entry : share.entries)
                inner.prepare_open(entry);
        }
        inner.exchange(P);
        for (auto& share : this->secrets)
        {
            this->values.push_back({share.n_rows, share.n_cols});
            for (auto& entry : this->values.back().entries)
                entry = inner.finalize_open();
        }
    }
};

#endif /* PROTOCOLS_SHAREMATRIX_H_ */
