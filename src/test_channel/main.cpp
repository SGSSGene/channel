// SPDX-FileCopyrightText: 2024 Simon Gene Gottlieb
// SPDX-License-Identifier: CC0-1.0
#include <catch2/catch_all.hpp>
#include <channel/channel.h>
#include <string>
#include <thread>
#include <value_mutex.h>
#include <vector>

TEST_CASE("Tests channel with vector", "[channel][vector]") {

    auto results = channel::value_mutex<std::vector<std::string>>{};

    {
        auto queue = channel::channel<std::string>{};

        // secure access to id
        auto sid = channel::value_mutex<size_t>{0ull};

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
                auto id = *sid;
                if (*id >= 1000) { // nothing left todo
                    return false;
                }
                *id += 1;
                auto local_id = *id;
                id.unlock();

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
        auto sid = channel::value_mutex<size_t>{0ull};

        // function that should be executed by every thread
        auto threadFunc = [&]() {
            // a sender/receiver object, every thread needs to have its own
            auto sender_receiver = queue.make_sender_receiver();
            sender_receiver.loop_or_idle([&](auto value) {
                if (auto iter = value.find('1'); iter != std::string::npos) {
                    results->emplace_back(value);
                }
            }, [&]() {
                // some pseudo work
                auto id = *sid;
                if (*id >= 1000) { // nothing left todo
                    return false;
                }
                *id += 1;
                auto local_id = *id;
                id.unlock();

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

TEST_CASE("Tests value_mutex", "[value_mutex]") {
    SECTION("simple int") {
        auto myint = channel::value_mutex<int>{};

        SECTION("test lock") {
            auto l = *myint;
            *l = 0;
        }
        SECTION("test const lock") {
            auto l = *std::as_const(myint);
            int x = *l; // read only

            // check writing doesn't compile
            []<typename T=decltype(l)>() {
                static_assert(!requires(T const& l) {
                    { *l = 0 };
                });
            }();
        }
    }

    SECTION("simple std::pair<int, int>") {
        auto mypair = channel::value_mutex<std::pair<int, int>>{};

        SECTION("test lock") {
            mypair->first = 0;
        }
        SECTION("test const lock") {
            int x = std::as_const(mypair)->first;

            // check writing doesn't compile
            []<typename T=decltype(mypair)>() {
                static_assert(!requires(T const& l) {
                    { l->first = 0 };
                });

            }();
        }
    }

    SECTION("test read lock") {
        // check this does not cause a dead lock
        auto myint = channel::value_mutex<int, /*.shared=*/true>{};
        auto l1 = *std::as_const(myint);
        auto l2 = *std::as_const(myint);
    }
}
