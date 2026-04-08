#ifndef SRC_HPP
#define SRC_HPP

#include "fraction.hpp"
#include <vector>
#include <algorithm>

// 如果你不需要使用 matrix 类，请将 IGNORE_MATRIX 改为 0
#define IGNORE_MATRIX 0

#if IGNORE_MATRIX

class matrix {
private:
    int m, n;
    fraction **data;
public:
    matrix() {
        m = n = 0;
        data = nullptr;
    }
    matrix(int m_, int n_);
    matrix(const matrix &obj);
    matrix(matrix &&obj) noexcept;
    ~matrix();
    matrix &operator=(const matrix &obj);
    fraction &operator()(int i, int j);
    friend matrix operator*(const matrix &lhs, const matrix &rhs);
    matrix transposition();
    fraction determination();
};

#endif

class resistive_network {
private:
    int n, m;
    std::vector<int> from_nodes;
    std::vector<int> to_nodes;
    std::vector<fraction> resistances;
    std::vector<std::vector<fraction>> L;

    fraction determinant(std::vector<std::vector<fraction>> mat) {
        int sz = mat.size();
        if (sz == 0) return fraction(1);
        fraction det(1);
        for (int i = 0; i < sz; ++i) {
            int pivot = i;
            for (int j = i; j < sz; ++j) {
                if (!(mat[j][i] == fraction(0))) {
                    pivot = j;
                    break;
                }
            }
            if (mat[pivot][i] == fraction(0)) {
                return fraction(0);
            }
            if (pivot != i) {
                std::swap(mat[i], mat[pivot]);
                det = det * fraction(-1);
            }
            det = det * mat[i][i];
            fraction inv = fraction(1) / mat[i][i];
            for (int j = i + 1; j < sz; ++j) {
                fraction factor = mat[j][i] * inv;
                for (int k = i; k < sz; ++k) {
                    mat[j][k] = mat[j][k] - factor * mat[i][k];
                }
            }
        }
        return det;
    }

public:
    resistive_network(int interface_size_, int connection_size_, int from[], int to[], fraction resistance[]) {
        n = interface_size_;
        m = connection_size_;
        from_nodes.assign(from, from + m);
        to_nodes.assign(to, to + m);
        resistances.assign(resistance, resistance + m);

        L.assign(n, std::vector<fraction>(n, fraction(0)));
        for (int i = 0; i < m; ++i) {
            int u = from[i] - 1;
            int v = to[i] - 1;
            fraction c = fraction(1) / resistance[i];
            L[u][u] = L[u][u] + c;
            L[v][v] = L[v][v] + c;
            L[u][v] = L[u][v] - c;
            L[v][u] = L[v][u] - c;
        }
    }

    ~resistive_network() = default;

    fraction get_equivalent_resistance(int interface_id1, int interface_id2) {
        if (interface_id1 == interface_id2) return fraction(0);
        int u = interface_id1 - 1;
        int v = interface_id2 - 1;
        if (u > v) std::swap(u, v);

        std::vector<std::vector<fraction>> M_u(n - 1, std::vector<fraction>(n - 1));
        for (int i = 0, r = 0; i < n; ++i) {
            if (i == u) continue;
            for (int j = 0, c = 0; j < n; ++j) {
                if (j == u) continue;
                M_u[r][c] = L[i][j];
                c++;
            }
            r++;
        }
        fraction det_M_u = determinant(M_u);

        std::vector<std::vector<fraction>> M_uv(n - 2, std::vector<fraction>(n - 2));
        for (int i = 0, r = 0; i < n; ++i) {
            if (i == u || i == v) continue;
            for (int j = 0, c = 0; j < n; ++j) {
                if (j == u || j == v) continue;
                M_uv[r][c] = L[i][j];
                c++;
            }
            r++;
        }
        fraction det_M_uv = determinant(M_uv);

        return det_M_uv / det_M_u;
    }

    fraction get_voltage(int id, fraction current[]) {
        int target = id - 1;
        if (target == n - 1) return fraction(0);

        std::vector<std::vector<fraction>> M_n(n - 1, std::vector<fraction>(n - 1));
        for (int i = 0; i < n - 1; ++i) {
            for (int j = 0; j < n - 1; ++j) {
                M_n[i][j] = L[i][j];
            }
        }
        fraction det_M_n = determinant(M_n);

        std::vector<std::vector<fraction>> M_n_target = M_n;
        for (int i = 0; i < n - 1; ++i) {
            M_n_target[i][target] = current[i];
        }
        fraction det_M_n_target = determinant(M_n_target);

        return det_M_n_target / det_M_n;
    }

    fraction get_power(fraction voltage[]) {
        fraction power(0);
        for (int i = 0; i < m; ++i) {
            int u = from_nodes[i] - 1;
            int v = to_nodes[i] - 1;
            fraction v_diff = voltage[u] - voltage[v];
            power = power + (v_diff * v_diff) / resistances[i];
        }
        return power;
    }
};

#endif //SRC_HPP