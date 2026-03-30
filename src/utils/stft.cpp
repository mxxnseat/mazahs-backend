module;

#include <vector>
#include <fftw3.h>
#include <complex>
#include <Eigen/Dense>

module core.utils;

int get_stft_frames(int data_size, int hop_size, int fft_size) noexcept {
    if (data_size < fft_size) {
        return 1;
    }
    return 1 + int(std::ceil(float(data_size - fft_size) / hop_size));
}

Eigen::MatrixXcf Core::Utils::stft(const std::vector<float> &data, int hop_size, int fft_size) {
    std::vector<float> window = hann_window(fft_size);

    int bins = fft_size / 2 + 1;
    int num_frames = get_stft_frames(data.size(), hop_size, fft_size);
    Eigen::MatrixXcf spectrogram(num_frames,bins);

    float *in = (float *)fftwf_malloc(sizeof(float) * fft_size);
    fftwf_complex *out = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * bins);
    fftwf_plan plan = fftwf_plan_dft_r2c_1d(fft_size, in, out, FFTW_ESTIMATE);

    for (int frame = 0; frame < num_frames; frame++) {
        const int start = frame * hop_size;

        for (int n = 0; n < fft_size; n++) {
            const std::size_t idx = start + n;
            const float sample = (idx < data.size()) ? data[idx] : 0.0;
            in[n] = sample * window[n];
        }

        fftwf_execute(plan);

        for (int bin = 0; bin < bins; bin++) {
            spectrogram(frame, bin) = std::complex<float>(out[bin][0], out[bin][1]);
        }
    }

    fftwf_destroy_plan(plan);
    fftwf_free(in);
    fftwf_free(out);

    return spectrogram;
}
