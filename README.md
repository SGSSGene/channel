<!--
    SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
    SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
    SPDX-License-Identifier: CC-BY-4.0
-->

# Channel

Provides utility functions for threaded work loads.

## API
- `channel::value_mutex<T>` value with mutex
    Helps locking a mutex before accessing the value.
    Example 1:
    ```
    auto id = channel::value_mutex<int>{0};

    auto [g, v] = *id; // returns a guard and a pointer to the value
    *v = *v + 1; // safe read/write access to *v
    ```
    Example 2:
    ```
    // value_mutex with multiple read access
    auto id = channel::value_mutex<int, /*.shared=*/ true>{0};

    {
        auto [g, v] = *id; // returns a guard and a pointer with write access
        *v = *v + 1;
    }
    {
        auto [g, v] = *std::as_const(id); // guard with only read access
        int x = *v;
    }
    ```
- `channel::channel`/`channel::sender_receiver` a channel for processing messages
    Example 1
    ```
    // creates a channel FILA (First in Last out)(stack)
    auto ch = channel::channel<std::string>{};
    ```
    Example 2:
    ```
    // creates a channel FILA (First in First out)(queue)
    auto ch = channel::channel<std::string, std::deque>{};
    ```
    Example 3:
    ```
    // Every threads needs its own sender_receiver
    auto sender_receiver = ch.make_sender_receiver();
    ```
    Example 4:
    ```
        sender_receiver.send("message"); // sends a message
        // blocks until a message is arrived, or no message is available and all
        // sender/receivers are waiting to receive a message
        auto msg = sender_receiver.recv(); // -> std::optional<std::string>

        // if message is available, returns other wise std::nullopt
        auto msg2 = sender_receiver.try_recv(); // -> std::optional<std::string>
    ```
