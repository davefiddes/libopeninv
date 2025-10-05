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

#include "test.h"
#include <ringbuffer.h>

class RingBufferTest : public UnitTest
{
public:
   RingBufferTest(const std::list<VoidFunction>* cases) : UnitTest(cases)
   {
   }
};

static void TestInitialState()
{
   RingBuffer<int, 4> rb;

   ASSERT(rb.IsEmpty());
   ASSERT(!rb.IsFull());
   ASSERT(rb.Size() == 0);
   int value;
   ASSERT(!rb.Pop(value));
}

static void TestSinglePushPopSucceeds()
{
   RingBuffer<int, 4> rb;

   ASSERT(rb.Push(42));

   ASSERT(!rb.IsEmpty());
   ASSERT(!rb.IsFull());
   ASSERT(rb.Size() == 1);
}

static void TestSinglePushPopValue()
{
   RingBuffer<int, 4> rb;
   int                value;

   ASSERT(rb.Push(42));
   ASSERT(rb.Pop(value));

   ASSERT(value == 42);
   ASSERT(rb.IsEmpty());
   ASSERT(!rb.IsFull());
   ASSERT(rb.Size() == 0);
}

static void TestPushUntilFull()
{
   RingBuffer<int, 4> rb;

   ASSERT(rb.Push(1));
   ASSERT(rb.Push(2));
   ASSERT(rb.Push(3));

   ASSERT(!rb.IsEmpty());
   ASSERT(rb.IsFull());
   ASSERT(rb.Size() == 3);

   ASSERT(!rb.Push(4)); // Should fail as buffer is full
}

static void TestPushPopSequence()
{
   RingBuffer<int, 4> rb;
   int                value;

   ASSERT(rb.Push(1));
   ASSERT(rb.Push(2));
   ASSERT(rb.Push(3));

   ASSERT(rb.Pop(value));
   ASSERT(value == 1);

   ASSERT(rb.Push(4));
   ASSERT(rb.IsFull());
   ASSERT(rb.Size() == 3);

   ASSERT(rb.Pop(value));
   ASSERT(value == 2);
   ASSERT(rb.Pop(value));
   ASSERT(value == 3);
   ASSERT(rb.Pop(value));
   ASSERT(value == 4);

   ASSERT(rb.IsEmpty());
   ASSERT(!rb.IsFull());
   ASSERT(rb.Size() == 0);
}

static void TestLargeStructure()
{
   struct LargeStruct
   {
      int data[100];
   };

   RingBuffer<LargeStruct, 4> rb;
   LargeStruct                ls1, ls2;

   for (int i = 0; i < 100; i++)
   {
      ls1.data[i] = i;
   }

   ASSERT(rb.Push(ls1));
   ASSERT(rb.Pop(ls2));

   for (int i = 0; i < 100; i++)
   {
      ASSERT(ls2.data[i] == i);
   }

   ASSERT(rb.IsEmpty());
   ASSERT(!rb.IsFull());
   ASSERT(rb.Size() == 0);
}

static void TestResetFunction()
{
   RingBuffer<int, 4> rb;
   int                value;

   ASSERT(rb.Push(1));
   ASSERT(rb.Push(2));
   ASSERT(rb.Push(3));

   rb.Reset();

   ASSERT(rb.IsEmpty());
   ASSERT(!rb.IsFull());
   ASSERT(rb.Size() == 0);
   ASSERT(!rb.Pop(value));
}

// This line registers the test
REGISTER_TEST(
   RingBufferTest,
   TestInitialState,
   TestSinglePushPopSucceeds,
   TestSinglePushPopValue,
   TestPushUntilFull,
   TestPushPopSequence,
   TestLargeStructure,
   TestResetFunction);
