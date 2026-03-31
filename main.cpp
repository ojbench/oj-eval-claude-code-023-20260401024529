#include <iostream>
#include "deque.hpp"

int main() {
    // Read test commands from stdin and execute
    sjtu::deque<int> dq;

    int n;
    std::cin >> n;

    for (int i = 0; i < n; ++i) {
        std::string cmd;
        std::cin >> cmd;

        if (cmd == "push_back") {
            int val;
            std::cin >> val;
            dq.push_back(val);
        } else if (cmd == "push_front") {
            int val;
            std::cin >> val;
            dq.push_front(val);
        } else if (cmd == "pop_back") {
            try {
                dq.pop_back();
            } catch (...) {
                std::cout << "error" << std::endl;
            }
        } else if (cmd == "pop_front") {
            try {
                dq.pop_front();
            } catch (...) {
                std::cout << "error" << std::endl;
            }
        } else if (cmd == "front") {
            try {
                std::cout << dq.front() << std::endl;
            } catch (...) {
                std::cout << "error" << std::endl;
            }
        } else if (cmd == "back") {
            try {
                std::cout << dq.back() << std::endl;
            } catch (...) {
                std::cout << "error" << std::endl;
            }
        } else if (cmd == "size") {
            std::cout << dq.size() << std::endl;
        } else if (cmd == "empty") {
            std::cout << (dq.empty() ? "true" : "false") << std::endl;
        } else if (cmd == "clear") {
            dq.clear();
        } else if (cmd == "at") {
            size_t pos;
            std::cin >> pos;
            try {
                std::cout << dq.at(pos) << std::endl;
            } catch (...) {
                std::cout << "error" << std::endl;
            }
        }
    }

    return 0;
}
