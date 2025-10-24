#include <windows.h>

import Glas.Exception;
import Glas.Test.CommandLineLogger;
import std;


void testLogger(std::shared_ptr<Glas::Test::CommandLineLogger> logger) {
    logger->info("info text 1");
    logger->info({ "info text {}", 2 });

    logger->warning("warning text 1");
    logger->warning({ "warning text {}", 2 });

    logger->error("error text 1");
    logger->error({ "error text {}", 2 });
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, 
    _In_ int nCmdShow)
{
    try {
        // In the CommandLineLogger class, we call the `createOutputs` method to create a `file`
        // output, for which we pass `lg1` as the ID of our logger. The `File` class, through
        // its constructor that takes this ID, will then parse the command line by calling
        // the WinAPI function `GetCommandLineW`.
        //
        // In the project settings under Debugging/Command Arguments, we use the following
        // line for testing:
        //
        // --lg1.path = "logs/ap p1 23.log" --lg1.pathType = " Relative " --lg1.flushSize = "20" 
        // --lg1.openMode = "out, trunc, binary"
        //
        // See the `File` class for the list of possible command-line argument values.

        auto logger = Glas::Test::CommandLineLogger::create("lg1");
        testLogger(logger);

        int d = 1;
    }
    catch (const Glas::Exception& ex) {
        std::ofstream file{ "exception.txt" };
        file << ex.what();
    }
    return 0;
}