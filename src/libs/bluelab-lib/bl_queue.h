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

#ifndef NL_QUEUE_H
#define NL_QUEUE_H

#include <vector>
#include <deque>
using namespace std;

// Implementation of queue with fixed size, and circular buffer
// to get optimal memory (avoid memory allocation/deallocation)
template<class T> class bl_queue
{
public:
    bl_queue(int size) { mFixedSize = true; mData.resize(size); mCursor = 0; }
    bl_queue() { mFixedSize = false; mCursor = 0; }
    bl_queue(const bl_queue &other)
    {
        mData = other.mData;
        mCursor = other.mCursor;
        mFixedSize = other.mFixedSize;
        mNonFixedSizeData = other.mNonFixedSizeData;
    }
    
    ~bl_queue() { mData.clear(); mNonFixedSizeData.clear(); }

    bool empty() const
    {
        if (mFixedSize)
            return mData.empty();
        else
            return mNonFixedSizeData.empty();
    }
    
    void clear(const T &value)
    {
        if (mFixedSize)
        {
            for (int i = 0; i < mData.size(); i++)
                mData[i] = value;
            mCursor = 0;
        }
        else
        {
            mNonFixedSizeData.clear();
        }
    }

    void clear()
    {
        if (!mFixedSize)
            mNonFixedSizeData.clear();
    }
    
    long size() const
    {
        if (mFixedSize)
            return mData.size();
        else
            return mNonFixedSizeData.size();
    };
    
    // After this, the cursor is not well set
    // Need to call set_all() maybe, and restart
    // from the beginning in the caller.
    void resize(int size)
    {
        mNonFixedSizeData.clear();

        mFixedSize = true;
        
        mData.resize(size);
        mCursor = 0;
    }

    void freeze() { set_fixed_size(true); }
    void unfreeze() { set_fixed_size(false); }
    
    // For fixed size
    //
    
    // Push a new value and pop at the same time
    // so the queue size stays the same, and
    // no memory is allocated or deallocated
    void push_pop(const T &value)
    {
        if (mFixedSize)
        {
            mData[mCursor] = value;
            mCursor = (mCursor + 1) % mData.size();
        }
        else
        {
            mNonFixedSizeData.push_back(value);
            mNonFixedSizeData.pop_front();
        }
    }

    // For non fixed size
    //
    
    void push_back(const T &value)
    {
        set_fixed_size(false);
        mNonFixedSizeData.push_back(value);
    }

    void pop_front()
    {
        set_fixed_size(false);
        mNonFixedSizeData.pop_front();
    }
    
    // Get the i-th element
    T &operator[](int index)
    {
        if (mFixedSize)
            return mData[(mCursor + index) % mData.size()];
        else
            return mNonFixedSizeData[index];
    }

    const T &operator[](int index) const
    {
        if (mFixedSize)
            return mData[(mCursor + index) % mData.size()];
        else
            return mNonFixedSizeData[index];
    }
    
 protected:
    void set_fixed_size(bool fixed)
    {
        if (mFixedSize && !fixed)
        {
            mNonFixedSizeData.clear();
            for (int i = 0; i < mData.size(); i++)
            {
                mNonFixedSizeData.push_back(mData[(mCursor + i) % mData.size()]);
            }
            
            mData.clear();
            mCursor = 0;
        }
        else if (!mFixedSize && fixed)
        {
            mData.resize(mNonFixedSizeData.size());
            mCursor = 0;
            
            for (int i = 0; i < mNonFixedSizeData.size(); i++)
            {
                mData[i] = mNonFixedSizeData[i];
            }

            mNonFixedSizeData.clear();
        }

        mFixedSize = fixed;
    }

    //
    vector<T> mData;

    int mCursor;

    bool mFixedSize;
    deque<T> mNonFixedSizeData;
};
#endif
