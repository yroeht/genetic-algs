#include <iostream>

#include <breeder.hh>
#include <printablegene.hh>
#include <queensolver.hh>

int main()
{
  const int n_queens = 18;
  PrintableBreeder<int, int> b (QueenSolver<n_queens>::generator,
                                QueenSolver<n_queens>::scorer,
                                10000, .3);
  auto best = b.pick(10, QueenSolver<n_queens>::max_conflicts, std::cout);

#if 0 // An other example.
  PrintableBreeder<PrintableGene, int> b2(PrintableSolver::generator,
                                          PrintableSolver::scorer,
                                          2);
  b2.pick(2, 42, std::cout);
  b2.pick(2, 42);
#endif
}

