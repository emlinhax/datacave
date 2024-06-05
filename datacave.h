#include <Windows.h>

typedef unsigned long u32;
typedef unsigned long long u64;

#define DC_INLINE __forceinline

u64 DC_KEY = 0x77449911FFAABBCC;
#define DC_MAX_CAVES 64

namespace datacave
{
    struct cave_t
    {
        bool in_use;
        bool locked;
        u64 base;
        u64 size;
        u32 protect;
    };
    cave_t caves[DC_MAX_CAVES];

    void* lock_thread_handle;

    DC_INLINE cave_t* push_cave(cave_t cave)
    {
        for (int i = 0; i < DC_MAX_CAVES; i++)
        {
            if (caves[i].in_use == 0)
            {
                caves[i].base = cave.base;
                caves[i].size = cave.size;
                caves[i].protect = cave.protect;
                caves[i].locked = cave.locked;
                caves[i].in_use = true;
                return &caves[i];
            }
        }
    }

    DC_INLINE void pop_cave(cave_t* cave)
    {
        cave->in_use = false;
        VirtualProtect((void*)cave->base, cave->size, PAGE_READWRITE, &cave->protect);
        memset((void*)cave->base, 0, cave->size);
        VirtualFree((void*)cave->base, cave->size, MEM_FREE);
    }

    DC_INLINE void _xor(char* data, u64 data_len)
    {
        for (int i = 0; i < data_len; i++)
            data[i] ^= *(char*)((&DC_KEY) + i % 8);
    }

    DC_INLINE void xor_cave(cave_t* cave)
    {
        _xor((char*)cave->base, cave->size);
    }

    DC_INLINE void lock(cave_t* cave)
    {
        xor_cave(cave);

        u32 tmp;
        VirtualProtect((void*)cave->base, cave->size, PAGE_NOACCESS, &tmp);
        cave->locked = true;
    }

    DC_INLINE void unlock(cave_t* cave)
    {
        u32 tmp;
        VirtualProtect((void*)cave->base, cave->size, cave->protect, &tmp);
        cave->locked = false;

        xor_cave(cave);
    }

    DC_INLINE void lock_all()
    {
        for (int i = 0; i < DC_MAX_CAVES; i++)
        {
            if (caves[i].in_use && !caves[i].locked)
            {
                lock(&caves[i]);
            }
        }
    }

    DC_INLINE cave_t* cave_from_address(u64 base)
    {
        for (int i = 0; i < DC_MAX_CAVES; i++)
        {
            if (base >= caves[i].base && base < caves[i].base + caves[i].size)
            {
                return &(caves[i]);
            }
        }

        return 0;
    }

    DC_INLINE u64 allocate_memory(u64 size, u32 protect = PAGE_READWRITE, u32 allocation_type = MEM_COMMIT | MEM_RESERVE)
    {
        u64 base = (u64)VirtualAlloc((LPVOID)0, size, allocation_type, protect);
        if (base == 0)
            return 0;

        memset((void*)base, 0, size);

        cave_t cave;
        cave.base = base;
        cave.size = size;
        cave.protect = protect;
        lock(push_cave(cave));

        return cave.base;
    }

    DC_INLINE u64 secure_data(u64 data, u64 size)
    {
        MEMORY_BASIC_INFORMATION mbi;
        VirtualQuery((LPCVOID)data, &mbi, size);

        u64 base = allocate_memory(size, mbi.Type, mbi.Protect);
        memcpy((void*)base, (void*)data, size);
        return base;
    }

    DC_INLINE LONG WINAPI __cave_handler(EXCEPTION_POINTERS* exception_info)
    {
        u64 access_type = exception_info->ExceptionRecord->ExceptionInformation[0];
        u64 access_address = exception_info->ExceptionRecord->ExceptionInformation[1];

        cave_t* cave = cave_from_address(access_address);
        if (cave != 0)
        {
            unlock(cave);
            return EXCEPTION_CONTINUE_EXECUTION;
        }

        return EXCEPTION_EXECUTE_HANDLER;
    }

    // has to be called
    DC_INLINE void initialize()
    {
        AddVectoredExceptionHandler(1, __cave_handler);
    }
}
