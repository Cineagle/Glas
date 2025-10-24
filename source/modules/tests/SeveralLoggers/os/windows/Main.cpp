#include <windows.h>

import Glas.Exception;
import Glas.Test.DebugLogger;
import Glas.Test.NotifyLogger;
import Glas.Test.OtherLogger;

import Glas.FileStringOutput;
import Glas.ConsoleStringOutput;
import Glas.DebugStringOutput;

import std;


void testDebugLogger(std::shared_ptr<Glas::Test::DebugLogger> logger) {
    for (int i{}; i < 100; ++i) {
        logger->success("success text 1");
        logger->success({ "success text {}", 2 });

        logger->warning("warning text 1");
        logger->warning({ "warning text {}", 2 });

        logger->error("error text 1");
        logger->error({ "error text {}", 2 });
    }
}

void testNotifyLogger(std::shared_ptr<Glas::Test::NotifyLogger> logger) {
    for (int i{}; i < 100; ++i) {
        logger->info("info text 1");
        logger->info({ "info text {}", 2 });

        logger->success("success text 1");
        logger->success({ "success text {}", 2 });
    }
}

void testOtherLogger(std::shared_ptr<Glas::Test::OtherLogger> logger) {
    for (int i{}; i < 100; ++i) {
        logger->info("info text 1");
        logger->info({ "info text {}", 2 });
    }
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
    try {
        auto debugLogger = Glas::Test::DebugLogger::create("DebugLogger",
            Glas::Queue::Scheme::Unbound, 2);

        auto notifyLogger = Glas::Test::NotifyLogger::create("NotifyLogger",
            Glas::Queue::Scheme::Bound, 256);

        auto otherLogger = Glas::Test::OtherLogger::create("OtherLogger", 
            Glas::Queue::Scheme::Bound, 32);

        auto console = Glas::ConsoleStringOutput::create(
            Glas::ConsoleConfig{
                .consoleTitle{ L"SeveralLoggers" },
                .fontFaceName{ L"Cascadia Mono" },
                .vtEnabled{ true }
            }
        );

        auto file1 = Glas::FileStringOutput::create("SeveralLoggers1.txt",
            Glas::FileStringOutput::PathType::Relative);
        auto file2 = Glas::FileStringOutput::create("SeveralLoggers2.txt",
            Glas::FileStringOutput::PathType::Relative);

        auto debug = Glas::DebugStringOutput::create();

        debugLogger->Success::outputs(file1, console);
        debugLogger->Warning::outputs(file1);
        debugLogger->Error::outputs(file1, file2, console, debug);

        notifyLogger->Info::outputs(file1, console);
        notifyLogger->Success::outputs(file1, debug, file2);

        auto start = std::chrono::high_resolution_clock::now();

        std::vector<std::jthread> threads;
        for (int i{}; i < 10; ++i) {
            auto thread1 = std::jthread(&testDebugLogger, debugLogger);
            auto thread2 = std::jthread(&testNotifyLogger, notifyLogger);
            auto thread3= std::jthread(&testOtherLogger, otherLogger);

            threads.push_back(std::move(thread1));
            threads.push_back(std::move(thread2));
            threads.push_back(std::move(thread3));
        }

        // Test thread-safety when modifying random properties.

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        debugLogger->Success::outputs(file2, debug);
        debugLogger->Warning::outputs();

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        debugLogger->Success::TypeField::VTSequence::enable();
        debugLogger->Success::TypeField::VTSequence::style({
            Glas::VTStyle{
                .fgColor{.red{ 144 }, .green{ 225 }, .blue{ 145 } },
                .bgColor{.red{ 50 }, .green{ 20 }, .blue{ 20 } },
                .effect{ Glas::VTStyle::Effect::Italic }
            }
        });

        debugLogger->Error::outputs(file2, debug);

        debugLogger->SuccessEntry::TimePointField::padding(
            Glas::Padding{ 
                .spaceAfter{ 3 },
                .breakAfter{ 2 },
            }
        );

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        notifyLogger->Info::outputs(debug);
        notifyLogger->Success::outputs(file1, file1, file1, console, console, debug, debug);

        debugLogger->Success::MessageField::disable();
        debugLogger->Success::TypeField::disable();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        debugLogger->WarningEntry::LoggerNameField::disable();
        notifyLogger->Success::TimePointField::disable();

        notifyLogger->InfoEntry::LoggerNameField::padding(
            Glas::Padding{ 
                .breakBefore{ 2 },
                .spaceAfter{ 10 },
            }
        );

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        otherLogger->Info::TimePointField::disable();

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        otherLogger->Info::outputs(debug, file2);

        otherLogger->InfoEntry::MessageField::padding(
            Glas::Padding{ 
                .breakBefore{ 2 },
                .spaceBefore{ 2 },
                .spaceAfter{ 4 },
                .breakAfter{ 3 },
            }
        );

        debugLogger->Success::MessageField::enable();
        debugLogger->Success::TypeField::enable();

        debugLogger->Warning::MessageField::VTSequence::enable();
        debugLogger->Warning::MessageField::VTSequence::style({
            Glas::VTStyle{
                .fgColor{.red{ 144 }, .green{ 144 }, .blue{ 200 } }
            }
        });

        notifyLogger->Success::TypeField::VTSequence::enable();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        notifyLogger->Success::TypeField::VTSequence::style({
            Glas::VTStyle{
                .fgColor{.red{ 200 }, .green{ 77 }, .blue{ 200 } },
                .bgColor{.red{ 20 }, .green{ 20 }, .blue{ 20 } },
            }
        });

        otherLogger->Info::MessageField::VTSequence::enable();
        otherLogger->Info::MessageField::VTSequence::style({
            Glas::VTStyle{
                .fgColor{.red{ 144 }, .green{ 144 }, .blue{ 200 } }
            }
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(400));
        notifyLogger->Success::TypeField::VTSequence::disable();
        debugLogger->Error::TypeField::VTSequence::enable();
        debugLogger->Error::MessageField::VTSequence::enable();

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