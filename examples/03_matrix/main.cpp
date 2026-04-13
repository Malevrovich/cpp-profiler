// ─────────────────────────────────────────────────────────────────────────────
// Example 03 — Matrix multiplication: naive vs cache-friendly
//
// Two implementations of N×N matrix multiply:
//   - NaiveMultiply:      accesses B column-by-column → cache misses
//   - TransposedMultiply: transposes B first → row-by-row access → cache friendly
//
// The profiler report shows the real cost of cache-unfriendly memory access.
// ─────────────────────────────────────────────────────────────────────────────

#include <cstdio>
#include <vector>

using Matrix = std::vector<std::vector<double>>;

static Matrix MakeMatrix(int n, double fill) {
  return Matrix(n, std::vector<double>(n, fill));
}

// ── Naive O(N³) multiply — column access on B causes cache misses ─────────────

static Matrix NaiveMultiply(const Matrix& A, const Matrix& B) {
  const int n = static_cast<int>(A.size());
  auto C = MakeMatrix(n, 0.0);
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      double sum = 0.0;
      for (int k = 0; k < n; ++k) {
        sum += A[i][k] * B[k][j];  // B[k][j]: stride-n access
      }
      C[i][j] = sum;
    }
  }
  return C;
}

// ── Cache-friendly multiply — transpose B first, then row×row ─────────────────

static Matrix Transpose(const Matrix& M) {
  const int n = static_cast<int>(M.size());
  auto T = MakeMatrix(n, 0.0);
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      T[j][i] = M[i][j];
    }
  }
  return T;
}

static Matrix TransposedMultiply(const Matrix& A, const Matrix& B) {
  const int n = static_cast<int>(A.size());
  const auto Bt = Transpose(B);
  auto C = MakeMatrix(n, 0.0);
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      double sum = 0.0;
      for (int k = 0; k < n; ++k) {
        sum += A[i][k] * Bt[j][k];  // Bt[j][k]: sequential access
      }
      C[i][j] = sum;
    }
  }
  return C;
}

// ─────────────────────────────────────────────────────────────────────────────

int main() {
  constexpr int kN = 200;

  // Fill with simple values so we can verify correctness.
  auto A = MakeMatrix(kN, 1.0);
  auto B = MakeMatrix(kN, 2.0);

  const auto C1 = NaiveMultiply(A, B);
  const auto C2 = TransposedMultiply(A, B);

  // Every element of C should equal N * 1.0 * 2.0 = 2*N.
  const double expected = static_cast<double>(kN) * 2.0;
  bool ok = true;
  for (int i = 0; i < kN && ok; ++i) {
    for (int j = 0; j < kN && ok; ++j) {
      if (C1[i][j] != C2[i][j] || C1[i][j] != expected) {
        ok = false;
      }
    }
  }

  std::printf("Matrix multiply %dx%d: results %s\n", kN, kN, ok ? "match" : "DIFFER");
  return ok ? 0 : 1;
}
