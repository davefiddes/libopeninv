/*
 * This file is part of the libopeninv project.
 *
 * Copyright (C) 2025 David J. Fiddes <D.J@fiddes.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stddef.h>
#include <string.h>
#include <type_traits>

//! \brief A lock-free ring buffer for single producer and single consumer
//!
//! Based heavily on
//! https://www.codeproject.com/articles/Lock-Free-Single-Producer-Single-Consumer-Circular#heading_atomic_operations
//!
//! Rather than using C++11 atomics we use volatiles as Cortex-M3 does not
//! require memory barriers in a unicore system
//!
//! \tparam T The type of items to store in the buffer
//! \tparam capacity The maximum number of items the buffer can hold. For
//! optimal performance this should be a power of two.
//!
//! \note The actual capacity is one less than the template parameter to
//!       distinguish between full and empty states
template <typename T, size_t capacity>
class RingBuffer
{
public:
   static_assert(std::is_trivial<T>::value, "T must be POD or a trivial type");
   using value_type = T;

   //! \brief Default constructor
   RingBuffer() : head(0), tail(0)
   {
   }

   //! \brief Reset the buffer to empty state
   //!
   //! \note The system must be in a quiescent state when this is called
   void Reset()
   {
      head = 0;
      tail = 0;
   }

   //! \brief Push an item onto the buffer
   //!
   //! \param item The item to push
   //!
   //! \return true if the item was pushed, false if the buffer was full
   bool Push(const value_type& item)
   {
      const size_t current_tail = tail;
      const size_t next_tail = increment(current_tail);

      if (next_tail != head)
      {
         memcpy((void*)&data[current_tail], (void*)&item, sizeof(value_type));
         tail = next_tail;
         return true;
      }

      return false; // Buffer is full
   }

   //! \brief Pop an item from the buffer
   //!
   //! \param item Reference to store the popped item
   //!
   //! \return true if an item was popped, false if the buffer was empty
   bool Pop(value_type& item)
   {
      const size_t current_head = head;
      if (current_head == tail)
      {
         return false; // Buffer is empty
      }

      memcpy((void*)&item, (void*)&data[current_head], sizeof(value_type));
      head = increment(current_head);

      return true;
   }

   //! \brief Get the current number of items in the buffer
   size_t Size() const
   {
      return (tail - head + capacity) % capacity;
   }

   //! \brief Check if the buffer is empty
   //!
   //! \return true if the buffer is empty, false otherwise
   bool IsEmpty() const
   {
      return head == tail;
   }

   //! \brief Check if the buffer is full
   //!
   //! \return true if the buffer is full, false otherwise
   bool IsFull() const
   {
      return increment(tail) == head;
   }

private:
   size_t increment(size_t idx) const
   {
      return (idx + 1) % capacity;
   }

private:
   volatile value_type data[capacity];
   volatile size_t     head; // Index of the next available slot to write
   volatile size_t     tail; // Index of the oldest item in the buffer
};

#endif // RINGBUFFER_H