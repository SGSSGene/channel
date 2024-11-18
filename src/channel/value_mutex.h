// SPDX-FileCopyrightText: 2024 Simon Gene Gottlieb
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <mutex>
#include <tuple>

namespace channel {

/** Data Type that is forced to be locked to be accessible
 */
template <typename T>
class value_mutex {
    template <typename T2>
    class lock {
        std::lock_guard<std::mutex> lock_;
        T&                          value;
    public:
        lock(std::mutex& _mutex, T2& _value)
            : lock_{_mutex}
            , value{_value}
        {}
        auto operator->() -> T2* {
            return &value;
        }
        auto operator->() const -> T2 const* {
            return &value;
        }
    };

    mutable std::mutex mutex;
    T value;

public:
    template <typename ...Args>
    value_mutex(Args&&... args)
        : value{std::forward<Args>(args)...}
    {}

    auto operator*()&  {
        return lock<T>{mutex, value};
    }
    auto operator*() const& {
        return lock<T const>{mutex, value};
    }
    auto operator->() {
        return lock<T>{mutex, value};
    }
    auto operator->() const {
        return lock<T const>{mutex, value};
    }
};

}
