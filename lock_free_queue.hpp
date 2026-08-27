#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <new>

#define ALIGN_CACHE_LINE alignas(std::hardware_destructive_interference_size)

// Single Producer & Single Consumer
template <typename T, size_t N>
class SPSCQueue {
    static_assert((N & (N - 1)) == 0, "N should be power of two");

  public:
    bool enqueue(const T& item) noexcept {
        const auto head = head_.load(std::memory_order_acquire);
        const auto tail = tail_.load(std::memory_order_relaxed);
        if (increment(tail) == head) { return false; } // is full
        data_[tail] = item;
        tail_.store(increment(tail), std::memory_order_release);
        return true;
    }

    bool dequeue(T& item) noexcept {
        const auto head = head_.load(std::memory_order_relaxed);
        const auto tail = tail_.load(std::memory_order_acquire);
        if (head == tail) { return false; } // is empty
        item = std::move(data_[head]);
        head_.store(increment(head), std::memory_order_release);
        return true;
    }

  private:
    constexpr size_t increment(size_t n) noexcept { return (n + 1) & (N - 1); }

    ALIGN_CACHE_LINE std::atomic<size_t> head_{};
    ALIGN_CACHE_LINE std::atomic<size_t> tail_{};
    std::array<T, N> data_{};
};

// Multiple Producer & Single Consumer
template <typename T, size_t N>
class MPSCQueue {
    static_assert((N & (N - 1)) == 0, "N should be power of two");

  public:
    MPSCQueue() {
        for (size_t i = 0; i < N; ++i) { data_[i].seq.store(i, std::memory_order_relaxed); }
    }

    bool try_enqueue(const T& item) noexcept {
        auto pos = tail_.load(std::memory_order_relaxed);
        const auto idx = pos & (N - 1);
        const auto seq = data_[idx].seq.load(std::memory_order_acquire);
        const ptrdiff_t diff = static_cast<ptrdiff_t>(seq) - static_cast<ptrdiff_t>(pos);
        if (diff == 0) {
            if (tail_.compare_exchange_strong(pos, pos + 1, std::memory_order_relaxed, std::memory_order_relaxed)) {
                data_[idx].val = item;
                data_[idx].seq.store(pos + 1, std::memory_order_release);
                return true;
            }
        }
        return false;
    }

    bool enqueue(const T& item) noexcept {
        auto pos = tail_.load(std::memory_order_relaxed);
        while (true) {
            const auto idx = pos & (N - 1);
            const auto seq = data_[idx].seq.load(std::memory_order_acquire);
            const ptrdiff_t diff = static_cast<ptrdiff_t>(seq) - static_cast<ptrdiff_t>(pos);
            if (diff == 0) { // slot is available
                if (tail_.compare_exchange_strong(pos, pos + 1, std::memory_order_relaxed, std::memory_order_relaxed)) {
                    data_[idx].val = item;
                    data_[idx].seq.store(pos + 1, std::memory_order_release);
                    return true;
                }
            } else if (diff < 0) { // is full
                return false;
            } else { // other producers changed tail_
                pos = tail_.load(std::memory_order_relaxed);
            }
        }
    }

    bool try_dequeue(T& item) noexcept {
        auto pos = head_.load(std::memory_order_relaxed);
        const auto idx = pos & (N - 1);
        const auto seq = data_[idx].seq.load(std::memory_order_acquire);
        const ptrdiff_t diff = static_cast<ptrdiff_t>(seq) - static_cast<ptrdiff_t>(pos + 1);
        if (diff == 0) {
            item = std::move(data_[idx].val);
            data_[idx].seq.store(pos + N, std::memory_order_release);
            head_.store(pos + 1, std::memory_order_relaxed);
            return true;
        }
        return false;
    }

    bool dequeue(T& item) noexcept {
        auto pos = head_.load(std::memory_order_relaxed);
        while (true) {
            const auto idx = pos & (N - 1);
            const auto seq = data_[idx].seq.load(std::memory_order_acquire);
            const ptrdiff_t diff = static_cast<ptrdiff_t>(seq) - static_cast<ptrdiff_t>(pos + 1);
            if (diff == 0) {
                item = std::move(data_[idx].val);
                data_[idx].seq.store(pos + N, std::memory_order_release);
                head_.store(pos + 1, std::memory_order_relaxed);
                return true;
            } else if (diff < 0) {
                return false;
            }
        }
    }

