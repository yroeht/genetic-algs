# Generic genetic algorithms

I recently found myself in need of such an implementation, while working on a
Rubik's Cube solver. It turned out to be a bit messy, so I started this as a
side project.

# The class template

```C++
template<typename Gene, typename Fitness>
class Breeder;
```

The `Gene` parameter is the type from which chromosomes are built. Chromosomes
are typically what the end user will be interested in: a sequence of genes. In
the N-queens problem, for example, a gene is an integer -- the number of the
row on which a queen is placed, and a chromosome is a candidate solution: an
ordered set of positions, such as 46352857.

Why is the parameter a `Gene`, and not a `Chromosome`? It seemed more convient,
avoiding a few `decltype`, but this is subject to change.

The `Fitness` parameter is the return type of the evaluation function for a
given chromosome. It must have comparison operators defined on it. `int` and
`double` are fine. Note that the algorithm tries to maximize the fitness, so
higher means better.

This may also get abstracted.

# The constructor

```C++
Breeder(Generator<Gene>, Scorer<Gene, Fitness>,
        unsigned population_size_, double mutation_rate_);
```

The generator argument is a function that returns a fresh, random, chromosome.

The scorer argument is a function that gives the fitness of a chromosome.

The third (optional) argument specifies a default size for the population.

The last (optional) argument specifies a rate for spontaneous mutations. It
must be comprised between 0 and 1.

# The public member

```C++
  Chromosome<Gene>          pick(unsigned generations, Fitness score);
```

Return the fittest individual after given maximum amount of generations, or if
one's fitness surpasses the second argument.

If compiled with `-DCPUS=N`, the method will create N threads to perform
the calculations. It will also proceed to destroy them at the end.

This call is, for obvious reasons, blocking.

# The derived class

`PrintableBreeder` adds some verbose output to the evolution process. This adds
a requirement to the `Gene` type: it must have an overloaded `operator<<`.
