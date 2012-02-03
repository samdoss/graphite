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

#ifndef GRAPHITE2_NSEGCACHE

#include "inc/Main.h"
#include "inc/Slot.h"
#include "inc/Segment.h"
#include "inc/SegCacheEntry.h"


using namespace graphite2;

SegCacheEntry::SegCacheEntry(Segment *seg, size_t start, size_t end)
: m_glyph(NULL), m_glyph_end(NULL),
  m_attr(NULL), m_unicode(gralloc<unsigned int>(end - start)),
  m_length(end - start)
{
    for (unsigned int i = start, *p = m_unicode; i != end; ++i, ++p)
        *p = seg->charinfo(i)->unicodeChar();
}

SegCacheEntry::~SegCacheEntry()
{
    free(m_unicode);
    free(m_attr);
    delete [] m_glyph;
}


void SegCacheEntry::addSegment(Segment * seg, size_t charOffset)
{
    const size_t glyphCount = seg->slotCount();
    const Slot * slot = seg->first();
    m_glyph = new Slot[glyphCount];
    m_glyph_end = m_glyph + glyphCount;
    m_attr = gralloc<int16>(glyphCount * seg->numAttrs());
    Slot * slotCopy = m_glyph;
    m_glyph->prev(NULL);
    m_glyph_end[-1].next(NULL);

    seg->associateChars();

    slot = seg->first();
    unsigned int pos = 0;
    while (slot)
    {
        slotCopy->userAttrs(m_attr + pos * seg->numAttrs());
        slotCopy->set(*slot, -static_cast<int32>(charOffset), seg->numAttrs());
        if (slot->firstChild())
        { slotCopy->m_child = m_glyph + slot->firstChild()->index(); }
        if (slot->attachedTo())
        { slotCopy->attachTo(m_glyph + slot->attachedTo()->index()); }
        if (slot->nextSibling())
        { slotCopy->m_sibling = m_glyph + slot->nextSibling()->index(); }
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
    uint32 nh  = 16777551,
    	   nh1 = 2166136261;
    for (unsigned int *g = m_unicode, *e = m_unicode + m_length; g != e; ++g)
    {
        nh = ((nh << 7) | (nh >> 31)) ^ *g;
        nh1 = (nh1 * 16777619) ^ *g;
    }
    *h = nh % size;
    *h1 = nh1 % size;
}

#endif


