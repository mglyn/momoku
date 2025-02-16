#include "threads.h"
#include "move.h"
#include "search.h"
#include "time.h"
#include "common.h"
#include "engine.h"

#include <algorithm>
#include <cassert>
#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

// Constructor launches the thread and waits until it goes to sleep
// in idle_loop(). Note that 'searching' and 'exit' should be already set.
Thread::Thread(SharedState& sharedState,
    std::unique_ptr<ISearchManager> sm,
    size_t idx) :
    idx(idx),
    stdThread(&Thread::idle_loop, this) {
    wait_for_search_finished();

    run_custom_job([this, &sharedState, &sm, idx]() {
        this->worker = std::make_unique<Worker>(sharedState, std::move(sm), idx);
        });

    wait_for_search_finished();
}


// Destructor wakes up the thread in idle_loop() and waits
// for its termination. Thread should be already waiting.
Thread::~Thread() {

    assert(!searching);

    exit = true;
    start_searching();
    stdThread.join();
}


// Wakes up the thread that will start the search
void Thread::start_searching() {
    assert(worker != nullptr);
    run_custom_job([this]() { worker->start_searching(); });
}


// Clears the histories for the thread worker (usually before a new game)
void Thread::clear_worker() {
    assert(worker != nullptr);
    run_custom_job([this]() { worker->clear(); });
}


// Blocks on the condition variable until the thread has finished searching
void Thread::wait_for_search_finished() {

    std::unique_lock<std::mutex> lk(mutex);
    cv.wait(lk, [&] { return !searching; });
}


// Launching a function in the thread
void Thread::run_custom_job(std::function<void()> f) {
    {
        std::unique_lock<std::mutex> lk(mutex);
        cv.wait(lk, [&] { return !searching; });
        jobFunc = std::move(f);
        searching = true;
    }
    cv.notify_one();
}


// Thread gets parked here, blocked on the condition variable
// when the thread has no work to do.
void Thread::idle_loop() {

    while (true) {

        std::unique_lock<std::mutex> lk(mutex);
        searching = false;
        cv.notify_one();  // Wake up anyone waiting for search finished
        cv.wait(lk, [&] { return searching; });

        if (exit)
            return;

        std::function<void()> job = std::move(jobFunc);
        jobFunc = nullptr;

        lk.unlock();

        if (job)
            job();
    }
}


SearchManager* ThreadPool::main_manager() const { return main_thread()->worker->main_manager(); }


uint64_t ThreadPool::nodes_searched() const { return accumulate(&Worker::nodes); }


// Creates/destroys threads to match the requested number.
// Created and launched threads will immediately go to sleep in idle_loop.
// Upon resizing, threads are recreated to allow for binding if necessary.
void ThreadPool::set(SharedState sharedState, 
    const SearchManager::UpdateContext& updateContext) {

    if (threads.size() > 0) { // destroy any existing thread(s)
        main_thread()->wait_for_search_finished();

        threads.clear();
    }

    const size_t requested = sharedState.options["Threads"];

    if (requested > 0) {    // create new thread(s)

        while (threads.size() < requested) {
            const size_t    threadId = threads.size();
            auto            manager = threadId == 0 ? std::unique_ptr<ISearchManager>(
                std::make_unique<SearchManager>(updateContext))
                : std::make_unique<NullSearchManager>();

            threads.emplace_back( std::make_unique<Thread>(sharedState, std::move(manager), threadId));
        }

        clear();

        main_thread()->wait_for_search_finished();
    }
}


// Sets threadPool data to initial values
void ThreadPool::clear() {
    if (threads.size() == 0)
        return;

    for (auto&& th : threads)
        th->clear_worker();

    for (auto&& th : threads)
        th->wait_for_search_finished();

    // These two affect the time taken on the first move of a game:
    main_manager()->bestPreviousAverageScore = VALUE_INFINITE;

    main_manager()->callsCnt = 0;
    main_manager()->bestPreviousScore = VALUE_INFINITE;
}


void ThreadPool::run_on_thread(size_t threadId, std::function<void()> f) {
    assert(threads.size() > threadId);
    threads[threadId]->run_custom_job(std::move(f));
}


void ThreadPool::wait_on_thread(size_t threadId) {
    assert(threads.size() > threadId);
    threads[threadId]->wait_for_search_finished();
}


size_t ThreadPool::num_threads() const { return threads.size(); }


