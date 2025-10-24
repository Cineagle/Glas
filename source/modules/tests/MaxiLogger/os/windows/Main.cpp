#include <windows.h>

import Glas.Exception;
import Glas.Test.MaxiLogger;
import std;


void testLogger(std::shared_ptr<Glas::Test::MaxiLogger> logger) {
    logger->info("info text 1");
    logger->success("success text 1");
    logger->warning("warning text 1");
    logger->error("error text 1");

    logger->lines(2);

    logger->info({ "info text {}", 2 });
    logger->success({ "success text {}", 2 });
    logger->warning({ "warning text {}", 2 });
    logger->error({ "error text {}", 2 });

    logger->lines(2);

    logger->sequence(2, 'A');
    logger->sequence(5, "Repeated ");
    logger->sequence(3, { "Repeated {} times ", 3});

    logger->trace();
    logger->trace("trace with message\n");
    logger->trace({"trace with {} message\n", "formated"});

    const std::size_t value{ 123456789 };

    logger->bytes(&value, sizeof(value));
    logger->bytes("bytes with message\n", &value, sizeof(value));
    logger->bytes({ "bytes with {} message\n", "formated" }, &value, sizeof(value));
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, 
    _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) 
{
    try {
        auto logger = Glas::Test::MaxiLogger::create("MaxiLogger");
        testLogger(logger);

        int d = 1;
    }
    catch (const Glas::Exception& ex) {
        std::ofstream file{"exception.txt"};
        file << ex.what();
    }
    return 0;
}