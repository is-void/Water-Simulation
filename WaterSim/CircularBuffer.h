#include <vector>

template<typename T>
class CircularBuffer {
public:
    std::vector<T> buffer;
    explicit CircularBuffer(size_t capacity)
        : buffer(capacity), capacity(capacity) {
    }

    void push(const T& value) {
        buffer[head] = value;
        head = (head + 1) % capacity;

        if (size < capacity)
            ++size;
    }

    size_t getSize() const { return size; }

private:
    size_t head = 0;
    size_t size = 0;
    size_t capacity;
};
