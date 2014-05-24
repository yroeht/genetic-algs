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

template<typename Chromosome>
using Pair = std::pair<Chromosome, Chromosome>;

template<typename Chromosome, typename Fitness>
using Individual = std::pair<Chromosome, Fitness>;

template<typename Chromosome, typename Fitness>
using Population =  std::vector<Individual<Chromosome, Fitness>>;

template<typename Chromosome>
using Generator = std::function<Chromosome()>;

template<typename Chromosome, typename Fitness>
using Scorer = std::function<Fitness(Chromosome)>;

/* An implementation of a generic genetic algorithm. */
template<typename Chromosome, typename Fitness>
class Breeder
{
public:
  Breeder(Generator<Chromosome>, Scorer<Chromosome, Fitness>,
          unsigned population_size_ = DEF_POP,
          double mutation_rate_ = DEF_MUT_RATE);

  /* Return the fittest individual after given maximum amount of generations.
  ** If compiled with -DCPUS=N, the method will create N threads to perform
  ** the calculations. It will also proceed to destroy them at the end. */
  Chromosome                        pick(unsigned generations, Fitness score);

private:
  void                              cross(Chromosome&, Chromosome&) const;
  Individual<Chromosome, Fitness>   populator() const;
  void                              score_threaded();
  void                              sort();
  void                              thread_func();
  void                              start_workers();

protected:
  void                              evolve();
  void                              stop_workers();
  Population<Chromosome, Fitness>   population;

private:
  Generator<Chromosome>             generator;
  Scorer<Chromosome, Fitness>       scorer;
  std::default_random_engine        rng;
  std::function<int()>              crossover_rnd;
  std::function<int()>              mutation_rnd;
  /* Threaded stuff... */
  std::vector<std::thread>          workers;
  std::vector<bool>                 processed;
  std::condition_variable           cv;
  std::mutex                        m_cv;
  std::atomic<bool>                 kill_workers;

  long                              padding : 56;
};

/* This derived class provides an overload of the pick() method, which
** features a verbose output of the evolutionary process. This however adds a
** constraint on the Chromosome type: it must have an overload for operator<<.
** */
template<typename Chromosome, typename Fitness>
class PrintableBreeder : public Breeder<Chromosome, Fitness>
{
  using Breeder<Chromosome, Fitness>::evolve;
  using Breeder<Chromosome, Fitness>::stop_workers;
public:
  using Breeder<Chromosome, Fitness>::Breeder;
  using Breeder<Chromosome, Fitness>::pick;
  Chromosome                          pick(unsigned generations, Fitness score,
                                           std::ostream& os);
};

# include <breeder.hxx>
# include <breeder_thread.hxx>

#endif /* BREEDER_HH */
