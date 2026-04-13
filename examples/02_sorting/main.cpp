// ─────────────────────────────────────────────────────────────────────────────
// Example 02 — Sorting algorithm comparison
//
// Runs bubble sort, merge sort, and std::sort on the same 8 000-element array.
// The profiler report reveals the real CPU cost of each algorithm.
// ─────────────────────────────────────────────────────────────────────────────

#include <algorithm>
#include <cstdio>
#include <vector>

// ── Bubble sort ───────────────────────────────────────────────────────────────

static void BubbleSort(std::vector<int>& v) {
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

static void MergeSort(std::vector<int>& v, int lo, int hi) {
  if (lo >= hi)
    return;
  const int mid = lo + (hi - lo) / 2;
  MergeSort(v, lo, mid);
  MergeSort(v, mid + 1, hi);
  Merge(v, lo, mid, hi);
}

// ── Helpers ───────────────────────────────────────────────────────────────────

static std::vector<int> MakeData(int n) {
  std::vector<int> v(n);
  unsigned state = 0xdeadbeef;
  for (auto& x : v) {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    x = static_cast<int>(state & 0x7fffffff);
  }
  return v;
}

// ─────────────────────────────────────────────────────────────────────────────

int main() {
  constexpr int kN = 8000;
  const auto data = MakeData(kN);

  auto d1 = data;
  BubbleSort(d1);

  auto d2 = data;
  MergeSort(d2, 0, kN - 1);

  auto d3 = data;
  std::sort(d3.begin(), d3.end());

  if (d1 != d2 || d2 != d3) {
    std::fprintf(stderr, "Sort results differ!\n");
    return 1;
  }

  std::printf("All three sorts produced identical results for %d elements.\n", kN);
  return 0;
}
