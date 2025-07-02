/* Copyright (C) 2025 Nicolas Dittlo <bluelab.plugins@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this software; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <string.h>

#include <vector>
using namespace std;

template <typename T>
class CircularBuffer
{
public:
    CircularBuffer()
    {    
        _readIdx = 0;
        _writeIdx = 0;
    }
    
    virtual ~CircularBuffer() {}

    void setCapacity(int size)
    {
        _data.resize(size);
        memset(_data.data(), 0, size*sizeof(T));

        _readIdx = 0;
        _writeIdx = 0;
    }
    
    int getSize() const
    {
        int size = _writeIdx - _readIdx;
        if (size < 0)
            size += _data.size();
        return size;
    }

    void push(const T *data, int size)
    {
        if (_writeIdx + size < _data.size())
            memcpy(&_data.data()[_writeIdx], data, size*sizeof(T));
        else
        {
            memcpy(&_data.data()[_writeIdx], data,
                   (_data.size() - _writeIdx)*sizeof(T));
            memcpy(_data.data(),
                   &data[_data.size() - _writeIdx],
                   (size - (_data.size() - _writeIdx))*sizeof(T));
        }
        _writeIdx += size;
        _writeIdx = _writeIdx % _data.size();
        if (_writeIdx < 0)
            _writeIdx += _data.size();
    }

    void peek(T *data, int size)
    {
        if (_readIdx + size < _data.size())
            memcpy(data, &_data.data()[_readIdx], size*sizeof(T));
        else
        {
            memcpy(data, &_data.data()[_readIdx],
                   (_data.size() - _readIdx)*sizeof(T));
            memcpy(&data[_data.size() - _readIdx],
                   _data.data(),
                   (size - (_data.size() - _readIdx))*sizeof(T));
        }
    }

    T peek(int index)
    {
        int idx = (_readIdx + index) %  _data.size();
        if (idx < 0)
            idx += _data.size();
        return _data[idx];
    }
    
    void pop(int size)
    {
        _readIdx += size;
        _readIdx = _readIdx % _data.size();
        if (_readIdx < 0)
            _readIdx += _data.size();
    }

    void poke(T *data, int size)
    {
        if (_readIdx + size < _data.size())
            memcpy(&_data.data()[_readIdx], data, size*sizeof(T));
        else
        {
            memcpy(&_data.data()[_readIdx], data,
                   (_data.size() - _readIdx)*sizeof(T));
            memcpy(_data.data(),
                   &data[_data.size() - _readIdx],
                   (size - (_data.size() - _readIdx))*sizeof(T));
        }
    }

    void poke(int index, T value)
    {
        int idx = (_readIdx + index) %  _data.size();
        if (idx < 0)
            idx += _data.size();
        _data[idx] = value;
    }
    
protected:
    vector<T> _data;
    int _readIdx;
    int _writeIdx;
};

#endif
