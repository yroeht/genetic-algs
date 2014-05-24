# include <algorithm>
# include <cassert>

using std::chrono::system_clock;

template<typename C, typename F>
Breeder<C, F>::Breeder(Generator<C> generator_,
                       Scorer<C, F> scorer_,
                       unsigned population_size_,
                       double mutation_rate_)
: generator(generator_)
, scorer(scorer_)
, rng(static_cast<unsigned>(system_clock::now().time_since_epoch().count()))
{
  /* Initialize the crossover_rnd RNG. */
    {
      assert (mutation_rate_ <= 1.0);
      mutation_rnd = std::bind(std::discrete_distribution<>{
                               1 - mutation_rate_, mutation_rate_}, rng);
    }

  /* Initialize the crossover_rnd RNG. */
    {
      auto size = static_cast<int>(generator().size() - 1);
      std::uniform_int_distribution<int> crossover_distribution;
      crossover_distribution = std::uniform_int_distribution<>(0, size);
      crossover_rnd = std::bind(crossover_distribution, rng);
    }

  /* Generate the inital population. */
    {
      population.resize(population_size_);
      std::generate(population.begin(), population.end(),
                    [this] { return populator(); });
      this->sort();
    }
}

/* The Generator fonction provided to the constructor returns a chromosome.
** This function makes an individual out of that chromosome, by pairing it up
** with its score. */
template<typename Chromosome, typename F>
Individual<Chromosome, F>
Breeder<Chromosome, F>::populator() const
{
  Chromosome c = generator();
  return {c, scorer(c)};
}

template<typename C, typename F>
void
Breeder<C, F>::sort()
{
  /* Sort individuals by comparing their fitness, which is the second element
  ** of the Individual pair. */
  auto cmp = [this](const Individual<C, F>& lhs,
                    const Individual<C, F>& rhs) -> bool
    {
      return (lhs.second > rhs.second);
    };
  /* Shuffling the vector might add the necessary entropy to avoid local
  ** maxima, but this might also be useless considering the top chromosomes
  ** will always get crossed, and thus get a new score. Also, AFAIK, local
  ** plateaus aren't really a problem with GA, because the point is not to get
  ** a 100% fitness, it's just to get as good as possible. */
  std::random_shuffle(population.begin(), population.end());
  std::sort(population.begin(), population.end(), cmp);
}

template<typename Chromosome, typename F>
void
Breeder<Chromosome, F>::cross(Chromosome& parent1, Chromosome& parent2) const
{
  auto crossover = crossover_rnd();
  auto crossover_point1 = std::next(parent1.begin(), crossover);
  auto crossover_point2 = std::next(parent2.begin(), crossover);

  /* Swap the first portions or both chromosomes. */
    {
      Chromosome swp (parent1.begin(), crossover_point1);
      std::move(parent2.begin(), crossover_point2, parent1.begin());
      std::move(swp.begin(), swp.end(), parent2.begin());
    }

  /* If the odds say so, a mutation occurs on one of the genes (of the first
  ** chromosome). There is no generator for a single gene, so take one from a
  ** temporary chromosome. */
  if (mutation_rnd())
    {
      auto mutation = static_cast<unsigned>(crossover_rnd());
      auto throw_away = generator();
      parent1[mutation] = throw_away.front();
    }
}

template<typename C, typename F>
void
Breeder<C, F>::evolve()
{
  /* Cross the 25% fittest individuals, two by two. */
  for (unsigned i = 0; i < population.size() / 4; i += 2)
    cross(population[i].first, population[i + 1].first);

  /* Replace the 10% least fit individuals with new ones. */
  std::generate(std::prev(population.end(), population.size() / 10),
                population.end(),
                [this] { return populator(); });

  /* Score the new population. */
#if CPUS
  score_threaded();
#else
  for (auto& i : population)
    i.second = scorer(i.first);
#endif
  this->sort();
}

template<typename Chromosome, typename F>
Chromosome
Breeder<Chromosome, F>::pick(unsigned max_generations, F max_score)
{
  for (unsigned generations = 0; generations < max_generations; ++generations)
    {
      this->evolve();
      if (population.front().second >= max_score)
        break;
    }
#if CPUS
  stop_workers();
#endif
  return population.front().first;
}

/* This is the same as its base Breeder::pick(), but with some verbosity in the
** middle. This code duplication is ugly, FIXME. */
template<typename Chromosome, typename F>
Chromosome
PrintableBreeder<Chromosome, F>::pick(unsigned max_generations, F max_score,
                                      std::ostream& os)
{
  for (unsigned generations = 0; generations < max_generations; ++generations)
    {
      this->evolve();
      os << "Generation " << generations << ", best=";
      os << this->population.front().first;
      os << " score=" << this->population.front().second << std::endl;

      if (this->population.front().second >= max_score)
        break;
    }
#if CPUS
  stop_workers();
#endif
  return this->population.front().first;
}
