#ifndef THREADS
#define THREADS

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>
#include <thread>

#include "position.h"
#include "search.h"

// Abstraction of a thread. It contains a pointer to the worker and a native thread.
// After construction, the native thread is started with idle_loop()
// waiting for a signal to start searching.
// When the signal is received, the thread starts searching and when
// the search is finished, it goes back to idle_loop() waiting for a new signal.
class Thread {
    std::mutex                mutex;
    std::condition_variable   cv;
    size_t                    idx;
    bool                      exit = false, searching = true;  // Set before starting std::thread
    std::thread               stdThread;

public:
    Thread(SharedState& sharedState,
        std::unique_ptr<ISearchManager> sm,
        size_t idx);
    virtual ~Thread();

    void idle_loop();
    void start_searching();
    void clear_worker();
    void run_custom_job(std::function<void()> f);

    // Thread has been slightly altered to allow running custom jobs, so
    // this name is no longer correct. However, this class (and ThreadPool)
    // require further work to make them properly generic while maintaining
    // appropriate specificity regarding search, from the point of view of an
    // outside user, so renaming of this function is left for whenever that happens.
    void   wait_for_search_finished();
    size_t id() const { return idx; }

    std::unique_ptr<Worker> worker;
    std::function<void()>   jobFunc;
};


// ThreadPool struct handles all the threads-related stuff like init, starting,
// parking and, most importantly, launching a thread. All the access to threads
// is done through this class.
class ThreadPool {

    std::vector<std::unique_ptr<Thread>> threads;

    uint64_t accumulate(std::atomic<uint64_t> Worker::* member) const {

        uint64_t sum = 0;
        for (auto&& th : threads)
            sum += (th->worker.get()->*member).load(std::memory_order_relaxed);
        return sum;
    }

public:
    ThreadPool() {}

    ~ThreadPool() {
        // destroy any existing thread(s)
        if (threads.size() > 0) {
            main_thread()->wait_for_search_finished();

            threads.clear();
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;

    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    void   start_thinking(const Position&, LimitsType);
    void   run_on_thread(size_t threadId, std::function<void()> f);
    void   wait_on_thread(size_t threadId);
    size_t num_threads() const;
    void   clear();
    void   set(SharedState, const SearchManager::UpdateContext&);

    SearchManager* main_manager() const;
    Thread* main_thread() const { return threads.front().get(); }
    uint64_t       nodes_searched() const;
    Thread* get_best_thread() const;
    void           start_searching();
    void           wait_for_search_finished() const;

    std::atomic_bool stop, abortedSearch;

    auto cbegin() const noexcept { return threads.cbegin(); }
    auto begin() noexcept { return threads.begin(); }
    auto end() noexcept { return threads.end(); }
    auto cend() const noexcept { return threads.cend(); }
    auto size() const noexcept { return threads.size(); }
    auto empty() const noexcept { return threads.empty(); }
};

#endif