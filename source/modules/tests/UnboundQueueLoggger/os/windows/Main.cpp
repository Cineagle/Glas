#include <windows.h>

import Glas.Exception;
import Glas.Test.UnboundQueueLogger;
import std;


void testLogger(std::shared_ptr<Glas::Test::UnboundQueueLogger> logger) {
    logger->info("info text 1");
    logger->info({ "info text {}", 2 });

    logger->warning("warning text 1");
    logger->warning({ "warning text {}", 2 });

    logger->error("error text 1");
    logger->error({ "error text {}", 2 });
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, 
    _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) 
{
    try {
        auto logger = Glas::Test::UnboundQueueLogger::create("UnboundQueueLogger",
            Glas::Test::UnboundQueueLogger::Scheme::Unbound, 32);

        auto start = std::chrono::high_resolution_clock::now();

        std::vector<std::jthread> threads;
        for (int i{}; i < 100; ++i) {
            auto thread = std::jthread(&testLogger, logger);
            threads.push_back(std::move(thread));
        }

        for (auto& thread : threads) {
            thread.join();
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;

        int d = 1;
    }
    catch (const Glas::Exception& ex) {
        std::ofstream file{"exception.txt"};
        file << ex.what();
    }
    return 0;
}