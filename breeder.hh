#ifndef BREEDER_HH
# define BREEDER_HH

# include <atomic>
# include <condition_variable>
# include <functional>
# include <iterator>
# include <mutex>
# include <ostream>
# include <random>
# include <thread>
# include <vector>

# define DEF_MUT_RATE   0.01
# define DEF_POP        1000

template<typename Gene>
using Chromosome = std::vector<Gene>;

template<typename Gene>
using Pair = std::pair<Chromosome<Gene>, Chromosome<Gene>>;

template<typename Gene, typename Fitness>
using Individual = std::pair<Chromosome<Gene>, Fitness>;

template<typename Gene, typename Fitness>
using Population =  std::vector<Individual<Gene, Fitness>>;

template<typename Gene>
using Generator = std::function<Chromosome<Gene>()>;

template<typename Gene, typename Fitness>
using Scorer = std::function<Fitness(Chromosome<Gene>)>;

/* An implementation of a generic genetic algorithm. */
template<typename Gene, typename Fitness>
class Breeder
{
public:
  Breeder(Generator<Gene>, Scorer<Gene, Fitness>);
  Breeder(Generator<Gene>, Scorer<Gene, Fitness>,
          unsigned population_size_);
  Breeder(Generator<Gene>, Scorer<Gene, Fitness>,
          unsigned population_size_, double mutation_rate_);

  /* Return the fittest individual after given maximum amount of generations.
  ** If compiled with -DCPUS=N, the method will create N threads to perform
  ** the calculations. It will also proceed to destroy them at the end. */
  Chromosome<Gene>          pick(unsigned generations, Fitness score);

private:
  void                      cross(Chromosome<Gene>&, Chromosome<Gene>&) const;
  Individual<Gene, Fitness> populator() const;
  void                      score_threaded();
  void                      sort();
  void                      thread_func();
  void                      start_workers();

protected:
  void                      evolve();
  void                      stop_workers();
  Population<Gene, Fitness> population;

private:
  Generator<Gene>               generator;
  Scorer<Gene, Fitness>         scorer;
  std::default_random_engine    rng;
  std::function<int()>          crossover_rnd;
  std::function<int()>          mutation_rnd;
  /* Threaded stuff... */
  std::vector<std::thread>      workers;
  std::vector<bool>             processed;
  std::condition_variable       cv;
  std::mutex                    m_cv;
  std::atomic<bool>             kill_workers;

  long                          padding : 56;
};

/* This derived class provides an overload of the pick() method, which
** features a verbose output of the evolutionary process. This however adds a
** constraint on the Gene type: it must have an overload for operator<<. */
template<typename Gene, typename Fitness>
class PrintableBreeder : public Breeder<Gene, Fitness>
{
  using Breeder<Gene, Fitness>::evolve;
  using Breeder<Gene, Fitness>::stop_workers;
public:
  using Breeder<Gene, Fitness>::Breeder;
  using Breeder<Gene, Fitness>::pick;
  Chromosome<Gene>              pick(unsigned generations, Fitness score,
                                     std::ostream& os);
};

# include <breeder.hxx>
# include <breeder_thread.hxx>

#endif /* BREEDER_HH */
