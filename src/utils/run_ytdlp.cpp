module;

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <stdio.h>

module core.utils;

import std;
import core.config;

std::string form_output(const std::string& output_dir) {
    return output_dir + "/" + "%(title)s.%(ext)s";
}

std::string trim_newlines(std::string s) {
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) {
        s.pop_back();
    }
    return s;
}

Core::Utils::YtdlpResponse Core::Utils::run_ytdlp(const std::string& url, const std::string& output_dir){
    // yt-dlp -o "%(title)s.%(ext)s" -x --audio-format wav --postprocessor-args "-ar 21050" https://music.youtube.com/watch\?v\=sh6BbQlpKl4\&si\=dEah1qnqgQuQZdpN --restrict-filenames
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        throw std::runtime_error("pipe failed: " + std::string(std::strerror(errno)));
    }
    YtdlpResponse result;
    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        throw std::runtime_error("fork failed: " + std::string(std::strerror(errno)));
    }

    if (pid == 0) {
        close(pipefd[0]);
        
        if (dup2(pipefd[1], STDOUT_FILENO) < 0) {
            perror("dup2 stdout failed");
            _exit(127);
        }

        close(pipefd[1]);


        std::string output = form_output(output_dir);


        std::vector<std::string> args_str = {
            "yt-dlp",
            "--print",
            "after_move:filepath",
            "-o",
            output.c_str(),
            "-x",
            "--audio-format",
            "wav",
            "--postprocessor-args",
            std::format("-ar {}", Core::Config::WORKING_SAMPLERATE()),
            "--restrict-filenames",
            url.c_str()
        };
        
        std::vector<char*> args;
        args.reserve(args_str.size());

        for (auto& arg : args_str) {
            args.push_back(arg.data());
        }
        args.push_back(nullptr);

        execvp("yt-dlp", args.data());

        perror("execvp failed");
        _exit(127);
    }

    close(pipefd[1]);

    std::string stdout_data;
    char buffer[4096];
    ssize_t n = 0;

    while ((n = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
        stdout_data.append(buffer, static_cast<size_t>(n));
    }

    close(pipefd[0]);

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        throw std::runtime_error("waitpid failed");
    }
    result.status = status;
    result.file_path = trim_newlines(stdout_data);
    return result;
}