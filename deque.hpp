#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"
#include <cstddef>

namespace sjtu {

template<class T>
class deque {
private:
    static const size_t BLOCK_SIZE = 512;

    struct Block {
        T* data;
        size_t size;
        size_t capacity;
        Block* prev;
        Block* next;

        Block() : data(nullptr), size(0), capacity(BLOCK_SIZE), prev(nullptr), next(nullptr) {
            data = (T*)::operator new(sizeof(T) * capacity);
        }

        ~Block() {
            if (data) {
                for (size_t i = 0; i < size; ++i) {
                    data[i].~T();
                }
                ::operator delete(data);
            }
        }

        void push_back(const T& value) {
            if (size >= capacity) {
                throw runtime_error();
            }
            new (data + size) T(value);
            ++size;
        }

        void push_front(const T& value) {
            if (size >= capacity) {
                throw runtime_error();
            }
            // Move elements manually to avoid memmove on non-trivial types
            for (size_t i = size; i > 0; --i) {
                new (data + i) T(data[i - 1]);
                data[i - 1].~T();
            }
            new (data) T(value);
            ++size;
        }

        void pop_back() {
            if (size == 0) {
                throw runtime_error();
            }
            --size;
            data[size].~T();
        }

        void pop_front() {
            if (size == 0) {
                throw runtime_error();
            }
            data[0].~T();
            // Move elements manually to avoid memmove on non-trivial types
            for (size_t i = 0; i < size - 1; ++i) {
                new (data + i) T(data[i + 1]);
                data[i + 1].~T();
            }
            --size;
        }
    };

    Block* head;
    Block* tail;
    size_t total_size;

    void clear_blocks() {
        Block* curr = head;
        while (curr) {
            Block* next = curr->next;
            delete curr;
            curr = next;
        }
    }

    void copy_from(const deque& other) {
        head = tail = nullptr;
        total_size = 0;

        if (other.head == nullptr) {
            return;
        }

        Block* other_curr = other.head;
        while (other_curr) {
            Block* new_block = new Block();
            for (size_t i = 0; i < other_curr->size; ++i) {
                new_block->push_back(other_curr->data[i]);
            }

            if (!head) {
                head = tail = new_block;
            } else {
                tail->next = new_block;
                new_block->prev = tail;
                tail = new_block;
            }

            other_curr = other_curr->next;
        }

        total_size = other.total_size;
    }

public:
    class const_iterator;
    class iterator {
    friend class deque;
    friend class const_iterator;
    private:
        Block* block;
        size_t index;
        const deque* deq;

    public:
        iterator() : block(nullptr), index(0), deq(nullptr) {}
        iterator(Block* b, size_t i, const deque* d) : block(b), index(i), deq(d) {}

        iterator operator+(const int &n) const {
            iterator result = *this;
            if (n >= 0) {
                for (int i = 0; i < n; ++i) {
                    ++result;
                }
            } else {
                for (int i = 0; i > n; --i) {
                    --result;
                }
            }
            return result;
        }

        iterator operator-(const int &n) const {
            return *this + (-n);
        }

        int operator-(const iterator &rhs) const {
            if (deq != rhs.deq) {
                throw invalid_iterator();
            }

            int count = 0;
            iterator temp = *this;

            if (temp.block == rhs.block) {
                return (int)temp.index - (int)rhs.index;
            }

            // Check which one is ahead
            iterator check = *this;
            while (check.block) {
                if (check.block == rhs.block) {
                    // rhs is behind this
                    temp = *this;
                    while (!(temp.block == rhs.block && temp.index == rhs.index)) {
                        --temp;
                        ++count;
                    }
                    return count;
                }
                if (check.block->next == nullptr) break;
                check.block = check.block->next;
                check.index = 0;
            }

            // this is behind rhs
            temp = *this;
            while (!(temp.block == rhs.block && temp.index == rhs.index)) {
                ++temp;
                --count;
            }
            return count;
        }

