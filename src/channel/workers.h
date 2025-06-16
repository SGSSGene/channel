// SPDX-FileCopyrightText: 2025 Simon Gene Gottlieb
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <thread>
#include <vector>

namespace channel {
struct workers {
    std::vector<std::jthread> workers_;

    template <typename CB>
    workers(size_t threadCt, CB&& cb) {
        if (threadCt == 0) {
            cb();
        } else for (size_t i{0}; i < threadCt; ++i) {
            workers_.emplace_back(cb);
        }
    }
    ~workers() {
        join();
    }
    void join() {
        workers_.clear();
    }
};
}
