// SPDX-FileCopyrightText: 2023 Erin Catto
// SPDX-License-Identifier: MIT

#include "arena_allocator.h"

#include <stdbool.h>
#include "mem/mem.h"

#include "array.h"
#include "core.h"

#include "mem/mem.h"

typedef struct b2StackEntry
{
	char* data;
	const char* name;
	int32_t size;
	bool usedMalloc;
} b2StackEntry;

// This is a stack-like arena allocator used for fast per step allocations.
// You must nest allocate/free pairs. The code will B2_ASSERT
// if you try to interleave multiple allocate/free pairs.
// This allocator uses the heap if space is insufficient.
// I could remove the need to free entries individually.
typedef struct b2StackAllocator
{
	char* data;
	int32_t capacity;
	int32_t index;

	int32_t allocation;
	int32_t maxAllocation;

	b2StackEntry* entries;
} b2StackAllocator;

b2StackAllocator* b2CreateStackAllocator(int32_t capacity)
{
	b2StackAllocator* allocator = xxmalloc(sizeof(b2StackAllocator));
	allocator->capacity = capacity;
	allocator->data = xxmalloc(capacity);
	allocator->allocation = 0;
	allocator->maxAllocation = 0;
	allocator->index = 0;
	allocator->entries = b2CreateArray(sizeof(b2StackEntry), 32);
	return allocator;
}

void b2DestroyStackAllocator(b2StackAllocator* allocator)
{
	b2DestroyArray(allocator->entries, sizeof(b2StackEntry));
	xxfree(allocator->data, allocator->capacity);
	xxfree(allocator, sizeof(b2StackAllocator));
}

void* b2AllocateStackItem(b2StackAllocator* alloc, int32_t size, const char* name)
{
	// ensure allocation is 32 byte aligned to support 256-bit SIMD
	int32_t size32 = ((size - 1) | 0x1F) + 1;

	b2StackEntry entry;
	entry.size = size32;
	entry.name = name;
	if (alloc->index + size32 > alloc->capacity)
	{
		// fall back to the heap (undesirable)
		entry.data = (char*)xxmalloc(size32);
		entry.usedMalloc = true;
	}
	else
	{
		entry.data = alloc->data + alloc->index;
		entry.usedMalloc = false;
		alloc->index += size32;
	}

	alloc->allocation += size32;
	if (alloc->allocation > alloc->maxAllocation)
	{
		alloc->maxAllocation = alloc->allocation;
	}

	b2Array_Push(alloc->entries, entry);
	return entry.data;
}

void b2FreeStackItem(b2StackAllocator* alloc, void* mem)
{
	int32_t entryCount = b2Array(alloc->entries).count;
	b2StackEntry* entry = alloc->entries + (entryCount - 1);
	if (entry->usedMalloc)
	{
		xxfree(mem, entry->size);
	}
	else
	{
		alloc->index -= entry->size;
	}
	alloc->allocation -= entry->size;
	b2Array_Pop(alloc->entries);
}

void b2GrowStack(b2StackAllocator* alloc)
{
	if (alloc->maxAllocation > alloc->capacity)
	{
		xxfree(alloc->data, alloc->capacity);
		alloc->capacity = alloc->maxAllocation + alloc->maxAllocation / 2;
		alloc->data = xxmalloc(alloc->capacity);
	}
}

int32_t b2GetStackCapacity(b2StackAllocator* alloc)
{
	return alloc->capacity;
}

int32_t b2GetStackAllocation(b2StackAllocator* alloc)
{
	return alloc->allocation;
}

int32_t b2GetMaxStackAllocation(b2StackAllocator* alloc)
{
	return alloc->maxAllocation;
}
