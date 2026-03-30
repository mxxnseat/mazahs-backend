module;

#include <Eigen/Dense>
#include <vector>

export module core.utils;

import std;

export namespace Core::Utils {
    std::vector<float> hann_window(int N);
    Eigen::MatrixXcf stft(const std::vector<float> &data, int hop_size, int fft_size);
    Eigen::MatrixXf max_filter(const Eigen::MatrixXf &spectrogram, int window_size);
    class RingBuffer {
        public:
            RingBuffer(int capacity);

            void add(double data);
            void add(std::vector<double> data);
            void erase(int start, int end);

            const std::vector<double> get_buffer() const;
        private:
            int capacity;
            std::vector<double> buffer;
    };
    std::vector<std::string> split(const std::string& str, char delimiter);

    struct YtdlpResponse {
        int status;
        std::optional<std::string> file_path;
    };
    YtdlpResponse run_ytdlp(const std::string& url, const std::string& output_dir);
}
