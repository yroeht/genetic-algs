template<typename C, typename F>
void
Breeder<C, F>::start_workers()
{
  /* The population is equally divided among the workers by range. */
  using P = Population<C, F>;
  typename std::vector<typename P::iterator> bounds;
    {
      unsigned step = static_cast<unsigned>(population.size() / CPUS);
      for (unsigned i = 0; i < CPUS; ++i)
        bounds.push_back(std::next(population.begin(), step * i));
      bounds.push_back(population.end());
    }
  /* The job that will be executed by the workers. */
  auto thread_func = [this, bounds] (unsigned idx) -> void
    {
      auto lower_bound = bounds[idx];
      auto higher_bound = bounds[idx + 1];
      for (;;)
        {
          /* Use the std::condition_variable to wait until either the next
          ** generation must be processed, (which is signaled by resetting the
          ** processed[idx] boolean to false) or junction is required (in which
          ** case the kill_workers boolean flag has been set). */
            {
              std::unique_lock<std::mutex> lock(m_cv);
              cv.wait(lock, [this, idx] { return (processed[idx] == false
                                                  || kill_workers == true); });
              if (kill_workers)
                break;
            }
          /* Do the CPU heavy work (score the assigned population range). */
          std::for_each(lower_bound, higher_bound, [&] (Individual<C, F> i)
                        { i.second = scorer(i.first); });

          /* Let it be known that the work is now done. */
          processed[idx] = true;
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };

  /* Instantiate the workers. */

  /* Each worker is started with an index. This index is used to establish a
  ** unique posession of a boolean in the processed vector, which indicates
  ** whether or not there is some scoring to perform for the given thread. */
  processed.resize(CPUS);
  workers.resize(CPUS);
  std::generate(workers.begin(), workers.end(),
                [thread_func] { static unsigned i = 0;
                return std::thread(thread_func, i++); });
}

template<typename C, typename F>
void
Breeder<C, F>::score_threaded()
{

  /* This function is called at each generation, but the workers are only
  ** started once, effectively creating a pool. Creating new threads at each
  ** generation would be unefficient. */
  if (workers.empty())
    start_workers();

  /* Signal the workers that a new generation is underway, and that their
  ** slices are no longer in a scorer state (ie. go-back-to-work). */
  std::fill(processed.begin(), processed.end(), false);
  cv.notify_all();

  /* Semi-actively wait for all slices to have been processed. */
  for (;;)
    {
      if (std::all_of(processed.begin(), processed.end(),
                      [](bool b) -> bool { return b; }))
        break;
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

template<typename C, typename F>
void
Breeder<C, F>::stop_workers()
{
  /* Signal the workers to exit their loop. */
  kill_workers = true;
  cv.notify_all();
  for (auto& w : workers)
    if (w.joinable())
      w.join();
  workers.clear();
}
