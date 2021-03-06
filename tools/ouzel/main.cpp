#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main(int argc, const char* argv[])
{
    enum class Action
    {
        NONE,
        NEW_PROJECT,
        GENERATE
    };

    enum class Project
    {
        ALL,
        VISUAL_STUDIO,
        XCODE,
        MAKEFILE
    };

    enum class Platform
    {
        ALL,
        WINDOWS,
        MACOS,
        LINUX,
        IOS,
        TVOS,
        ANDROID,
        EMSCRIPTEN
    };

    Action action = Action::NONE;
    std::string path;
    std::string name;
    Project project = Project::ALL;
    Platform platform = Platform::ALL;

    for (int i = 1; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--help")
        {
            std::cout << "Usage:" << std::endl;
            std::cout << argv[0] << " [--help] [--new-project <name>] [--location <location>]" << std::endl;
            return EXIT_SUCCESS;
        }
        else if (std::string(argv[i]) == "--new-project")
        {
            if (++i >= argc)
                throw std::runtime_error("Invalid command");

            action = Action::NEW_PROJECT;
            path = std::string(argv[i]);
        }
        else if (std::string(argv[i]) == "--name")
        {
            if (++i >= argc)
                throw std::runtime_error("Invalid command");

            name = std::string(argv[i]);
        }
        else if (std::string(argv[i]) == "--generate")
        {
            action = Action::GENERATE;

            if (++i >= argc)
                throw std::runtime_error("Invalid command");

            path = std::string(argv[i]);
        }
        else if (std::string(argv[i]) == "--project")
        {
            if (std::string(argv[i]) == "all")
                project = Project::ALL;
            else if (std::string(argv[i]) == "visualstudio")
                project = Project::VISUAL_STUDIO;
            else if (std::string(argv[i]) == "xcode")
                project = Project::XCODE;
            else if (std::string(argv[i]) == "makefile")
                project = Project::MAKEFILE;
            else
                throw std::runtime_error("Invalid project");
        }
        else if (std::string(argv[i]) == "--platform")
        {
            if (std::string(argv[i]) == "all")
                platform = Platform::ALL;
            else if (std::string(argv[i]) == "windows")
                platform = Platform::WINDOWS;
            else if (std::string(argv[i]) == "macos")
                platform = Platform::MACOS;
            else if (std::string(argv[i]) == "linux")
                platform = Platform::LINUX;
            else if (std::string(argv[i]) == "ios")
                platform = Platform::IOS;
            else if (std::string(argv[i]) == "tvos")
                platform = Platform::TVOS;
            else if (std::string(argv[i]) == "android")
                platform = Platform::ANDROID;
            else if (std::string(argv[i]) == "emscripten")
                platform = Platform::EMSCRIPTEN;
            else
                throw std::runtime_error("Invalid platform");
        }
    }

    try
    {
        switch (action)
        {
            case Action::NONE:
                throw std::runtime_error("No action selected");
            case Action::NEW_PROJECT:
                break;
            case Action::GENERATE:
                break;
            default:
                throw std::runtime_error("Invalid action selected");
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}
