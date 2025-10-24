#include <windows.h>

import Glas.Exception;
import Glas.Test.MiniLogger;
import std;


void testLogger(std::shared_ptr<Glas::Test::MiniLogger> logger) {
    logger->info("info text 1");
    logger->info({ "info text {}", 2 });

    logger->warning("warning text 1");
    logger->warning({ "warning text {}", 2 });

    logger->error("error text 1");
    logger->error({ "error text {}", 2 });
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
    try {
        auto logger = Glas::Test::MiniLogger::create("MiniLogger");
        testLogger(logger);

        int d = 1;
    }
    catch (const Glas::Exception& ex) {
        std::ofstream file{ "exception.txt" };
        file << ex.what();
    }
    return 0;
}