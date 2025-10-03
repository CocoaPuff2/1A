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

        // special check for ps command to remove extra processes
        if (strcmp(token[0], "ps") == 0) {
            pid_t ps_pid = fork();
            if (ps_pid < 0) {
                continue;
            } else if (ps_pid == 0) {
                // Child process: run ps in two sections, filtering unwanted processes
                char* ps_command = (char*)"bash";
                char* ps_args[] = {
                        (char*)"bash",
                        (char*)"-c",
                        (char*)"(ps -u css430 -o pid,tty | grep -v sftp-server | grep -v sshd; "
                               "ps -u css430 -o time,cmd | grep -v sftp-server | grep -v sshd)",
                        NULL
                };
                execvp(ps_command, ps_args);
                exit(1);
            } else {
                // Parent waits if foreground
                if (!background) {
                    waitpid(ps_pid, &status, 0);
                } else {
                    pid_stack.push(ps_pid);
                }
            }
            continue; // skip normal execvp
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
// bg: push PID to stack, show $
// pid_stack.push(pid);
// printf("$ ");
// fflush(stdout);

// fg: wait for child to finish
// waitpid(pid, &status, 0);