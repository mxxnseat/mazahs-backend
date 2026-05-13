module;

#include <exception>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <cerrno>
#include <cstdio>
#include <cstring>

#include <stdexcept>
#include <streambuf>
#include <thread>
#include <atomic>
#include <string>
#include <iostream>
#include <vector>
#include <functional>

export module core.ffmpeg;

import core.concepts;

export namespace Core::FFMPEG {
    class Commander {
    public:
        Commander() {
            start();
        }

        ~Commander() {
            try {
                if (!finished) {
                    finish();
                }
            } catch (std::exception& e) {
                std::cerr << "Error in ffmpeg commander destructor: " << e.what() << std::endl;
            }
        }

        Commander& set_format(const std::string& fmt) {
            format = fmt;
            return *this;
        }

        Commander& set_codec(const std::string& cd) {
            codec = cd;
            return *this;
        }

        Commander& use_experemental() {
            experemental = true;
            return *this;
        }

        Commander& set_channels_number(int number) {
            channels = number;
            return *this;
        }

        Commander& set_samplerate(int sr) {
            samplerate = sr;
            return *this;
        }

        template <Core::Concepts::WritableStream T>
        Commander& pipe(T& stream) {
            output_stream_writer = [&stream](const char* data, std::streamsize n) {
                stream.write(data, n);
            };
            return *this;
        }

        void write(const char* data, std::streamsize n) {
            if (stdin_fd == -1) {
                throw std::runtime_error("ffmpeg stdin is not available");
            }

            const char* ptr = data;
            std::streamsize remaining = n;

            while (remaining > 0) {
                ssize_t written = ::write(stdin_fd, ptr, static_cast<size_t>(remaining));
                if (written < 0) {
                    if (errno == EINTR) {
                        continue;
                    }
                    throw std::runtime_error(std::string("write to ffmpeg failed: ") + std::strerror(errno));
                }

                ptr += written;
                remaining -= written;
            }
        }
    private:
        int finish() {
            if (finished) {
                return exit_code;
            }
            finished = true;

            if (stdin_fd != -1) {
                ::close(stdin_fd);
                stdin_fd = -1;
            }

            if (reader.joinable()) {
                reader.join();
            }

            int status = 0;
            if (pid > 0) {
                while (::waitpid(pid, &status, 0) < 0) {
                    if (errno != EINTR) {
                        throw std::runtime_error(std::string("waitpid failed: ") + std::strerror(errno));
                    }
                }
            }

            if (stdout_fd != -1) {
                ::close(stdout_fd);
                stdout_fd = -1;
            }

            if (WIFEXITED(status)) {
                exit_code = WEXITSTATUS(status);
                return exit_code;
            }

            if (WIFSIGNALED(status)) {
                exit_code = 128 + WTERMSIG(status);
                return exit_code;
            }

            exit_code = 1;
            return exit_code;
        }
        void start() {
            int stdin_pipe[2];
            int stdout_pipe[2];

            if (::pipe(stdin_pipe) == -1) {
                throw std::runtime_error(std::string("pipe stdin failed: ") + std::strerror(errno));
            }

            if (::pipe(stdout_pipe) == -1) {
                ::close(stdin_pipe[0]);
                ::close(stdin_pipe[1]);
                throw std::runtime_error(std::string("pipe stdout failed: ") + std::strerror(errno));
            }

            pid = ::fork();
            if (pid < 0) {
                ::close(stdin_pipe[0]);
                ::close(stdin_pipe[1]);
                ::close(stdout_pipe[0]);
                ::close(stdout_pipe[1]);
                throw std::runtime_error(std::string("fork failed: ") + std::strerror(errno));
            }

            if (pid == 0) {
                ::close(stdin_pipe[1]);
                ::close(stdout_pipe[0]);

                if (::dup2(stdin_pipe[0], STDIN_FILENO) < 0) {
                    std::perror("dup2 stdin failed");
                    _exit(127);
                }

                if (::dup2(stdout_pipe[1], STDOUT_FILENO) < 0) {
                    std::perror("dup2 stdout failed");
                    _exit(127);
                }
                ::close(stdin_pipe[0]);
                ::close(stdout_pipe[1]);

                auto args = build_args();

                execvp(
                    "ffmpeg",
                    args.data()
                );

                std::perror("execlp ffmpeg failed");
                _exit(127);
            }

            ::close(stdin_pipe[0]);
            ::close(stdout_pipe[1]);

            stdin_fd = stdin_pipe[1];
            stdout_fd = stdout_pipe[0];

            reader = std::thread([this]() {
                char buffer[4096];

                while (true) {
                    ssize_t n = ::read(stdout_fd, buffer, sizeof(buffer));
                    if (n == 0) {
                        break;
                    }
                    if (n < 0) {
                        if (errno == EINTR) {
                            continue;
                        }
                        break;
                    }

                    output_stream_writer(buffer, n);
                }
            });

        }

    private:
        std::vector<std::string> args{};
        int samplerate{22050};
        int channels{1};
        std::string codec;
        bool experemental{false};
        std::string format = "wav";
        pid_t pid{-1};
        int stdin_fd{-1};
        int stdout_fd{-1};
        std::thread reader;
        bool finished{false};
        int exit_code{0};


        std::vector<char*> build_args()  {
            args = {
                "ffmpeg",
                "-hide_banner",
                "-loglevel", "quiet",
                "-i", "pipe:0",
                "-vn",
                "-ac", std::to_string(channels),
                "-ar", std::to_string(samplerate),
                "-f", format
            };

            if (!codec.empty()) {
                args.push_back("-c:a");
                args.push_back(codec);
            }

            if (experemental) {
                args.push_back("-strict");
                args.push_back(get_experemental_flag());
            }

            args.push_back("pipe:1");

            std::vector<char*> argv;
            argv.reserve(args.size()+1);

            for (auto& s : args) {
                argv.push_back(s.data());
            }
            argv.push_back((char*)nullptr);
            return argv;
        }

        std::function<void(const char*, std::streamsize)> output_stream_writer;

        std::string get_experemental_flag() const {
            return experemental ? "-2" : "";
        }
    };
}