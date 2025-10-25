#include <windows.h>

import Glas.Exception;
import Glas.Test.CustomStringOutputLogger;
import std;


// In this test example, the messages will also be posted to the Windows Event Log.
// To view them, open the Windows Event Viewer (press Win + X and select "Event Viewer").

void testLogger(std::shared_ptr<Glas::Test::CustomStringOutputLogger> logger) {
    logger->info("info text 1");
    logger->info({ "info text {}", 2 });

    logger->error("error text 1");
    logger->error({ "error text {}", 2 });
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, 
    _In_ int nCmdShow) 
{
    try {
        auto logger = Glas::Test::CustomStringOutputLogger::create("CustomStringOutputLogger");
        testLogger(logger);

        int d = 1;
    }
    catch (const Glas::Exception& ex) {
        std::ofstream file{"exception.txt"};
        file << ex.what();
    }
    return 0;
}