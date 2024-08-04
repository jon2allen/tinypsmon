#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <array>
#include <chrono>

class ShellScriptExecutor {
public:
    ShellScriptExecutor(const std::string& scriptPath, const std::vector<std::string>& args, int time_throttle)
        : scriptPath(scriptPath), args(args), time_throttle(time_throttle), time_last_executed(0) {
        validateInputs();
    }

    std::string execute() {
        auto now = std::chrono::system_clock::now().time_since_epoch();
        auto now_seconds = std::chrono::duration_cast<std::chrono::seconds>(now).count();

        if (time_last_executed == 0 || now_seconds >= time_last_executed + time_throttle) {
            time_last_executed = now_seconds;
            return executeScript();
        } else {
            return "Throttle time not reached. Script not executed.";
        }
    }

private:
    std::string scriptPath;
    std::vector<std::string> args;
    int time_throttle;
    long long time_last_executed;

    void validateInputs() const {
        if (scriptPath.empty()) {
            throw std::invalid_argument("Script path cannot be empty");
        }
        // Add more validation as needed
    }

    std::string executeScript() const {
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            throw std::runtime_error("pipe() failed");
        }

        pid_t pid = fork();
        if (pid == -1) {
            throw std::runtime_error("fork() failed");
        }

        if (pid == 0) { // Child process
            close(pipefd[0]); // Close read end
            dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe
            close(pipefd[1]);

            std::vector<char*> execArgs;
            execArgs.push_back(const_cast<char*>(scriptPath.c_str()));
            for (const auto& arg : args) {
                execArgs.push_back(const_cast<char*>(arg.c_str()));
            }
            execArgs.push_back(nullptr);

            execvp(scriptPath.c_str(), execArgs.data());
            _exit(EXIT_FAILURE); // execvp failed
        } else { // Parent process
            close(pipefd[1]); // Close write end

            std::array<char, 128> buffer;
            std::string result;
            ssize_t count;
            while ((count = read(pipefd[0], buffer.data(), buffer.size())) > 0) {
                result.append(buffer.data(), count);
            }
            close(pipefd[0]);

            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                throw std::runtime_error("Script execution failed with status " + std::to_string(WEXITSTATUS(status)));
            }

            return result;
        }
    }
};