        iterator& operator+=(const int &n) {
            *this = *this + n;
            return *this;
        }

        iterator& operator-=(const int &n) {
            *this = *this - n;
            return *this;
        }

        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        iterator& operator++() {
            if (!block) {
                throw invalid_iterator();
            }

            ++index;
            if (index >= block->size) {
                if (block->next) {
                    block = block->next;
                    index = 0;
                } else {
                    // end() iterator
                    index = block->size;
                }
            }
            return *this;
        }

        iterator operator--(int) {
            iterator temp = *this;
            --(*this);
            return temp;
        }

        iterator& operator--() {
            if (!block) {
                throw invalid_iterator();
            }

            if (index > 0) {
                --index;
            } else {
                if (block->prev) {
                    block = block->prev;
                    index = block->size - 1;
                } else {
                    throw invalid_iterator();
                }
            }
            return *this;
        }

        T& operator*() const {
            if (!block || index >= block->size) {
                throw invalid_iterator();
            }
            return block->data[index];
        }

        T* operator->() const noexcept {
            return &(block->data[index]);
        }

        bool operator==(const iterator &rhs) const {
            return deq == rhs.deq && block == rhs.block && index == rhs.index;
        }

        bool operator==(const const_iterator &rhs) const {
            return deq == rhs.deq && block == rhs.block && index == rhs.index;
        }

        bool operator!=(const iterator &rhs) const {
            return !(*this == rhs);
        }

        bool operator!=(const const_iterator &rhs) const {
            return !(*this == rhs);
        }
    };

    class const_iterator {
    friend class deque;
    friend class iterator;
    private:
        Block* block;
        size_t index;
        const deque* deq;

    public:
        const_iterator() : block(nullptr), index(0), deq(nullptr) {}
        const_iterator(Block* b, size_t i, const deque* d) : block(b), index(i), deq(d) {}
        const_iterator(const iterator& it) : block(it.block), index(it.index), deq(it.deq) {}

        const_iterator operator+(const int &n) const {
            const_iterator result = *this;
            if (n >= 0) {
                for (int i = 0; i < n; ++i) {
                    ++result;
                }
            } else {
                for (int i = 0; i > n; --i) {
                    --result;
                }
            }
            return result;
        }

        const_iterator operator-(const int &n) const {
            return *this + (-n);
        }

        int operator-(const const_iterator &rhs) const {
            if (deq != rhs.deq) {
                throw invalid_iterator();
            }

            int count = 0;
            const_iterator temp = *this;

            if (temp.block == rhs.block) {
                return (int)temp.index - (int)rhs.index;
            }

            const_iterator check = *this;
            while (check.block) {
                if (check.block == rhs.block) {
                    temp = *this;
                    while (!(temp.block == rhs.block && temp.index == rhs.index)) {
                        --temp;
                        ++count;
                    }
                    return count;
                }
                if (check.block->next == nullptr) break;
                check.block = check.block->next;
                check.index = 0;
            }

            temp = *this;
            while (!(temp.block == rhs.block && temp.index == rhs.index)) {
                ++temp;
                --count;
            }
            return count;
        }

        const_iterator& operator+=(const int &n) {
            *this = *this + n;
            return *this;
        }

        const_iterator& operator-=(const int &n) {
            *this = *this - n;
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator temp = *this;
            ++(*this);
            return temp;
        }

        const_iterator& operator++() {
            if (!block) {
                throw invalid_iterator();
            }

            ++index;
            if (index >= block->size) {
                if (block->next) {
                    block = block->next;
                    index = 0;
                } else {
                    index = block->size;
                }
            }
            return *this;
        }

        const_iterator operator--(int) {
            const_iterator temp = *this;
            --(*this);
            return temp;
        }

        const_iterator& operator--() {
            if (!block) {
                throw invalid_iterator();
            }

            if (index > 0) {
                --index;
            } else {
                if (block->prev) {
                    block = block->prev;
                    index = block->size - 1;
                } else {
                    throw invalid_iterator();
                }
            }
            return *this;
        }

