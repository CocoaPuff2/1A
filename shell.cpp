// Shell Program
/*
 *
 * Command Input Process:
 * 1) The shell locates an executable file whose name is specified in the firs string give from a keyboard input
 * 2) It creates a child process by duplicating itself (through fork),
 * 3) The duplicated child shell overloads its process image with the executable file (through execvp),
 * 4) The overloaded process receives an array of *char (all strings after the command), and starts command execution.
 * 5) The shell checks if the command was delimited with ‘;’ or ‘&’. The former makes the shell to wait for the
 *      child process termination, whereas the latter allows the shell to indicate a new prompt and to accept
 *      a next command without waiting for the child process termination.
 *
 */
#include <iostream>
#include <string>
#include <stack>
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


        // wait for pid on stack top then pop it, fork a child shell, child replace it with
        // token[0] to now be the entire token. If !background, parent waits for child,
        // else put child pid back into pid_stack)
        if (strcmp(token[0], "fg") == 0) {
            if (!pid_stack.empty()) {
                pid_t bg_pid = pid_stack.top();
                pid_stack.pop();
                // wait for pid on stack top then pop it
                // TODO: uncomment the bottom line if new code doesn't work
                // waitpid(bg_pid, &status, 0);
                // TODO: delete bottom code if it doesn't work
                pid = fork();
                if (pid < 0) {
                    perror("fork failed");
                } else if (pid == 0) {
                    // Child just exits immediately after wait (optional)
                    exit(0);
                } else {
                    // Parent waits for the background process
                    waitpid(bg_pid, &status, 0);
                    waitpid(pid, &status, 0); // wait for dummy child if needed
                }
                // todo: delete in bw

            }
            continue;
        }

        pid = fork();
        if (pid < 0) {
            continue;
        } else if (pid == 0) {
            // child process
            if (execvp(token[0], token) < 0) {
                exit(1);
            }
        } else if (pid > 0) {
            // the parent process
            if (!background) {
                waitpid(pid, &status, 0);
            } else {
                pid_stack.push(pid);
            }
        }
    }
    return 0;
}
