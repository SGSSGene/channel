// SPDX-FileCopyrightText: 2024 Simon Gene Gottlieb
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "value_mutex.h"
#include "workers.h"

#include <cassert>
#include <condition_variable>
#include <stdexcept>
#include <mutex>
#include <optional>
#include <deque>
#include <vector>

namespace channel {
template <typename channel_t>
class sender_receiver {
    using value_t = typename channel_t::value_t;
    channel_t* channel{};

public:
    sender_receiver(channel_t* c)
        : channel{c}
    {
        assert(channel);
        auto _ = std::lock_guard{channel->mutex};
        channel->number_of_sender_receiver += 1;
    }
    sender_receiver(sender_receiver const&) = delete;
    sender_receiver(sender_receiver&&) = delete;
    ~sender_receiver() {
        close();
    }

    auto operator=(sender_receiver const&) = delete;
    auto operator=(sender_receiver&&) = delete;

    void close() {
        if (channel) {
            auto _ = std::lock_guard{channel->mutex};
            channel->number_of_sender_receiver -= 1;
            channel->number_of_sender_receiver.notify_all();
            if (channel->number_of_sender_receiver > 0) {
                channel->cv_on_msg.notify_one();
            }
            channel = nullptr;
        }
    }

    // send value to channel
    void send(value_t value) {
        assert(channel);
        auto _ = std::lock_guard{channel->mutex};
        channel->insert(std::move(value));
        channel->cv_on_msg.notify_one();
    }

    // waits blocking for new message
    // if all sender_receiver are blocking, than one sender_receiver will return from `recv()`
    // returns a value if available, otherwise blocks, returns immediately if all
    //   sender_receiver are in a blocking call.
    [[nodiscard]]
    auto recv() -> std::optional<value_t> {
        assert(channel);
        auto g = std::unique_lock{channel->mutex};
        while (channel->empty()) {
            if (channel->number_of_blocking_recv+1
                == channel->number_of_sender_receiver) {
                return std::nullopt;
            }
            channel->number_of_blocking_recv += 1;

            channel->cv_on_msg.wait(g);
            channel->number_of_blocking_recv -= 1;
        }
        return channel->pop();
    }

    // peeks for new message
    // returns a value if available, otherwise std::nullopt
    [[nodiscard]]
    auto try_recv() -> std::optional<value_t> {
        assert(channel);
        auto g = std::unique_lock{channel->mutex};
        if (channel->empty()) return std::nullopt;
        return channel->pop();
    }

    // waits blocking for a new message
    // if all sender_receiver are blocking, execute idle function, until idle function returns false
    // returns a value if available, otherwise blocks, returns immediately if all
    //  sender_receiver are in blocking call and no idle work left
    template <typename CB>
    [[nodiscard]]
    auto recv_or_idle(CB idle) {
        do {
            if (auto value = try_recv()) {
                return value;
            }
        } while(idle());
        return recv();
    }

    // loops over the work,
    // if no work is available it will loop over idle_cb.
    // if idle_cb returns false, the loop will abort if also no work is available
    template <typename work_cb, typename idle_cb>
    void loop_or_idle(work_cb&& _work_cb, idle_cb&& _idle_cb) {
        while (true) {
            // function that receives data, and executes the idle callback
            // if nothing is available.
            auto value = recv_or_idle(_idle_cb);
            // If no value available,
            if (!value) break;
            _work_cb(std::move(*value));
        }
    }


};

template <typename _value_t, template <class, class...> typename type = std::vector>
class channel {
    static_assert(std::same_as<type<_value_t>, std::vector<_value_t>>
        || std::same_as<type<_value_t>, std::deque<_value_t>>);

    friend class sender_receiver<channel>;

    using value_t = _value_t;
    type<value_t> stack;

    std::atomic_size_t number_of_sender_receiver;
    size_t number_of_blocking_recv;

    std::mutex mutex;
    std::condition_variable cv_on_msg;

    [[nodiscard]]
    auto empty() const {
        return stack.empty();
    }
    template <typename T>
    void insert(T&& t) {
        stack.emplace_back(std::forward<T>(t));
    }

    [[nodiscard]]
    auto pop() {
        assert(stack.size());
        if constexpr (std::same_as<type<_value_t>, std::vector<_value_t>>) {
            auto top = stack.back();
            stack.pop_back();
            return top;
        } else {
            auto top = stack.front();
            stack.pop_front();
            return top;
        }
        throw std::runtime_error{"implementation error"};
    }
public:
    channel() = default;
    ~channel() {
        join();
    }

    // creates a new sender_receiver
    [[nodiscard]]
    auto make_sender_receiver() {
        return sender_receiver{this};
    }

    // waits until all sender_receiver are destroyed
    void join() {
        auto v = number_of_sender_receiver.load();
        while (v != 0) {
            number_of_sender_receiver.wait(v);
            v = number_of_sender_receiver.load();
        }
    }
};

}