        const T& operator*() const {
            if (!block || index >= block->size) {
                throw invalid_iterator();
            }
            return block->data[index];
        }

        const T* operator->() const noexcept {
            return &(block->data[index]);
        }

        bool operator==(const iterator &rhs) const {
            return deq == rhs.deq && block == rhs.block && index == rhs.index;
        }

        bool operator==(const const_iterator &rhs) const {
            return deq == rhs.deq && block == rhs.block && index == rhs.index;
        }

        bool operator!=(const iterator &rhs) const {
            return !(*this == rhs);
        }

        bool operator!=(const const_iterator &rhs) const {
            return !(*this == rhs);
        }
    };

    deque() : head(nullptr), tail(nullptr), total_size(0) {}

    deque(const deque &other) : head(nullptr), tail(nullptr), total_size(0) {
        copy_from(other);
    }

    ~deque() {
        clear_blocks();
    }

    deque &operator=(const deque &other) {
        if (this != &other) {
            clear_blocks();
            copy_from(other);
        }
        return *this;
    }

    T& at(const size_t &pos) {
        if (pos >= total_size) {
            throw index_out_of_bound();
        }

        size_t curr_pos = 0;
        Block* curr = head;
        while (curr) {
            if (curr_pos + curr->size > pos) {
                return curr->data[pos - curr_pos];
            }
            curr_pos += curr->size;
            curr = curr->next;
        }

        throw index_out_of_bound();
    }

    const T& at(const size_t &pos) const {
        if (pos >= total_size) {
            throw index_out_of_bound();
        }

        size_t curr_pos = 0;
        Block* curr = head;
        while (curr) {
            if (curr_pos + curr->size > pos) {
                return curr->data[pos - curr_pos];
            }
            curr_pos += curr->size;
            curr = curr->next;
        }

        throw index_out_of_bound();
    }

    T& operator[](const size_t &pos) {
        return at(pos);
    }

    const T& operator[](const size_t &pos) const {
        return at(pos);
    }

    const T& front() const {
        if (total_size == 0) {
            throw container_is_empty();
        }
        return head->data[0];
    }

    const T& back() const {
        if (total_size == 0) {
            throw container_is_empty();
        }
        return tail->data[tail->size - 1];
    }

    iterator begin() {
        if (!head) {
            return iterator(nullptr, 0, this);
        }
        return iterator(head, 0, this);
    }

    const_iterator cbegin() const {
        if (!head) {
            return const_iterator(nullptr, 0, this);
        }
        return const_iterator(head, 0, this);
    }

    iterator end() {
        if (!tail) {
            return iterator(nullptr, 0, this);
        }
        return iterator(tail, tail->size, this);
    }

    const_iterator cend() const {
        if (!tail) {
            return const_iterator(nullptr, 0, this);
        }
        return const_iterator(tail, tail->size, this);
    }

    bool empty() const {
        return total_size == 0;
    }

    size_t size() const {
        return total_size;
    }

    void clear() {
        clear_blocks();
        head = tail = nullptr;
        total_size = 0;
    }

