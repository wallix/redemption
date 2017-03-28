/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Javier Caverni, Martin Potier,
              Meng Tan
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   This file implement the bitmap items data structure
   including RDP RLE compression and decompression algorithms

   It also features storage and color versionning of the bitmap
   returning a pointer on a table, corresponding to the required
   color model.
*/


#pragma once

#include <new>

using std::size_t;

namespace aux_ {
    extern class BmpMemAlloc {
        class Memory {
            void * mem_first = nullptr;
            void * mem_last = nullptr;
            char * * first = nullptr;
            char * * last = nullptr;
            char * * pos = nullptr;
            size_t size = 0;

        public:
            Memory() = default;

            void init(char * * beg, char * * end, size_t sz) noexcept {
                this->mem_first = *beg;
                this->mem_last = *(end-1) + sz;
                this->first = beg;
                this->last = end;
                this->pos = end;
                this->size = sz;
            }

            bool contains(void const * p) const noexcept {
                return this->mem_first <= p && p < this->mem_last;
            }

            bool empty() const noexcept {
                return this->pos == this->first;
            }

            size_t size_element() const noexcept {
                return this->size;
            }

            void * pop() noexcept {
                return *--this->pos;
            }

            void push(void * p) noexcept {
                *this->pos = static_cast<char*>(p);
                ++this->pos;
            }

            Memory(Memory const &) = delete;
            Memory & operator=(Memory const &) = delete;
        };

        Memory mems[5];
        void * data = nullptr;

    public:
        BmpMemAlloc() = default;

        ~BmpMemAlloc() {
            ::operator delete(this->data);
        }

        void * alloc(size_t n) {
            for (Memory & mem : this->mems) {
                if (n <= mem.size_element()) {
                    if (!mem.empty()) {
                        return mem.pop();
                    }
                }
            }
            return ::operator new(n);
        }

        void dealloc(void * p) noexcept {
            for (Memory & mem : this->mems) {
                if (mem.contains(p)) {
                    mem.push(p);
                    return ;
                }
            }
            ::operator delete(p);
        }

        struct MemoryDef {
            size_t cel;
            size_t sz;

        private:
            static const size_t align = sizeof(void*) > sizeof(size_t) ? sizeof(void*) : sizeof(size_t);

        public:
            MemoryDef(size_t cel, size_t sz)
            : cel(cel)
            , sz((sz + (align-1)) & ~(align-1))
            {}
        };

        void reserve(MemoryDef const & m1, MemoryDef const & m2, MemoryDef const & m3, MemoryDef const & m4, MemoryDef const & m5) {
            if (!this->data) {
                const size_t mem_size = m1.cel * m1.sz + m2.cel * m2.sz + m3.cel * m3.sz + m4.cel * m4.sz + m5.cel * m5.sz;
                const size_t ntotal = (m1.cel + m2.cel + m3.cel + m4.cel + m5.cel);
                this->data = ::operator new(mem_size + ntotal * sizeof(void*));
                char * p = static_cast<char*>(this->data);
                char * * pp = reinterpret_cast<char * *>(p + mem_size);
                const size_t cels[] = {m1.cel, m2.cel, m3.cel, m4.cel, m5.cel};
                const size_t szs[] = {m1.sz, m2.sz, m3.sz, m4.sz, m5.sz};
                for (unsigned i = 0; i < sizeof(this->mems)/sizeof(this->mems[0]); ++i) {
                    if (szs[i] * cels[i] == 0) {
                        continue;
                    }
                    char * * pp_tmp = pp;
                    char * * epp = pp + cels[i];
                    for (; pp < epp; ++pp, p += szs[i]) {
                        *pp = p;
                    }
                    this->mems[i].init(pp_tmp, epp, szs[i]);
                }
            }
        }
    } bitmap_data_allocator;
}

