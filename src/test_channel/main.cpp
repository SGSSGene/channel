// SPDX-FileCopyrightText: 2024 Simon Gene Gottlieb
// SPDX-License-Identifier: CC0-1.0
#include <catch2/catch_all.hpp>
#include <channel/channel.h>
#include <value_mutex.h>
#include <thread>
#include <string>
#include <vector>

TEST_CASE("Tests channel with vector", "[channel][vector]") {

    auto results = channel::value_mutex<std::vector<std::string>>{};

    {
        auto queue = channel::channel<std::string>{};

        // secure access to id
        std::mutex mutex;
        size_t id{};

        // function that should be executed by every thread
        auto threadFunc = [&]() {
            // a sender/receiver object, every threads needs to have its own
            auto sender_receiver = queue.make_sender_receiver();
            sender_receiver.loop_or_idle([&](auto value) {
                if (auto iter = value.find('1'); iter != std::string::npos) {
                    results->emplace_back(value);
                }
            }, [&]() {
                // some pseudo work
                auto g = std::unique_lock{mutex};
                if (id >= 1000) { // nothing left todo
                    return false;
                }
                auto local_id = (id += 1);
                g.unlock();

                auto subjobs = local_id % 7+1;
                for (size_t i{0}; i < subjobs; ++i) {
                    sender_receiver.send(std::to_string(local_id) + "." + std::to_string(i));
                }
                return true;
            });
        };


        // spawn 8 threads
        auto threads = std::vector<std::thread>{};
        for (size_t i{0}; i < 8; ++i) {
            threads.emplace_back(threadFunc);
        }

        // join all threads
        for (auto& t : threads) t.join();
    }

    CHECK(results->size() == 1710);
}

TEST_CASE("Tests channel with deque", "[channel][deque]") {

    auto results = channel::value_mutex<std::vector<std::string>>{};

    {
        auto queue = channel::channel<std::string, std::deque>{};

        // secure access to id
        std::mutex mutex;
        size_t id{};

        // function that should be executed by every thread
        auto threadFunc = [&]() {
            // a sender/receiver object, everythreads needs to have its own
            auto sender_receiver = queue.make_sender_receiver();
            sender_receiver.loop_or_idle([&](auto value) {
                if (auto iter = value.find('1'); iter != std::string::npos) {
                    results->emplace_back(value);
                }
            }, [&]() {
                // some pseudo work
                auto g = std::unique_lock{mutex};
                if (id >= 1000) { // nothing left todo
                    return false;
                }
                auto local_id = (id += 1);
                g.unlock();

                auto subjobs = local_id % 7+1;
                for (size_t i{0}; i < subjobs; ++i) {
                    sender_receiver.send(std::to_string(local_id) + "." + std::to_string(i));
                }
                return true;
            });
        };


        // spawn 8 threads
        auto threads = std::vector<std::thread>{};
        for (size_t i{0}; i < 8; ++i) {
            threads.emplace_back(threadFunc);
        }

        // join all threads
        for (auto& t : threads) t.join();

    }

    CHECK(results->size() == 1710);

}
