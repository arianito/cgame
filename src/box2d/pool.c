// SPDX-FileCopyrightText: 2023 Erin Catto
// SPDX-License-Identifier: MIT

#include "pool.h"

#include "allocate.h"
#include "core.h"
#include "math.h"

#include "box2d/types.h"

#include <string.h>

void b2ValidatePool(const b2Pool* pool)
{
	B2_MAYBE_UNUSED(pool);
}

b2Pool b2CreatePool(int32_t objectSize, int32_t capacity)
{
	

	b2Pool pool;
	pool.objectSize = objectSize;
	pool.capacity = capacity > 1 ? capacity : 1;
	pool.count = 0;
	pool.memory = (char*)b2Alloc(pool.capacity * objectSize);

	pool.freeList = 0;
	for (int32_t i = 0; i < pool.capacity - 1; ++i)
	{
		b2Object* object = (b2Object*)(pool.memory + i * objectSize);
		object->index = i;
		object->next = i + 1;
		object->revision = 0;
	}

	b2Object* object = (b2Object*)(pool.memory + (pool.capacity - 1) * objectSize);
	object->index = pool.capacity - 1;
	object->next = B2_NULL_INDEX;
	object->revision = 0;

	return pool;
}

void b2DestroyPool(b2Pool* pool)
{
	b2Free(pool->memory, pool->capacity * pool->objectSize);
	pool->memory = NULL;
	pool->capacity = 0;
	pool->count = 0;
	pool->freeList = B2_NULL_INDEX;
	pool->objectSize = 0;
}

void b2GrowPool(b2Pool* pool, int32_t capacity)
{
	int32_t oldCapacity = pool->capacity;
	if (oldCapacity >= capacity)
	{
		return;
	}

	int32_t newCapacity = capacity > 2 ? capacity : 2;
	pool->capacity = newCapacity;
	char* newMemory = (char*)b2Alloc(pool->capacity * pool->objectSize);
	memcpy(newMemory, pool->memory, oldCapacity * pool->objectSize);
	b2Free(pool->memory, oldCapacity * pool->objectSize);
	pool->memory = newMemory;

	int32_t oldFreeList = pool->freeList;
	pool->freeList = oldCapacity;
	for (int32_t i = oldCapacity; i < newCapacity - 1; ++i)
	{
		b2Object* object = (b2Object*)(pool->memory + i * pool->objectSize);
		object->index = i;
		object->next = i + 1;
		object->revision = 0;
	}

	// Tail of free list
	b2Object* object = (b2Object*)(pool->memory + (newCapacity - 1) * pool->objectSize);
	object->index = newCapacity - 1;
	object->next = oldFreeList;
	object->revision = 0;

}

b2Object* b2AllocObject(b2Pool* pool)
{
	b2Object* newObject = NULL;
	if (pool->freeList != B2_NULL_INDEX)
	{
		newObject = (b2Object*)(pool->memory + pool->freeList * pool->objectSize);
		newObject->index = pool->freeList;
		newObject->revision += 1;
		pool->freeList = newObject->next;
		newObject->next = newObject->index;
		pool->count += 1;
		return newObject;
	}
	else
	{
		int32_t oldCapacity = pool->capacity;
		int32_t addedCapacity = maxf(2, oldCapacity / 2);
		int32_t newCapacity = maxf(2, oldCapacity + addedCapacity);
		pool->capacity = newCapacity;
		char* newMemory = (char*)b2Alloc(pool->capacity * pool->objectSize);
		memcpy(newMemory, pool->memory, oldCapacity * pool->objectSize);
		b2Free(pool->memory, oldCapacity * pool->objectSize);
		pool->memory = newMemory;

		newObject = (b2Object*)(pool->memory + oldCapacity * pool->objectSize);
		newObject->index = oldCapacity;
		newObject->revision = 0;
		newObject->next = newObject->index;

		// This assumes added capacity >= 2
		pool->freeList = oldCapacity + 1;
		for (int32_t i = oldCapacity + 1; i < newCapacity - 1; ++i)
		{
			b2Object* object = (b2Object*)(pool->memory + i * pool->objectSize);
			object->index = i;
			object->next = i + 1;
			object->revision = 0;
		}

		b2Object* object = (b2Object*)(pool->memory + (newCapacity - 1) * pool->objectSize);
		object->index = newCapacity - 1;
		object->next = B2_NULL_INDEX;
		object->revision = 0;

		pool->count += 1;

		return newObject;
	}
}

void b2FreeObject(b2Pool* pool, b2Object* object)
{
	
	
	

	object->next = pool->freeList;
	pool->freeList = object->index;
	pool->count -= 1;
}
