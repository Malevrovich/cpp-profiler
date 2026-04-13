#pragma once

// Shared workload used by both the gprof and malevrovich-prof comparison targets.
// Contains three independent algorithms so the profiler report has several
// distinct hot functions to compare.

#include <vector>

// ── Recursive Fibonacci ───────────────────────────────────────────────────────
long long Fib(int n);

// ── Sorting ───────────────────────────────────────────────────────────────────
void BubbleSort(std::vector<int>& v);
void MergeSort(std::vector<int>& v, int lo, int hi);

// ── Matrix multiply ───────────────────────────────────────────────────────────
using Matrix = std::vector<std::vector<double>>;
Matrix NaiveMultiply(const Matrix& A, const Matrix& B);

// ── Entry point ───────────────────────────────────────────────────────────────
// Runs all three workloads and prints a one-line summary.
void RunWorkload();