    iterator insert(iterator pos, const T &value) {
        if (pos.deq != this) {
            throw invalid_iterator();
        }

        if (pos == end()) {
            push_back(value);
            return iterator(tail, tail->size - 1, this);
        }

        if (pos == begin()) {
            push_front(value);
            return begin();
        }

        // General case: insert into the block
        Block* target_block = pos.block;
        size_t target_index = pos.index;

        if (target_block->size < target_block->capacity) {
            // Room in the block - move elements manually
            for (size_t i = target_block->size; i > target_index; --i) {
                new (target_block->data + i) T(target_block->data[i - 1]);
                target_block->data[i - 1].~T();
            }
            new (target_block->data + target_index) T(value);
            ++target_block->size;
            ++total_size;
            return iterator(target_block, target_index, this);
        }

        // Block is full, need to split
        Block* new_block = new Block();
        size_t split_point = target_block->size / 2;

        for (size_t i = split_point; i < target_block->size; ++i) {
            new_block->push_back(target_block->data[i]);
            target_block->data[i].~T();
        }
        target_block->size = split_point;

        new_block->next = target_block->next;
        new_block->prev = target_block;
        if (target_block->next) {
            target_block->next->prev = new_block;
        } else {
            tail = new_block;
        }
        target_block->next = new_block;

        if (target_index <= split_point) {
            for (size_t i = target_block->size; i > target_index; --i) {
                new (target_block->data + i) T(target_block->data[i - 1]);
                target_block->data[i - 1].~T();
            }
            new (target_block->data + target_index) T(value);
            ++target_block->size;
            ++total_size;
            return iterator(target_block, target_index, this);
        } else {
            size_t new_index = target_index - split_point;
            for (size_t i = new_block->size; i > new_index; --i) {
                new (new_block->data + i) T(new_block->data[i - 1]);
                new_block->data[i - 1].~T();
            }
            new (new_block->data + new_index) T(value);
            ++new_block->size;
            ++total_size;
            return iterator(new_block, new_index, this);
        }
    }

    iterator erase(iterator pos) {
        if (pos.deq != this || pos == end()) {
            throw invalid_iterator();
        }

        Block* target_block = pos.block;
        size_t target_index = pos.index;

        target_block->data[target_index].~T();
        // Move elements manually
        for (size_t i = target_index; i < target_block->size - 1; ++i) {
            new (target_block->data + i) T(target_block->data[i + 1]);
            target_block->data[i + 1].~T();
        }
        --target_block->size;
        --total_size;

        if (target_block->size == 0) {
            Block* next_block = target_block->next;
            Block* prev_block = target_block->prev;

            if (prev_block) {
                prev_block->next = next_block;
            } else {
                head = next_block;
            }

            if (next_block) {
                next_block->prev = prev_block;
            } else {
                tail = prev_block;
            }

            Block* return_block = next_block ? next_block : prev_block;
            size_t return_index = next_block ? 0 : (prev_block ? prev_block->size : 0);

            delete target_block;

            if (return_block == nullptr) {
                return end();
            }
            return iterator(return_block, return_index, this);
        }

        if (target_index >= target_block->size) {
            if (target_block->next) {
                return iterator(target_block->next, 0, this);
            } else {
                return iterator(target_block, target_block->size, this);
            }
        }

        return iterator(target_block, target_index, this);
    }

    void push_back(const T &value) {
        if (!tail || tail->size >= tail->capacity) {
            Block* new_block = new Block();
            new_block->push_back(value);

            if (!head) {
                head = tail = new_block;
            } else {
                tail->next = new_block;
                new_block->prev = tail;
                tail = new_block;
            }
        } else {
            tail->push_back(value);
        }
        ++total_size;
    }

    void pop_back() {
        if (total_size == 0) {
            throw container_is_empty();
        }

        tail->pop_back();
        --total_size;

        if (tail->size == 0) {
            Block* prev = tail->prev;
            delete tail;
            tail = prev;
            if (tail) {
                tail->next = nullptr;
            } else {
                head = nullptr;
            }
        }
    }

    void push_front(const T &value) {
        if (!head || head->size >= head->capacity) {
            Block* new_block = new Block();
            new_block->push_back(value);

            if (!head) {
                head = tail = new_block;
            } else {
                new_block->next = head;
                head->prev = new_block;
                head = new_block;
            }
        } else {
            head->push_front(value);
        }
        ++total_size;
    }

    void pop_front() {
        if (total_size == 0) {
            throw container_is_empty();
        }

        head->pop_front();
        --total_size;

        if (head->size == 0) {
            Block* next = head->next;
            delete head;
            head = next;
            if (head) {
                head->prev = nullptr;
            } else {
                tail = nullptr;
            }
        }
    }
};

}

#endif
