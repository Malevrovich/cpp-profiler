// ─────────────────────────────────────────────────────────────────────────────
// Example 04 — String processing pipeline
//
// Simulates a text-processing pipeline:
//   1. Generate a large corpus of pseudo-random words
//   2. Tokenise each line into words
//   3. Count word frequencies in a hash map
//   4. Find the top-10 most frequent words
//
// The profiler report shows which stage dominates the runtime.
// ─────────────────────────────────────────────────────────────────────────────

#include <algorithm>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <vector>

// ── Corpus generation ─────────────────────────────────────────────────────────

static std::string GenerateWord(unsigned& state) {
  // XorShift32 for a deterministic pseudo-random word length and letters.
  state ^= state << 13;
  state ^= state >> 17;
  state ^= state << 5;
  const int len = 3 + static_cast<int>(state % 8);  // 3–10 chars
  std::string w(len, ' ');
  unsigned s2 = state;
  for (char& c : w) {
    s2 ^= s2 << 13;
    s2 ^= s2 >> 17;
    s2 ^= s2 << 5;
    c = 'a' + static_cast<char>(s2 % 26);
  }
  return w;
}

static std::vector<std::string> GenerateLines(int num_lines, int words_per_line) {
  std::vector<std::string> lines;
  lines.reserve(num_lines);
  unsigned state = 0xcafebabe;
  for (int i = 0; i < num_lines; ++i) {
    std::string line;
    for (int j = 0; j < words_per_line; ++j) {
      if (j > 0)
        line += ' ';
      line += GenerateWord(state);
    }
    lines.push_back(std::move(line));
  }
  return lines;
}

// ── Tokeniser ─────────────────────────────────────────────────────────────────

static std::vector<std::string> Tokenise(const std::string& line) {
  std::vector<std::string> tokens;
  std::size_t start = 0;
  while (start < line.size()) {
    const std::size_t end = line.find(' ', start);
    if (end == std::string::npos) {
      tokens.push_back(line.substr(start));
      break;
    }
    tokens.push_back(line.substr(start, end - start));
    start = end + 1;
  }
  return tokens;
}

// ── Frequency counter ─────────────────────────────────────────────────────────

static std::unordered_map<std::string, int> CountFrequencies(
    const std::vector<std::string>& lines) {
  std::unordered_map<std::string, int> freq;
  freq.reserve(4096);
  for (const auto& line : lines) {
    for (const auto& word : Tokenise(line)) {
      ++freq[word];
    }
  }
  return freq;
}

// ── Top-N ─────────────────────────────────────────────────────────────────────

static std::vector<std::pair<std::string, int>> TopN(
    const std::unordered_map<std::string, int>& freq, int n) {
  std::vector<std::pair<std::string, int>> v(freq.begin(), freq.end());
  std::partial_sort(v.begin(), v.begin() + std::min(n, static_cast<int>(v.size())), v.end(),
                    [](const auto& a, const auto& b) { return a.second > b.second; });
  if (static_cast<int>(v.size()) > n) {
    v.resize(n);
  }
  return v;
}

// ─────────────────────────────────────────────────────────────────────────────

int main() {
  constexpr int kLines = 2000;
  constexpr int kWordsPerLine = 20;
  constexpr int kTopN = 10;

  const auto lines = GenerateLines(kLines, kWordsPerLine);
  const auto freq = CountFrequencies(lines);
  const auto top = TopN(freq, kTopN);

  std::printf("Top %d words across %d lines (%d words each):\n", kTopN, kLines, kWordsPerLine);
  for (int i = 0; i < static_cast<int>(top.size()); ++i) {
    std::printf("  %2d. %-12s %d\n", i + 1, top[i].first.c_str(), top[i].second);
  }
  return 0;
}
