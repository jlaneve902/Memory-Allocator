# Memory Allocator

A simple memory allocator in C that reimplements `malloc()`, `calloc()`, `realloc()`, and `free()` on top of the `sbrk()` system call. I built it to learn how heap management works on Linux.

This is an educational project, not a production allocator.

## Features

- Custom `malloc`, `calloc`, `realloc`, and `free`
- Heap growth and shrinkage with `sbrk()`
- A hidden header before every block that stores its size, a free flag, and a pointer to the next block
- A linked list of blocks with first-fit reuse of freed blocks
- Overflow check in `calloc`
- A global mutex (`pthread`) so concurrent calls don't corrupt the block list
- Can be built as a shared library and loaded into real programs with `LD_PRELOAD`
- Unit tests written with the [Unity](https://github.com/ThrowTheSwitch/Unity) framework

## How it works

Each allocation is `sizeof(header_t) + size` bytes obtained from `sbrk()`. The caller receives a pointer just past the header (`header + 1`), so the bookkeeping is invisible to them. `free()` steps back one header from the pointer it receives to find the block's metadata.

- **`malloc`** takes the lock and scans the list for a free block that is large enough (first-fit). If one exists, it is marked in use and returned. Otherwise the heap is extended with `sbrk()` and the new block is appended to the list.
- **`free`** checks whether the block is the last one on the heap. If so, it is removed from the list and the heap shrinks with `sbrk()`. Otherwise it is only marked free so a later `malloc` can reuse it.
- **`calloc`** checks `num * size` for overflow, calls `malloc`, and zeroes the memory.
- **`realloc`** returns the same block if it is already large enough. Otherwise it allocates a new block, copies the data, and frees the old one.

## Requirements

- Linux (or WSL2 on Windows). `sbrk()` and `LD_PRELOAD` are not available on native Windows.
- `gcc` and `build-essential`
- Optional: `cmake` (3.16 or newer)

## Project layout

```
.
├── memalloc.c        # the allocator
├── tests/tests.c     # Unity unit tests
├── vendor/Unity/     # Unity test framework (MIT licensed)
├── CMakeLists.txt
└── README.md
```

## Build and test

With gcc directly:

```bash
gcc -Wall -Wextra -g -pthread -fno-builtin -Ivendor/Unity/src \
    -o alloc_test tests/tests.c vendor/Unity/src/unity.c memalloc.c
./alloc_test
```

With CMake:

```bash
cmake -S . -B build
cmake --build build
./build/alloc_test
ctest --test-dir build --output-on-failure
```

`-fno-builtin` stops gcc from optimizing away `malloc`/`free` pairs in the tests. `memalloc.c` is compiled directly into the test program, so the tests exercise this allocator rather than glibc's.

## Using it with a real program

Build the shared library and preload it:

```bash
gcc -fPIC -shared -pthread -o memalloc.so memalloc.c
LD_PRELOAD=$PWD/memalloc.so ls
```

Prefer setting `LD_PRELOAD` per command as shown. Exporting it affects every program you run in that shell.

## Tests

The suite covers:

- `malloc`: non-NULL return, zero-size request, program break advancing by header plus size
- `free`: releasing the tail block shrinks the heap, and a freed middle block is reused without calling `sbrk`
- `calloc`: zeroing a reused dirty block, and overflow returning `NULL`
- `realloc`: growing preserves contents, and shrinking returns the same pointer

## Known limitations

- **Alignment:** the header is padded to 32 bytes with `_Alignas(16)`, but requested sizes are not rounded up to a multiple of 16, so returned pointers can be misaligned after odd-sized allocations. The initial program break is also not aligned.
- **No splitting or coalescing:** a freed block reused for a smaller request wastes the leftover space, and adjacent free blocks are never merged.
- **Slow lookups:** first-fit is O(n), and removing the tail block walks the whole list.
- **Only the tail shrinks the heap:** a free block that becomes last stays on the heap until another tail free happens.
- **`sbrk()` is not thread-safe on its own,** and the lock only protects this allocator's own bookkeeping.
- Not tuned for performance.

## Credits

Based on the tutorial "Memory Allocators 101 - Write a simple memory allocator" (link the original article here). Unity is by ThrowTheSwitch and is MIT licensed.