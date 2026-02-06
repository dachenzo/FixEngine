#include <iostream>
#include <fix/cli/FixShell.hpp>



namespace Fix::Cli {


    void FixShell::run() {
        print_to_screen("FIX Shell started. Type 'help' for commands.\n");
        std::string line;
        while (true) {
            print_to_screen("> ");
            if (!std::getline(std::cin, line)) {
                break;
            }
            if (line == "help") {
                help();
            } else if (line == "clear") {
                clear();
            } else if (line == "exit" || line == "quit") {
                break;
            } else {
                print_to_screen("Unknown command. Type 'help' for commands.\n");
            }
        }
    }

    void FixShell::help() {
        print_to_screen("Available commands:\n");
        print_to_screen("  help  - Show this help message\n");
        print_to_screen("  clear - Clear the screen and destroy the main thread which also destroys all active sessions\n");
        print_to_screen("  exit   - Exit the shell\n");
    }

    void FixShell::print_to_screen(const std::string_view message, bool newline) {
        std::cout << message;
        if (newline) {
            std::cout << std::endl;
        }
    }

    void FixShell::clear() {
        std::cout << "\033[2J\033[1;1H"; // ANSI escape code to clear screen and move cursor to top-left, might not work on all platforms
    }
    
    
}