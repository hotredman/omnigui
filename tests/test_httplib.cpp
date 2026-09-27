#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#include <httplib.h>
#include <iostream>

int main() {
    httplib::Server svr;
    if (!svr.is_valid()) {
        std::cerr << "Error: httplib server could not be created\n";
        return 1;
    }

    svr.Get("/ping", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("pong", "text/plain");
    });

    std::cout << "[PASS] httplib compiled and initialized successfully on this platform!\n";
    return 0;
}
