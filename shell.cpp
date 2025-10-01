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
#include <cstring>   // for strtok, strcmp
#include <sys/wait.h> // waitpid
#include <unistd.h> //  fork, execvp
#include <cstdlib> // exit

using namespace std;

int main() {
    pid_t pid = 0;           // child process id to be returned from
    stack<pid_t> pid_stack;  // pid stack of children in the background
    string fullLine;         // stores full line from the keyboard
    int status;              // get termination status from child
    char* token[100]; // tokenize the fullLine
    bool background = false; // true if command is &

    while (true) {
        printf("$ ");
        fflush(stdout);
        getline(cin, fullLine);
        if (fullLine.empty()) continue;

        // Handle Delimiters
        background = false;
        char endChar = fullLine.back(); // to get & or ;
        // if the last character is '&' or ';' --> then replace with space ' '
        if (endChar == '&') {
            background = true;  // set background as true if last char is &
            fullLine.back() = ' ';
        } else if (endChar == ';') {
            fullLine.back() = ' ';
        }

        // Tokenize Input Line
        const char* delimiter = " ";
        token[0] = strtok((char*)fullLine.c_str(), delimiter);
        int i = 0;
        while (token[i] != NULL) {
            i++;
            token[i] = strtok(NULL, delimiter);
        }
        if (token[0] == NULL) continue; // if there's no command entered

        // Commands
        if (strcmp(token[0], "exit") == 0) {
            break; // exit the shell
        }

        //wait for pid on stack top then pop it, fork a child shell, child replace it with
        // token[0] to now be the entire token. If !background, parent waits for child,
        // else put child pid back into pid_stack)
        if (strcmp(token[0], "fg") == 0) {
            if (!pid_stack.empty()) {
                pid_t bg_pid = pid_stack.top();
                pid_stack.pop();
                // wait for pid on stack top then pop it
                waitpid(bg_pid, &status, 0);
            } else {
                printf("No background processes");
            }
            continue;
        }

        pid = fork();
        if (pid < 0) {
            perror("The fork failed");
            continue;
        }

        if (pid == 0) {
            // child process
            if (execvp(token[0], token) < 0) {
                perror("The execvpt failed");
                exit(1);
            }
        } else {
            // wait for child process to finish
            waitpid(pid, &status, 0);
        }
    }
    return 0;
}
/*
int main() {
    // pid_t --> data type to rep PID (Process ID)
    // for background processes, used with fg
    stack<pid_t> background_stack;

    while(true) {
    // a) Show prompt ($), keep shell running (infinite loop)
        cout << "$" << flush; // display immediately
        string input;
        if (!getline(cin, input)) break; // exit the shell

        if(input.empty()) continue;

        // b) Parse input into individual words
        char *c_type_string = strdup(input.c_str());
        if (!c_type_string) continue;

        // tokenize input
        char *token = strtok(c_type_string, " ");
        if (!token) {
            free(c_type_string);
            continue;
        }

        vector<char*> args;
        while (token) {
            args.push_back(token);
            token = strtok(nullptr, " ");
        }
        args.push_back(nullptr); // execvp needs null-terminated array

        // exit command
        if (strcmp(args[0], "exit") == 0) {
            free(c_type_string);
            break;
        }

        // fg command
        if (strcmp(args[0], "fg") == 0) { // brings background processes to the foreground
            if (!background_stack.empty()) {
                pid_t pid = background_stack.top();
                background_stack.pop();
                waitpid(pid, nullptr, 0); // wait for finish
            }
            free(c_type_string);
            continue;
        }

        // d) Check delimiter ';' or '&'
        bool background = false;
        char *last_arg = args[args.size() - 2];
        size_t length = strlen(last_arg);

        if (last_arg[length - 1] == ';') {
            last_arg[length - 1] = '\0'; // remove the ';'
        } else if (last_arg[length - 1] == '&') {
            last_arg[length - 1] = '\0'; // remove the '&'
            background = true; // & means background processes
        }

        // e) Fork child proecess
        pid_t pid = fork();

        if (pid == 0) {
            // Execute the command in child process
            execvp(args[0], args.data());
            cerr << "execvp failed: " << endl;
            exit(1); // if exec fails
        } else if (pid > 0) {
            // Execute parent process
            if (background) {
                background_stack.push(pid);
            }  else {
                // wait for child process to complete
                waitpid(pid, nullptr, 0);
            }
        } else {
            // Fork Failure
            cerr << "fork failed: "  << endl;
        }

        free(c_type_string); // release mem allocated with strdup().
    }
    return 0;
}

*/

