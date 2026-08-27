#include <deque>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

class sum_up {
  public:
    sum_up(int begin, int end) : begin_(begin), end_(end) {}

    int operator()() {
        int sum = 0;
        for (int i = begin_; i < end_; ++i) sum += i;
        return sum;
    }

  private:
    int begin_, end_;
};

static const int HW_CON_GUESS = 4;
static const int NUMBER = 10000;

int main() {
    int hw_con = std::thread::hardware_concurrency();
    int hw_con_real = hw_con == 0 ? HW_CON_GUESS : hw_con;

    std::cout << "Will execute on " << hw_con_real << " threads" << std::endl;

    // 1. build functors
    std::vector<sum_up> objects;
    int base = NUMBER / hw_con_real;
    int remainder = NUMBER % hw_con_real;
    int begin = 1;
    for (int i = 0; i < hw_con_real; ++i) {
        int end = begin + base;
        objects.emplace_back(begin, end);
        if (i < remainder) end += 1;
        begin = end;
    }

    // 2. define tasks
    std::deque<std::packaged_task<int()>> tasks;
    for (int i = 0; i < hw_con_real; ++i) { tasks.emplace_back(objects[i]); }

    // 3. get futures
    std::vector<std::future<int>> futures;
    for (int i = 0; i < hw_con_real; ++i) { futures.emplace_back(tasks[i].get_future()); }

    // 4. execute each task in a separate thread
    for (int i = 0; i < hw_con_real; ++i) {
        std::packaged_task<int()> task = std::move(tasks.front());
        tasks.pop_front();
        std::thread t(std::move(task));
        t.detach();
    }

    // 5. get results from futures
    int total_sum = 0;
    for (int i = 0; i < hw_con_real; ++i) { total_sum += futures[i].get(); }

    std::cout << "Sum of 1-" << NUMBER << " is: " << total_sum << std::endl;

    return 0;
}
