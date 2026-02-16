#include "BasicApp.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <spdlog/spdlog.h>

int main()
{
    spdlog::set_pattern("[%H:%M:%S] [%^--%L--%$] [thread %t] %v");
#ifdef NDEBUG
    spdlog::set_level(spdlog::level::info);
#else
    spdlog::set_level(spdlog::level::debug);
#endif

    vpe::BasicApp app{};

    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}