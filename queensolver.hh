#ifndef QUEENSOLVER_HH
# define QUEENSOLVER_HH

# include <algorithm>
# include <cassert>
# include <chrono>
# include <random>
# include <set>
# include <vector>

template<unsigned N>
class QueenSolver
{
public:
  static std::vector<int> generator();
  static int scorer(std::vector<int> queens);

  static const int max_conflicts = (N * (N - 1) / 2);
};

template<unsigned N>
std::vector<int>
QueenSolver<N>::generator()
{
  long seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine e (seed);
  auto rows = std::uniform_int_distribution<>(1, N);
  std::vector<int> ret(N);

  std::generate(ret.begin(), ret.end(), std::bind(rows, e));
  return ret;
}

template<unsigned N>
int
QueenSolver<N>::scorer(std::vector<int> queens)
{
  auto conflicts = 0;

  std::set<int> diags1;
  std::set<int> diags2;
  int row = 1;
  for (auto it = queens.begin();
       it != queens.end(); ++it, ++row)
    {
      conflicts += std::count(std::next(it, 1), queens.end(), *it);
      int diag1 = row - *it, diag2 = *it + row;
      if (diags1.count(diag1) == 0)
        diags1.insert(diag1);
      else
        ++conflicts;
      if (diags2.count(diag2) == 0)
        diags2.insert(diag2);
      else
        ++conflicts;
    }
  auto ret = static_cast<int>(max_conflicts - conflicts);
  assert (ret >= 0);
  return ret;
}

#endif /* QUEENSOLVER_HH */
