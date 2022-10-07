#include <iostream>
#include <vector>

#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

#include <time.h>

void Sleep(uint64_t ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

uint64_t CurrentTimeNs()
{
    /* get monotonic clock time */
    struct timespec monotime;
    clock_gettime(CLOCK_MONOTONIC, &monotime);
    uint64_t t = monotime.tv_nsec + monotime.tv_sec * 1000000000L;
    return t;
}

uint64_t CurrentTimeMs()
{
    return CurrentTimeNs() / 1000 / 1000;
}

struct Item {
    uint64_t startTimeMs;
    uint64_t durationMs;
};

class WatchDog {
public:
    void Add(Item item)
    {
        tasks.push_back(item);
    }

    void Start()
    {
        std::thread t(&WatchDog::Check, this);
        thread = std::move(t);
    }

    void Destroy()
    {
        shutDown = true;
        cv.notify_all();
        thread.join();
    }

private:

    void Check()
    {
        while (!shutDown) {
            std::cout << "Dog checking..." << std::endl;
            std::unique_lock<std::mutex> ul(lock);
            cv.wait_for(ul, std::chrono::seconds(1));
            auto nowMs = CurrentTimeMs();
            for (auto &one : tasks) {
                if (nowMs > one.startTimeMs + one.durationMs) {
                    std::cout << "pds timeout startTimeMs = " << one.startTimeMs
                              << " durationMs = " << one.durationMs
                              << " nowMs = " << nowMs << std::endl;
                    abort();
                }
            }
        }

    }
    
    bool shutDown = false;
    std::vector<Item> tasks;
    std::mutex lock;
    std::condition_variable cv;
    std::thread thread;
};

int main(int argc, char** argv)
{
    std::cout << "hello" << std::endl;
    Item item;
    item.startTimeMs = CurrentTimeMs();
    item.durationMs = 5000;
    WatchDog dog;
    dog.Add(item);
    dog.Start();

    Sleep(10 * 1000);
    
    std::cout << "main end" << std::endl;
}