  private:
    ALIGN_CACHE_LINE std::atomic<size_t> head_{};
    ALIGN_CACHE_LINE std::atomic<size_t> tail_{};
    struct ALIGN_CACHE_LINE Slot {
        T val{};
        std::atomic<size_t> seq{};
    };
    std::array<Slot, N> data_{};
};

// Multiple Producer & Multiple Consumer
template <typename T, size_t N>
class MPMCQueue {
    static_assert((N & (N - 1)) == 0, "N should be power of two");

  public:
    MPMCQueue() {
        for (size_t i = 0; i < N; ++i) { data_[i].seq.store(i, std::memory_order_relaxed); }
    }

    bool try_enqueue(const T& item) noexcept {
        auto pos = tail_.load(std::memory_order_relaxed);
        const auto idx = pos & (N - 1);
        const auto seq = data_[idx].seq.load(std::memory_order_acquire);
        const ptrdiff_t diff = static_cast<ptrdiff_t>(seq) - static_cast<ptrdiff_t>(pos);
        if (diff == 0) {
            if (tail_.compare_exchange_strong(pos, pos + 1, std::memory_order_relaxed, std::memory_order_relaxed)) {
                data_[idx].val = item;
                data_[idx].seq.store(pos + 1, std::memory_order_release);
                return true;
            }
        }
        return false;
    }

    bool enqueue(const T& item) noexcept {
        auto pos = tail_.load(std::memory_order_relaxed);
        while (true) {
            const auto idx = pos & (N - 1);
            const auto seq = data_[idx].seq.load(std::memory_order_acquire);
            const ptrdiff_t diff = static_cast<ptrdiff_t>(seq) - static_cast<ptrdiff_t>(pos);
            if (diff == 0) { // slot is available
                if (tail_.compare_exchange_strong(pos, pos + 1, std::memory_order_relaxed, std::memory_order_relaxed)) {
                    data_[idx].val = item;
                    data_[idx].seq.store(pos + 1, std::memory_order_release);
                    return true;
                }
            } else if (diff < 0) { // is full
                return false;
            } else { // other producers changed tail_
                pos = tail_.load(std::memory_order_relaxed);
            }
        }
    }

    bool try_dequeue(T& item) noexcept {
        auto pos = head_.load(std::memory_order_relaxed);
        const auto idx = pos & (N - 1);
        const auto seq = data_[idx].seq.load(std::memory_order_acquire);
        const ptrdiff_t diff = static_cast<ptrdiff_t>(seq) - static_cast<ptrdiff_t>(pos + 1);
        if (diff == 0) {
            if (head_.compare_exchange_strong(pos, pos + 1, std::memory_order_relaxed, std::memory_order_relaxed)) {
                item = std::move(data_[idx].val);
                data_[idx].seq.store(pos + N, std::memory_order_release);
                return true;
            }
        }
        return false;
    }

    bool dequeue(T& item) noexcept {
        auto pos = head_.load(std::memory_order_relaxed);
        while (true) {
            const auto idx = pos & (N - 1);
            const auto seq = data_[idx].seq.load(std::memory_order_acquire);
            const ptrdiff_t diff = static_cast<ptrdiff_t>(seq) - static_cast<ptrdiff_t>(pos + 1);
            if (diff == 0) {
                if (head_.compare_exchange_strong(pos, pos + 1, std::memory_order_relaxed, std::memory_order_relaxed)) {
                    item = std::move(data_[idx].val);
                    data_[idx].seq.store(pos + N, std::memory_order_release);
                    return true;
                }
            } else if (diff < 0) { // is empty
                return false;
            } else { // other consumers changed head_
                pos = head_.load(std::memory_order_relaxed);
            }
        }
    }

  private:
    ALIGN_CACHE_LINE std::atomic<size_t> head_{};
    ALIGN_CACHE_LINE std::atomic<size_t> tail_{};
    struct ALIGN_CACHE_LINE Slot {
        T val{};
        std::atomic<size_t> seq{};
    };
    std::array<Slot, N> data_{};
};
