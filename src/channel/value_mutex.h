// SPDX-FileCopyrightText: 2024 Simon Gene Gottlieb
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <mutex>
#include <shared_mutex>
#include <utility>

namespace channel {

/** Data Type that is forced to be locked to be accessible
 */
template <typename T, bool Shared=false>
class value_mutex {
    using Mutex = std::conditional_t<Shared, std::shared_mutex, std::mutex>;

    template <typename T2, bool Shared2>
    struct lock {
        using Lock  = std::conditional_t<Shared2, std::shared_lock<Mutex>, std::unique_lock<Mutex>>;
        Lock lock_;
        T2*  value;

        auto operator*() -> T2& {
            return *value;
        }
        auto operator*() const -> T2 const& {
            return *value;
        }
        auto operator->() -> T2* {
            return value;
        }
        auto operator->() const -> T2 const* {
            return value;
        }
        void unlock() {
            lock_.unlock();
        }
    };

    mutable Mutex mutex;
    T value;
public:
    template <typename ...Args>
    value_mutex(Args&&... args)
        : value{std::forward<Args>(args)...}
    {}

    auto operator*() & {
        return lock<T, false>{std::unique_lock{mutex}, &value};
    }
    auto operator*() const& {
        using L = lock<T const, Shared>;
        return L{typename L::Lock{mutex}, &value};
    }
    auto operator->() & {
        return **this;
    }
    auto operator->() const& {
        return **this;
    }
};

}
