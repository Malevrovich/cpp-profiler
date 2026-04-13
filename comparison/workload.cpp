#include "workload.h"

#include <algorithm>
#include <cstdio>
#include <vector>

// ── Fibonacci ─────────────────────────────────────────────────────────────────

long long Fib(int n) {
  if (n <= 1)
    return n;
  return Fib(n - 1) + Fib(n - 2);
}

// ── Bubble sort ───────────────────────────────────────────────────────────────

void BubbleSort(std::vector<int>& v) {
  const std::size_t n = v.size();
  for (std::size_t i = 0; i < n; ++i) {
    for (std::size_t j = 0; j + 1 < n - i; ++j) {
      if (v[j] > v[j + 1]) {
        std::swap(v[j], v[j + 1]);
      }
    }
  }
}

// ── Merge sort ────────────────────────────────────────────────────────────────

static void Merge(std::vector<int>& v, int lo, int mid, int hi) {
  std::vector<int> tmp(v.begin() + lo, v.begin() + hi + 1);
  int left = 0;
  int right = mid - lo + 1;
  const int end = hi - lo + 1;
  for (int k = lo; k <= hi; ++k) {
    if (left > mid - lo) {
      v[k] = tmp[right++];
    } else if (right >= end) {
      v[k] = tmp[left++];
    } else if (tmp[left] <= tmp[right]) {
      v[k] = tmp[left++];
    } else {
      v[k] = tmp[right++];
    }
  }
}

void MergeSort(std::vector<int>& v, int lo, int hi) {
  if (lo >= hi)
    return;
  const int mid = lo + (hi - lo) / 2;
  MergeSort(v, lo, mid);
  MergeSort(v, mid + 1, hi);
  Merge(v, lo, mid, hi);
}

// ── Matrix multiply ───────────────────────────────────────────────────────────

Matrix NaiveMultiply(const Matrix& A, const Matrix& B) {
  const int n = static_cast<int>(A.size());
  Matrix C(n, std::vector<double>(n, 0.0));
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      double sum = 0.0;
      for (int k = 0; k < n; ++k) {
        sum += A[i][k] * B[k][j];
      }
      C[i][j] = sum;
    }
  }
  return C;
}

// ── RunWorkload ───────────────────────────────────────────────────────────────

void RunWorkload() {
  // 1. Fibonacci
  const long long fib = Fib(30);

  // 2. Sorting — bubble sort on 5 000 elements
  constexpr int kSortN = 5000;
  std::vector<int> data(kSortN);
  unsigned state = 0xdeadbeef;
  for (auto& x : data) {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    x = static_cast<int>(state & 0x7fffffff);
  }
  auto bubble_data = data;
  BubbleSort(bubble_data);

  auto merge_data = data;
  MergeSort(merge_data, 0, kSortN - 1);

  // 3. Matrix multiply — 150×150
  constexpr int kMatN = 150;
  Matrix A(kMatN, std::vector<double>(kMatN, 1.0));
  Matrix B(kMatN, std::vector<double>(kMatN, 2.0));
  const Matrix C = NaiveMultiply(A, B);

  std::printf("Fib(30)=%lld  sorted=%s  C[0][0]=%.0f\n", fib,
              (bubble_data == merge_data) ? "ok" : "FAIL", C[0][0]);
}
