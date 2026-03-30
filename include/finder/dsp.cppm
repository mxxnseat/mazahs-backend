module;


export module core.dsp;

export namespace Core::DSP {
    template<typename Input, typename Output>
    class IDSPNode {
        public:
            virtual Output process(Input data) = 0;
    };
}