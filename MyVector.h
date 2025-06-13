#ifndef VECTOR_H
#define VECTOR_H
#include <iostream>
#include <vector>

using namespace std;
template <typename T>
class MyVector
{
private:
    T* data;
    size_t vec_size;
    size_t capacity;

    void resize()
    {
        capacity *= 2;
        T* newData = new T[capacity];
        for (size_t i = 0; i < vec_size; ++i)
        {
            newData[i] = data[i];
        }
        delete[] data;
        data = newData;
    }

public:
    MyVector()
    {
        vec_size = 0;
        capacity = 1;
        data = new T[capacity];
    }

    MyVector(size_t initialSize)
    {
        vec_size = initialSize;
        capacity = initialSize > 0 ? initialSize : 1;
        data = new T[capacity];
    }

    MyVector(const MyVector& other)
    {
        vec_size = other.vec_size;
        capacity = other.capacity;
        data = new T[capacity];
        for (size_t i = 0; i < vec_size; ++i)
        {
            data[i] = other.data[i];
        }
    }

    ~MyVector()
    {
        delete[] data;
    }

    MyVector& operator=(const MyVector& other)
    {
        if (this != &other)
        {
            delete[] data;
            vec_size = other.vec_size;
            capacity = other.capacity;
            data = new T[capacity];
            for (size_t i = 0; i < vec_size; ++i)
            {
                data[i] = other.data[i];
            }
        }
        return *this;
    }

    void push_back(const T& value)
    {
        if (vec_size >= capacity)
        {
            resize();
        }
        data[vec_size++] = value;
    }

    void clear()
    {
        vec_size = 0;
    }

    size_t size() const
    {
        return vec_size;
    }

    bool empty() const
    {
        return vec_size == 0;
    }

    void erase(size_t index)
    {
        if (index >= vec_size)
        {
            return;
        }
        for (size_t i = index; i < vec_size - 1; ++i)
        {
            data[i] = data[i + 1];
        }
        vec_size--;
    }

    T& operator[](size_t index)
    {
        if (index >= vec_size)
        {
            static T dummy{};
            return dummy;
        }
        return data[index];
    }

    const T& operator[](size_t index) const
    {
        if (index >= vec_size)
        {
            static T dummy{};
            return dummy;
        }
        return data[index];
    }

    T* begin()
    {
        return data;
    }

    const T* begin() const
    {
        return data;
    }

    T* end()
    {
        return data + vec_size;
    }

    const T* end() const
    {
        return data + vec_size;
    }
};

#endif