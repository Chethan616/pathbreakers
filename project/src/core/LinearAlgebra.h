#pragma once

#include <vector>

// Minimal dense matrix + ordinary-least-squares solver. Deliberately small:
// the task-time estimator's one-hot-encoded feature count stays in the
// tens, so a full ML library would be overkill - normal equations solved by
// Gauss-Jordan elimination are enough and keep the app dependency-free.
namespace linalg {

using Vector = std::vector<double>;

class Matrix {
public:
    Matrix() = default;
    Matrix(int rows, int cols, double fill = 0.0);

    int rows() const { return m_rows; }
    int cols() const { return m_cols; }

    double& at(int r, int c) { return m_data[r * m_cols + c]; }
    double at(int r, int c) const { return m_data[r * m_cols + c]; }

    static Matrix transpose(const Matrix& m);
    static Matrix multiply(const Matrix& a, const Matrix& b);
    static Vector multiply(const Matrix& a, const Vector& v);

    // Solves A * x = b via Gauss-Jordan elimination with partial pivoting.
    // Returns an empty vector if A is singular.
    static Vector solve(Matrix a, Vector b);

private:
    int m_rows = 0;
    int m_cols = 0;
    std::vector<double> m_data;
};

// Fits y = X * beta by ordinary least squares (normal equations:
// beta = (X^T X)^-1 X^T y). X's rows are samples, columns are features
// (include a leading bias column of 1.0 if an intercept is wanted).
// Returns an empty vector if the system is singular (e.g. duplicate/collinear
// columns) - caller should fall back to a simple average in that case.
Vector fitLeastSquares(const Matrix& x, const Vector& y);

} // namespace linalg
