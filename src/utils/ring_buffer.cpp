module;

#include <vector>

module core.utils;

Core::Utils::RingBuffer::RingBuffer(int capacity) : 
    capacity(capacity) 
{
    buffer.reserve(capacity);
};

void Core::Utils::RingBuffer::erase(int start, int end) {
    buffer.erase(buffer.begin()+start, buffer.begin()+end);
}

void Core::Utils::RingBuffer::add(double data) {
    add(std::vector<double>{data});
};

void Core::Utils::RingBuffer::add(std::vector<double> data) {
    int new_buffer_size = buffer.size() + data.size();
    if(new_buffer_size <= capacity){
        buffer.insert(buffer.end(), data.begin(), data.end());
    } else {
        int erase_count = new_buffer_size-capacity;
        erase(0, erase_count);
        buffer.insert(buffer.end(), data.begin(), data.end());
    }
};

const std::vector<double> Core::Utils::RingBuffer::get_buffer() const {
    return buffer;
}
