module;

#include <vector>
#include <cmath>

module core.utils;

std::vector<float> Core::Utils::hann_window(int N) {
    std::vector<float> result(N);
    if(N == 1){
        result[0] = 1.0;
        return result;
    }
    for (int i = 0; i < N; i++) {
        result[i] = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / (N - 1)));
    }
    return result;
}
