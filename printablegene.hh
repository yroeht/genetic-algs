#ifndef PRINTABLE_GENE_HH
# define PRINTABLE_GENE_HH

#include <ostream>

struct PrintableGene
{};

std::ostream& operator<<(std::ostream& os, const PrintableGene&);
std::ostream& operator<<(std::ostream& os, const PrintableGene&)
{
  return (os << "Printed!");
}

struct PrintableSolver
{
  static
  std::vector<PrintableGene> generator()
    {
      return std::vector<PrintableGene>(1);
    }
  static
  int scorer(std::vector<PrintableGene>)
    {
      return 42;
    }
};

#endif /* PRINTABLE_GENE_HH */
