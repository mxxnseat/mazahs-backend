module;

#include <vector>
#include <string>
#include <Eigen/dense>
#include <thirdparty/iir/iir.h>

export module searcher.songs.dsp;

import core.dsp;

export namespace Searcher::Songs::DSP {
    struct ReadFileOutput {
        std::vector<float> data;
        int samplerate;
    };
    struct ExtractPeaksOutput {
        int frequency;
        int time;
    };
    struct BuildHashOutput {
        int time;
        uint32_t hash;
    };

    class ReadFileNode : public Core::DSP::IDSPNode<std::string, ReadFileOutput>{
        public:
            ReadFileOutput process(std::string path) override;
    };

    class ResampleNode : public Core::DSP::IDSPNode<ReadFileOutput, std::vector<float>>{
        public:
            ResampleNode();

            std::vector<float> process(ReadFileOutput data) override;

        private:
            int target_samplerate;
    };

    class STFTNode : public Core::DSP::IDSPNode<const std::vector<float>&, Eigen::MatrixXcf> {
        public:
            STFTNode();

            Eigen::MatrixXcf process(const std::vector<float>& data) override;

        private:
            int window_length;
            int hop_length;
    };

    class ExtractPeaksNode : public Core::DSP::IDSPNode<Eigen::MatrixXcf&, std::vector<ExtractPeaksOutput>> {
        public:
            std::vector<ExtractPeaksOutput> process(Eigen::MatrixXcf& spectrogram) override;

        private:
            bool is_peak(float spectrogram_magnitude, float local_maxima);
    };

    class BuildHashNode : public Core::DSP::IDSPNode<std::vector<ExtractPeaksOutput>&, std::vector<BuildHashOutput>> {
        public:
            std::vector<BuildHashOutput> process(std::vector<ExtractPeaksOutput>& peaks) override;

        private:
            uint32_t hash(int frequency1, int frequency2, int delta_time);
    };

    // Online only mode
    class PeakNormalizationNode : public Core::DSP::IDSPNode<std::vector<float>, std::vector<float>> {
        public:
            std::vector<float> process(std::vector<float> data) override;

        private:
            float target_peak = 0.9;
    };

    class BandpassFilterNode : public Core::DSP::IDSPNode<std::vector<float>, std::vector<float>> {
        public:
            BandpassFilterNode();
            std::vector<float> process(std::vector<float> data) override;

        private:
            Iir::Butterworth::BandPass<4> filter;
    };

}