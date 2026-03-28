#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <iomanip>
#include "ThreadPool.cpp"

long long total_wait_ms = 0;
long long total_exec_ms = 0;
int tasks_completed = 0;
std::mutex stats_mtx;

void record_metrics(long long wait, long long exec, int id)
{
    std::lock_guard<std::mutex> lock(stats_mtx);
    total_wait_ms += wait;
    total_exec_ms += exec;
    tasks_completed++;
    std::cout << "[Worker] Task #" << id << " done | Wait: " << wait << "ms | Work: " << exec << "ms" << std::endl;
}

void execute_task(int id, std::chrono::steady_clock::time_point created_at)
{
    auto start_exec = std::chrono::steady_clock::now();

    auto wait_ms = std::chrono::duration_cast<std::chrono::milliseconds>(start_exec - created_at).count();

    int work_duration = 6000 + (rand() % 6001);
    std::this_thread::sleep_for(std::chrono::milliseconds(work_duration));

    auto end_exec = std::chrono::steady_clock::now();

    auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_exec - start_exec).count();

    record_metrics(wait_ms, exec_ms, id);
}

int main() {
    const size_t num_queues = 3;
    const size_t threads_per_q = 2;
    const int tasks_to_send = 13;

    ThreadPool pool;
    pool.initialize(num_queues, threads_per_q);

    auto test_start = std::chrono::steady_clock::now();

    for (int i = 0; i < tasks_to_send; i++)
    {
        auto now = std::chrono::steady_clock::now();

        Task t([i, now]()
        {
            execute_task(i, now);
        }, i);

        pool.add_task(t);
        std::cout << "Added Task " << i << std::endl;

        if (i == 5)
        {
            std::cout << "\nPAUSING THREAD POOL\n";
            pool.pause();

            std::this_thread::sleep_for(std::chrono::seconds(5));

            std::cout << "RESUMING THREAD POOL\n\n";
            pool.resume();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "\nAll tasks sent. Finalizing processing..." << std::endl;
    pool.terminate();

    auto test_end = std::chrono::steady_clock::now();
    double total_duration = std::chrono::duration_cast<std::chrono::seconds>(test_end - test_start).count();

    double avg_wait = total_wait_ms * 1.0 / tasks_completed;
    double avg_exec = total_exec_ms * 1.0 / tasks_completed;

    double arrival_rate = tasks_completed * 1.0 / total_duration;
    double avg_q_len = (arrival_rate * (avg_wait / 1000.0)) / num_queues;

    std::cout << std::endl;
    std::cout << "Total Threads Created:     " << num_queues * threads_per_q << std::endl;
    std::cout << "Tasks Processed:           " << tasks_completed << std::endl;
    std::cout << "Average Wait Time:         " << std::fixed << std::setprecision(2) << avg_wait << " ms" << std::endl;
    std::cout << "Average Execution Time:    " << avg_exec / 1000.0 << " sec" << std::endl;
    std::cout << "Avg. Length Per Queue:     " << avg_q_len << " tasks" << std::endl;

    return 0;
}