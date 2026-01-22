// Usage is like
// int exit_key = input_readline(v1)
// int exit_key = input_readline(v1, "Enter: ")

#include <iostream>
#include <readline/readline.h>
#include <readline/history.h>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <vector>

#include <var/var.h>

namespace exo {

// Thread-local history storage
thread_local static std::vector<std::string> thread_history;
thread_local static int static_last_exit_key;

class ReadlineWrapper {
private:

	// readline uses globals so we have to lock during every operation
    static std::mutex mtx;

    thread_local static const char* default_text_to_use;

	// Complexity required by readline
    static int startup_hook(void) {
        if (default_text_to_use != nullptr) {
            rl_insert_text(default_text_to_use);
//            rl_point = 0; // Cursor to start
        }
        return 0;
    }

    // Handler for Esc, Ctrl+X: Clear buffer and finish input
    static int no_input_handler(int /*count*/, int key) {
		static_last_exit_key = key;
        rl_replace_line("", 0); // Clear the buffer
//		char keystr[2] {0,0};
//		keystr[0] = char(key);
//        rl_replace_line(keystr, 0); // return the key pressed
		// Cursor to beginning?
		//rl_point = 0;
        rl_done = 1; // Finish readline, return empty
        return 0;
    }

public:
    int last_exit_key;

    ReadlineWrapper() {
        std::lock_guard<std::mutex> lock(mtx);
        rl_initialize();
        rl_startup_hook = startup_hook;
        // Bind multiple keys for "no input"
        rl_bind_key(27, no_input_handler);   // Esc
//        rl_bind_key('\021', no_input_handler); // Ctrl+Q (ASCII 17)
        rl_bind_key('\030', no_input_handler); // Ctrl+X (ASCII 24)
    }

    char* read(const char* prompt, const char* default_text, int& last_exit_key) {
        std::lock_guard<std::mutex> lock(mtx);
        restore_history(default_text); // Load this thread’s history into Readline
        default_text_to_use = default_text;

		static_last_exit_key = 0;
        char* input = readline(prompt);
		last_exit_key = static_last_exit_key;

        if (input && *input  && !is_duplicate(input) && not static_last_exit_key) {
            add_history(input); // Add to history internally
            thread_history.emplace_back(input); // Save to thread-local history
        }
        return input;
    }

    // Restore thread-specific history to Readline
    void restore_history(const char* default_text) {
        clear_history(); // Clear global history
        for (const auto& entry : thread_history) {
			if (strcmp(entry.c_str(), default_text))
	            add_history(entry.c_str()); // Reload thread history
        }
    }

	// Add something to history if not a duplicate
	void add_to_history(const char* entry) {
        std::lock_guard<std::mutex> lock(mtx);
        if (entry && *entry && !is_duplicate(entry)) {
            add_history(entry); // Add to global history (is this necessary?)
            thread_history.emplace_back(entry); // Save to thread-local history
        }
    }

    // Helper to check if entry is a duplicate of the last history item
    static bool is_duplicate(const char* entry) {
        if (!entry || !*entry) return false; // Empty entries aren’t added
        if (history_length == 0) return false; // No history yet
        HIST_ENTRY* last = history_get(history_length); // 1-based index
        return last && last->line && strcmp(last->line, entry) == 0;
    }
};

// Define static members
std::mutex ReadlineWrapper::mtx;
thread_local const char* ReadlineWrapper::default_text_to_use = nullptr;

thread_local ReadlineWrapper rl;

//var input_main(int& last_exit_key, const char* prompt = "", const char* default_text) {
int input_readline(var& v1, const char* prompt = "") {

	std::string default_value = v1.unassigned() ? v1 : "";
//    while (true) {
//        char* input = rl.read("Prompt> ", "my custom text");
		int last_exit_key = 0;
        char* input = rl.read(prompt, default_value.c_str(), last_exit_key);

        if (input == nullptr) { // Ctrl+D or EOF
//            std::cout << std::endl << "Exiting." << std::endl;
			v1 = "";
            return 4;
        }
////        if (*input == '\0') { // Esc, or Ctrl+X pressed
//        if (rl.last_exit_key) { // Esc, or Ctrl+X pressed
////            std::cout << "No input provided." << std::endl;
//        } else { // Enter with content
////            std::cout << "You entered: " << input << std::endl;
//        }

		if (last_exit_key)
			v1 = "";
		else
			v1 = input;

		// Important
        free(input);

		return last_exit_key;

//    }

//    return line;
}

}; // namespace exo
