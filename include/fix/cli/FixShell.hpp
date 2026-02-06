#pragma once 
#include <string_view>


namespace Fix::Cli {

    /*
        Brief and minimal cli interface for keeping the main thread alive with the FIX engine.
        Can be replaced by your application if that runs on the main thread, or can be extended to include more functionality as needed. It is not intended to be a full-featured shell interface, but rather a simple way to keep the engine running and provide some basic interaction.
    */
    struct FixShell {

        FixShell() = default;   
        FixShell(const FixShell&) = delete;
        FixShell& operator=(const FixShell&) = delete;
        FixShell(FixShell&&) = delete;
        FixShell& operator=(FixShell&&) = delete;

        void run();

        private:

        void clear();
        void help();
        void print_to_screen(const std::string_view msg, bool newline = false);
        



    };
}