// Wakes up main thread waiting in idle_loop() and returns immediately.
// Main thread will wake up other threads and start the search.
void ThreadPool::start_thinking(const Position& pos, LimitsType limits) {

    main_thread()->wait_for_search_finished();

    stop = abortedSearch = false;

    std::vector<RootMove> rootMoves;
    const auto legalmoves = genRootMove(pos);

    for (const auto& sq : limits.searchmoves) {
        if (std::find(legalmoves.begin(), legalmoves.end(), sq) != legalmoves.end())
            rootMoves.emplace_back(sq);
    }

    if (rootMoves.empty())
        for (const auto& m : legalmoves)
            rootMoves.emplace_back(m);


    // We use Position::set() to set root position across threads. But there are
    // some StateInfo fields (previous, pliesFromNull, capturedPiece) that cannot
    // be deduced from move sequence, so set() clears them and they are set from
    // states->back() later. The rootState is per thread, earlier states are
    // shared since they are read-only.
    for (auto&& th : threads) {
        th->run_custom_job([&]() {
            th->worker->limits = limits;
            th->worker->nodes = th->worker->bestMoveChanges = 0;
            th->worker->rootDepth = th->worker->completedDepth = 0;
            th->worker->rootMoves.assign(rootMoves.begin(), rootMoves.end());
            th->worker->rootPos.set(pos);
            });
    }

    for (auto&& th : threads)
        th->wait_for_search_finished();

    main_thread()->start_searching();
}


Thread* ThreadPool::get_best_thread() const {

    Thread* bestThread = threads.front().get();
    int minScore = VALUE_INFINITE;

    std::unordered_map <int, int64_t> votes(2 * std::min(size(), bestThread->worker->rootMoves.size()));

    // Find the minimum score of all threads
    for (auto&& th : threads)
        minScore = std::min(minScore, th->worker->rootMoves[0].score);

    // Vote according to score and depth, and select the best thread
    auto thread_voting_value = [minScore](Thread* th) {
        return (th->worker->rootMoves[0].score - minScore + 14) * int(th->worker->completedDepth);
        };

    for (auto&& th : threads)
        votes[th->worker->rootMoves[0].pv[0]] += thread_voting_value(th.get());

    for (auto&& th : threads) {
        int bestThreadScore = bestThread->worker->rootMoves[0].score;
        int newThreadScore = th->worker->rootMoves[0].score;

        auto& bestThreadPV = bestThread->worker->rootMoves[0].pv;
        auto& newThreadPV = th->worker->rootMoves[0].pv;

        auto bestThreadMoveVote = votes[bestThreadPV[0]];
        auto newThreadMoveVote = votes[newThreadPV[0]];

        bool bestThreadInProvenWin = is_win(bestThreadScore);
        bool newThreadInProvenWin = is_win(newThreadScore);

        bool bestThreadInProvenLoss = bestThreadScore != -VALUE_INFINITE && is_loss(bestThreadScore);
        bool newThreadInProvenLoss = newThreadScore != -VALUE_INFINITE && is_loss(newThreadScore);

        // We make sure not to pick a thread with truncated principal variation
        bool betterVotingValue =
            thread_voting_value(th.get()) * int(newThreadPV.size() > 2)
          > thread_voting_value(bestThread) * int(bestThreadPV.size() > 2);

          if (bestThreadInProvenWin) {
              // Make sure we pick the shortest mate
              if (newThreadScore > bestThreadScore)
                  bestThread = th.get();
          }
          else if (bestThreadInProvenLoss) {
              // Make sure we pick the shortest mated
              if (newThreadInProvenLoss && newThreadScore < bestThreadScore)
                  bestThread = th.get();
          }
          else if (newThreadInProvenWin || newThreadInProvenLoss ||
              (!is_loss(newThreadScore) && (newThreadMoveVote > bestThreadMoveVote || (newThreadMoveVote == bestThreadMoveVote && betterVotingValue)))) {
              bestThread = th.get();
          }
    }

    return bestThread;
}

// Start non-main threads.
// Will be invoked by main thread after it has started searching.
void ThreadPool::start_searching() {

    for (auto&& th : threads)
        if (th != threads.front())
            th->start_searching();
}


// Wait for non-main threads
void ThreadPool::wait_for_search_finished() const {

    for (auto&& th : threads)
        if (th != threads.front())
            th->wait_for_search_finished();
}

