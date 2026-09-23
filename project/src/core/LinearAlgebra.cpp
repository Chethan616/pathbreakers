#include "LinearAlgebra.h"

#include <algorithm>
#include <cmath>

namespace linalg {

Matrix::Matrix(int rows, int cols, double fill)
    : m_rows(rows), m_cols(cols), m_data(static_cast<size_t>(rows) * cols, fill) {}

Matrix Matrix::transpose(const Matrix& m) {
    Matrix out(m.cols(), m.rows());
    for (int r = 0; r < m.rows(); ++r)
        for (int c = 0; c < m.cols(); ++c)
            out.at(c, r) = m.at(r, c);
    return out;
}

Matrix Matrix::multiply(const Matrix& a, const Matrix& b) {
    Matrix out(a.rows(), b.cols(), 0.0);
    for (int r = 0; r < a.rows(); ++r)
        for (int k = 0; k < a.cols(); ++k) {
            double av = a.at(r, k);
            if (av == 0.0) continue;
            for (int c = 0; c < b.cols(); ++c)
                out.at(r, c) += av * b.at(k, c);
        }
    return out;
}

Vector Matrix::multiply(const Matrix& a, const Vector& v) {
    Vector out(a.rows(), 0.0);
    for (int r = 0; r < a.rows(); ++r) {
        double sum = 0.0;
        for (int c = 0; c < a.cols(); ++c) sum += a.at(r, c) * v[c];
        out[r] = sum;
    }
    return out;
}

Vector Matrix::solve(Matrix a, Vector b) {
    const int n = a.rows();
    if (a.cols() != n || static_cast<int>(b.size()) != n) return {};

    for (int col = 0; col < n; ++col) {
        // Partial pivot.
        int pivotRow = col;
        double best = std::abs(a.at(col, col));
        for (int r = col + 1; r < n; ++r) {
            double v = std::abs(a.at(r, col));
            if (v > best) { best = v; pivotRow = r; }
        }
        if (best < 1e-10) return {}; // singular

        if (pivotRow != col) {
            for (int c = 0; c < n; ++c) std::swap(a.at(col, c), a.at(pivotRow, c));
            std::swap(b[col], b[pivotRow]);
        }

        double pivot = a.at(col, col);
        for (int c = 0; c < n; ++c) a.at(col, c) /= pivot;
        b[col] /= pivot;

        for (int r = 0; r < n; ++r) {
            if (r == col) continue;
            double factor = a.at(r, col);
            if (factor == 0.0) continue;
            for (int c = 0; c < n; ++c) a.at(r, c) -= factor * a.at(col, c);
            b[r] -= factor * b[col];
        }
    }
    return b;
}

Vector fitLeastSquares(const Matrix& x, const Vector& y) {
    Matrix xt = Matrix::transpose(x);
    Matrix xtx = Matrix::multiply(xt, x);
    Vector xty = Matrix::multiply(xt, y);
    return Matrix::solve(xtx, xty);
}

} // namespace linalg
