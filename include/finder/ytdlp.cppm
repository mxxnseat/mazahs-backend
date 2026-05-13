module;

#include <cstdint>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <stdio.h>
#include <iostream>
#include <fstream>


export module core.ytdlp;

import core.concepts;

export namespace Core::Ytdlp {
    class Commander {
        public:
            Commander() {}

            Commander& set_url(std::string u) {
                url = u;
                return *this;
            }
            
            template <Core::Concepts::WritableStream T> 
            int pipe(T&& stream){
                return execute(stream);
            }
            
            int execute(auto& stream) {
                int pipefd[2];

                if(::pipe(pipefd) == -1){
                    return 1;
                }

                pid_t pid = fork();
                if (pid < 0) {
                    close(pipefd[0]);
                    close(pipefd[1]);
                    return 1;
                }

                if (pid == 0) {
                    close(pipefd[0]);
                    
                    if (dup2(pipefd[1], STDOUT_FILENO) < 0) {
                        perror("dup2 stdout failed");
                        _exit(127);
                    }

                    close(pipefd[1]);

                    execlp(
                        "yt-dlp",
                        "yt-dlp",
                        "--quiet",
                        "--no-progress",
                        "-f",
                        "bestaudio",
                        "-o", "-",
                        url.c_str(),
                        (char*)nullptr
                    );
                    
                    perror("execlp failed");
                    _exit(127);
                }

                close(pipefd[1]);
                char buffer[4096];
                while(true){
                    ssize_t n = read(pipefd[0], buffer, sizeof(buffer));

                    if(n == 0){
                        break;
                    }
                    if(n == -1) {
                        perror("read failed");
                        close(pipefd[0]);
                        return 1;
                    }

                    stream.write(buffer, n);
                }

                close(pipefd[0]);

                return 0;
            }
        private:
            std::string url;
    };
}