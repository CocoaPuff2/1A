// Shell Program
/*
 * Basics:
 * 1) Displaying ‘$’ to indicate that it's ready to accept next command from user
 * 2) Reading a line of keyboard input as a command
 * 3) Spawning and having a new child process execute the user command
 *
 * Process:
 * 1) The shell locates an executable file whose name is specified in the firs string give from a keyboard input
 * 2) It creates a child process by duplicating itself (through fork),
 * 3) The duplicated child shell overloads its process image with the executable file (through execvp),
 * 4) The overloaded process receives an array of *char (all strings after the command), and starts command execution.
 * 5) The shell checks if the command was delimited with ‘;’ or ‘&’. The former makes the shell to wait for the
 *      child process termination, whereas the latter allows the shell to indicate a new prompt and to accept
 *      a next command without waiting for the child process termination.
 *
 * Shell Must:
 * 1) Displaying a prompt to show that it is ready to accept a new line input from the keyboard ($)
 * 2) Reading a keyboard input
 * 3) Repeating the following interpretation till reaching the end of input line:
        o Changing its current working status if a command is built-in, otherwise
        o Spawning a new process and having it execute this external command.
        o Waiting for the process to be terminated if the command is delimited by ';'.
        o Otherwise running the process in background without waiting for its termination.
        o If a command is “fg”, bring back the latest background process to a foreground execution and
            waits for its termination.
        o If a command is “exit”, terminate the shell itself.
 *
 * Example:
 * cssmpi1h$ a.out a b c
 * --> shell duplicates itself and has this duplicated, process execute a.out
 * ---> that receives a, b, and c as its arguments in argv[].
 *
 * cssmpi1h$ cd public_html
 * --> changes the shell's current working directory to public_html, cd is one of the shell built-in commands
 *
 * Delimiters:
 *  ' '  for arguments
 *  ';'  default, make shell wait for child termination command
 *      -->  fork,  execlp, wait
 *      ex: for cssmpi1h$ who & ls & date, shell waits for the completion of date
 *  '&'  move onto next prompt, accept next command
 *   -->  fork,  execlp
 *
 *  Functions:
 *  1. isteam& getline( istream& cin, string& fullLine)
 *  2. pid_t fork( )
 *  3. int execvp( const char *file, char *const argv[] )
 *  4. char *strtok( char *str, const char *delim)
 *  5. int stcmpt( const char* s1, const char* s2)
 *  6. pid_t waitpid( pid_t pid, int *status, 0 )
 */

#include <iostream>
#include <string>
#include <stack>
#include <vector>
#include <sstream>
#include <cstring>   // for strtok, strcmp
#include <sys/wait.h> // waitpid
#include <unistd.h> //  fork, execvp
#include <cstdlib> // exit

using namespace std;

int main() {
    // pid_t = data type to rep PID (Process ID)
    // for background processes, used with fg
    stack<pid_t> background_stack;

    while(true) {
    // a) Show prompt ($), keep shell running (infinite loop)
        cout << "$" << flush; // display immediately
        string input;
        if (!getline(cin, input)) break; // exits gracefully

        if(input.empty()) continue;

        // b) Parse intput into individual words
        char *char_to_string = strdup(input.c_str());
        if (!char_to_string) continue;

        // tokenize input
        char *token = strtok(char_to_string, " ");
        if (!token) {
            free (char_to_string);
            continue;
        }

        vector<char*> args;
        while (token) {
            args.push_back(token);
            token = strtok(nullptr, " ");
        }
        args.push_back(nullptr); // execvp needs null-terminated array


        if (strcmp(args[0], "exit") == 0) {
            free(char_to_string);
            break;
        }

        if (strcmp(args[0], "fg") == 0) { // brings background processes to the foreground
            if (background_stack.empty()) {
                pid_t pid = background_stack.top();
                waitpid(pid, nullptr, 0); // wait for finish
            }
            free(char_to_string);
            continue;
        }

        // d) Check delimiter ';' or '&'
        bool background = false;
        char *last_arg = args[args.size() - 2];
        size_t length = strlen(last_arg);

        if (last_arg[length - 1] == ';') {
            last_arg[length - 1] = '\0'; // remove ';'
        } else if (last_arg[length - 1] == '&') {
            last_arg[length - 1] = '\0'; // remove '&'
            background = true;
        }

        // e) Fork child proecess
        pid_t pid = fork();

        if (pid < 0) {
            cout << "Fork failure" << endl;
        } else if (pid == 0) {
            // Execute the command in child process
            execvp(args[0], args.data());
            exit(1); // if exec fails
        } else {
            // Execute parent process
            if (background) {
                // save the bg PID for fg
                cout << "Currently running PID " << pid << "in back" << endl;
                background_stack.push(pid);
            } else {
                // wait for child process to complete
                waitpid(pid, nullptr, 0);
            }
        }
    }

    cout << "Shell terminated" << endl;
    return 0;

}



