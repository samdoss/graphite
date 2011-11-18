/*  GRAPHITE2 LICENSING

    Copyright 2010, SIL International
    All rights reserved.

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should also have received a copy of the GNU Lesser General Public
    License along with this library in the file named "LICENSE".
    If not, write to the Free Software Foundation, 51 Franklin Street,
    Suite 500, Boston, MA 02110-1335, USA or visit their web page on the
    internet at http://www.fsf.org/licenses/lgpl.html.

Alternatively, the contents of this file may be used under the terms of the
Mozilla Public License (http://mozilla.org/MPL) or the GNU General Public
License, as published by the Free Software Foundation, either version 2
of the License or (at your option) any later version.
*/

#ifndef DISABLE_SEGCACHE

#include "Main.h"
#include "Slot.h"
#include "Segment.h"
#include "SegCacheEntry.h"


using namespace graphite2;

struct Ptr {
    ptrdiff_t _p;
    uint _o;
};

class PtrMap {
public:
    PtrMap(size_t length) {
        _size = length * 1.5;
        if (!(_size & 1)) ++_size;
        _table = grzeroalloc<Ptr>((size_t)_size);
    }
    ~PtrMap() { free(_table); }
    void inc(uint &offset, uint &delta, uint hash);
    void insert(ptrdiff_t p, uint offset);
    uint lookup(ptrdiff_t p);
private:
    Ptr *_table;
    uint _size;
};

void PtrMap::inc(uint &offset, uint &delta, uint hash)
{
    if (delta == 0) delta = _size - 1;
    offset = (offset + delta) % _size;
    if (offset == hash)
    {
        offset = (offset + 1) % _size;
        if (delta == 1) delta = _size - 1;
        else --delta;
    }
}

void PtrMap::insert(ptrdiff_t p, uint offset)
{
    uint h = p % _size;
    uint h1 = (p / _size) % _size;
    uint o = h;
    while (_table[o]._p)
    {
        if (_table[o]._p == p) return;
        inc(o, h1, h);
    }
    _table[o]._p = p;
    _table[o]._o = offset;
}

uint PtrMap::lookup(ptrdiff_t p)
{
    uint h = p % _size;
    uint h1 = (p / _size) % _size;
    uint o = h;
    while (_table[o]._p)
    {
        if (_table[o]._p == p) return _table[o]._o;
        inc(o, h1, h);
    }
    return 0;
}

SegCacheEntry::SegCacheEntry(Segment *seg, size_t start, size_t end)
    : m_length(end - start), m_glyphLength(0), m_unicode(gralloc<unsigned int>(end - start)), m_glyph(NULL),
    m_attr(NULL)
{
    for (unsigned int i = start, *p = m_unicode; i < end; ++i)
        *p++ = seg->charinfo(i)->unicodeChar();
}

void SegCacheEntry::addSegment(Segment * seg, size_t charOffset)
{
    size_t glyphCount = seg->slotCount();
    const Slot * slot = seg->first();
    m_glyph = new Slot[glyphCount];
    m_attr = gralloc<uint16>(glyphCount * seg->numAttrs());
    m_glyphLength = glyphCount;
    Slot * slotCopy = m_glyph;
    m_glyph->prev(NULL);
    m_glyph[glyphCount - 1].next(NULL);

    PtrMap pmap = PtrMap(glyphCount);
    for (int i = 0; slot; slot = slot->next())
    { pmap.insert((ptrdiff_t)slot, ++i); }

    slot = seg->first();
    uint pos = 0;
    while (slot)
    {
        slotCopy->userAttrs(m_attr + pos * seg->numAttrs());
        slotCopy->set(*slot, -static_cast<int32>(charOffset), seg->numAttrs());
        if (slot->firstChild())
        { slotCopy->firstChild(m_glyph + pmap.lookup((ptrdiff_t)(slot->firstChild())) - 1); }
        if (slot->attachedTo())
        { slotCopy->attachTo(m_glyph + pmap.lookup((ptrdiff_t)(slot->attachedTo())) - 1); }
        if (slot->nextSibling())
        { slotCopy->nextSibling(m_glyph + pmap.lookup((ptrdiff_t)(slot->nextSibling())) - 1); }
        slot = slot->next();
        ++slotCopy;
        ++pos;
        if (slot)
        {
            slotCopy->prev(slotCopy-1);
            (slotCopy-1)->next(slotCopy);
        }
    }
}

bool SegCacheEntry::cmp(SegCacheEntry *e)
{
    if (m_length != e->length()) return false;
    for (const unsigned int *g = m_unicode, *end = m_unicode + m_length, *o = e->unicode(); g != end; ++g, ++o)
    { if (*g != *o) return false; }
    return true;
}

void SegCacheEntry::hash(uint16 *h, uint16 *h1, uint16 size)
{
    uint nh = 16777551;
    uint nh1 = 2166136261;
    for (unsigned int *g = m_unicode, *e = m_unicode + m_length; g != e; ++g)
    {
        nh = ((nh << 7) | (nh >> 31)) ^ *g;
        nh1 = (nh1 * 16777619) ^ *g;
    }
    *h = nh % size;
    *h1 = nh1 % size;
}

void SegCacheEntry::log(GR_MAYBE_UNUSED size_t unicodeLength) const
{
#ifndef DISABLE_TRACING
    if (XmlTraceLog::get().active())
    {
        XmlTraceLog::get().openElement(ElementSegCacheEntry);
        for (size_t i = 0; i < unicodeLength; i++)
        {
            XmlTraceLog::get().openElement(ElementText);
            XmlTraceLog::get().addAttribute(AttrGlyphId, m_unicode[i]);
            XmlTraceLog::get().closeElement(ElementText);
        }
        for (size_t i = 0; i < m_glyphLength; i++)
        {
            XmlTraceLog::get().openElement(ElementGlyph);
            XmlTraceLog::get().addAttribute(AttrGlyphId, m_glyph[i].gid());
            XmlTraceLog::get().addAttribute(AttrX, m_glyph[i].origin().x);
            XmlTraceLog::get().addAttribute(AttrY, m_glyph[i].origin().y);
            XmlTraceLog::get().addAttribute(AttrBefore, m_glyph[i].before());
            XmlTraceLog::get().addAttribute(AttrAfter, m_glyph[i].after());
            XmlTraceLog::get().closeElement(ElementGlyph);
        }
        XmlTraceLog::get().closeElement(ElementSegCacheEntry);
    }
#endif
}

void SegCacheEntry::clear()
{
    free(m_unicode);
    free(m_attr);
    if (m_glyph) delete [] m_glyph;
    m_unicode = NULL;
    m_glyph = NULL;
    m_glyphLength = 0;
    m_attr = NULL;
}

#endif

