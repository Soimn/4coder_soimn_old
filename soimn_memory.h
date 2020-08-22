inline void*
Align(void* ptr, U8 alignment)
{
    U64 overshot = (U64)ptr + (alignment - 1);
    U64 rounded  = overshot & ~(alignment - 1);
    return (void*)rounded;
}

inline U8
AlignOffset(void* ptr, U8 alignment)
{
    U64 overshot = (U64)ptr + (alignment - 1);
    U64 rounded  = overshot & ~(alignment - 1);
    return (U8)(overshot - rounded);
}

inline void
Copy(void* src, void* dst, U64 size)
{
    U8* bsrc = (U8*)src;
    U8* bdst = (U8*)dst;
    
    if (bsrc < bdst)
    {
        for (U64 i = 0; i < size; ++i)
        {
            bdst[size - i] = bsrc[size - i];
        }
    }
    
    else
    {
        for (U64 i = 0; i < size; ++i)
        {
            bdst[i] = bsrc[i];
        }
    }
}

#define CopyStruct(src, dst) Copy((src), (dst), sizeof((src)[0]))

inline void
Zero(void* ptr, U64 size)
{
    for (U64 i = 0; i < size; ++i)
    {
        ((U8*)ptr)[i] = 0;
    }
}

#define ZeroStruct(ptr) Zero((ptr), sizeof((ptr)[0]))

typedef struct Memory_Block
{
    alignas(8) Memory_Block* next;
    alignas(8) U32 space;
    alignas(4) U32 offset;
} Memory_Block;

typedef struct Memory_Free_Entry
{
    Memory_Free_Entry* next;
    U32 space;
    U8 offset;
} Memory_Free_Entry;

#define MEMORY_ARENA_DEFAULT_BLOCK_SIZE KB(4)
typedef struct Memory_Arena
{
    Memory_Free_Entry* first_free;
    Memory_Block* first_block;
    Memory_Block* current_block;
} Memory_Arena;

void
Arena_Free(Memory_Arena* arena, void* ptr, U64 size)
{
    ASSERT(size <= U32_MAX);
    
#if SOIMN_DEBUG_MODE
    bool was_found = false;
    
    for (Memory_Block* block = arena->first_block; block != 0; block = block->next)
    {
        if (ptr >= block + 1 && ptr < (U8*)block + block->offset)
        {
            was_found = true;
            break;
        }
    }
    
    ASSERT(was_found);
#endif
    
    if (size >= sizeof(Memory_Free_Entry) + (alignof(Memory_Free_Entry) - 1))
    {
        Memory_Free_Entry* entry = (Memory_Free_Entry*)Align(ptr, alignof(void*));
        entry->next   = arena->first_free;
        entry->offset = (U8)((U8*)entry - (U8*)ptr);
        entry->space  = (U32)(size - entry->offset);
        
        arena->first_free = entry;
    }
}

void
Arena_FreeAll(Memory_Arena* arena)
{
    for (Memory_Block* block = arena->first_block; block != 0; )
    {
        Memory_Block* next = block->next;
        
        free((U8*)block - sizeof(void*));
        block = next;
    }
    
    arena->first_block = 0;
    arena->first_free  = 0;
}

void
Arena_ClearAll(Memory_Arena* arena)
{
    arena->first_block->space  = arena->first_block->offset - sizeof(Memory_Block);
    arena->first_block->offset = sizeof(Memory_Block);
    arena->current_block       = arena->first_block;
}

void*
Arena_Allocate(Memory_Arena* arena, U64 size, U8 alignment)
{
    ASSERT((alignment & (~alignment + 1)) == alignment && alignment <= 8);
    ASSERT(size <= U32_MAX);
    
    void* result = 0;
    
    if (arena->first_free != 0)
    {
        for (Memory_Free_Entry* scan = arena->first_free; scan != 0; scan = scan->next)
        {
            U8 offset = AlignOffset((U8*)scan - scan->offset, alignment);
            
            if (size <= scan->offset + scan->space - offset)
            {
                result = (U8*)scan + (offset - scan->offset);
                
                Arena_Free(arena, (U8*)result + size, (scan->offset + scan->space) - (offset + size));
            }
        }
    }
    
    if (result == 0)
    {
        if (arena->first_block == 0 ||
            arena->first_block->space < AlignOffset((U8*)arena->first_block + arena->first_block->offset, alignment) + size)
        {
            if (arena->first_block)
            {
                Arena_Free(arena, (U8*)arena->first_block + arena->first_block->offset, arena->first_block->space);
                
                arena->first_block->offset += arena->first_block->space;
                arena->first_block->space   = 0;
            }
            
            if (arena->current_block->next != 0)
            {
                arena->current_block = arena->current_block->next;
                arena->current_block->space  = arena->current_block->offset - sizeof(Memory_Block);
                arena->current_block->offset = sizeof(Memory_Block);
            }
            
            else
            {
                U32 block_size = (U32)MAX(MEMORY_ARENA_DEFAULT_BLOCK_SIZE, size);
                
                void* memory = (Memory_Block*)malloc((alignof(void*) - 1) +  sizeof(void*) +
                                                     sizeof(Memory_Block) + block_size);
                
                *(void**)Align(memory, alignof(void*)) = memory;
                
                Memory_Block* block = (Memory_Block*)((U8*)Align(memory, alignof(void*)) + sizeof(void*));
                block->next   = arena->first_block;
                block->offset = sizeof(Memory_Block);
                block->space  = block_size - block->offset;
                
                if (arena->current_block) arena->current_block->next = block;
                else                      arena->first_block         = block;
                
                arena->current_block = block;
            }
        }
        
        result = Align((U8*)arena->first_block + arena->first_block->offset, alignment);
        
        U32 advancement = (U32)(AlignOffset((U8*)arena->first_block + arena->first_block->offset, alignment) + size);
        arena->first_block->offset += advancement;
        arena->first_block->space  -= advancement;
    }
    
    return result;
}