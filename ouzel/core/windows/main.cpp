// Copyright (C) 2018 Elviss Strazdins
// This file is part of the Ouzel engine.

#include <Windows.h>
#include "EngineWin.hpp"
#include "utils/Log.hpp"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try
    {
        int argc;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

        std::unique_ptr<ouzel::EngineWin> engine(new ouzel::EngineWin(argc, argv));

        if (argv) LocalFree(argv);

        int result = engine->run();
        engine.reset(); // must release engine instance before exit on Windows

        return result;
    }
    catch (const std::exception& e)
    {
        ouzel::Log(ouzel::Log::Level::ERR) << e.what();
        return EXIT_FAILURE;
    }
    catch (...)
    {
        return EXIT_FAILURE;
    }
}
