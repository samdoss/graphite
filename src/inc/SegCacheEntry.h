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
#pragma once

#ifndef GRAPHITE2_NSEGCACHE

#include "inc/Main.h"
#include "inc/Slot.h"

namespace graphite2 {

class Segment;
class SegCacheEntry;

enum SegCacheParameters {
    /** number of characters used in initial prefix tree */
    ePrefixLength = 2,
    /** Segments more recent than maxSegmentCount() / eAgeFactor are kept */
    eAgeFactor = 4,
    /** Segments are purged according to the formular:
     * accessCount < (totalAccesses)/(ePurgeFactor * maxSegments) */
    ePurgeFactor = 5,
    /** Maximum number of Segments to store which have the same
     * prefix. Needed to prevent unique identifiers flooding the cache */
    eMaxSuffixCount = 15

};

class SegCacheCharInfo
{
public :
    uint16 m_unicode;
    uint16 m_before;
    uint16 m_after;
};

/**
 * SegCacheEntry stores the result of running the engine for specific unicode
 * code points in the typical mid-line situation.
 */
class SegCacheEntry
{
public :
    SegCacheEntry() :
        m_glyphLength(0), m_unicode(NULL), m_glyph(NULL), m_attr(NULL)
    {}
    SegCacheEntry(Segment *seg, size_t start, size_t end);
    ~SegCacheEntry() { clear(); };

    bool cmp(SegCacheEntry *e);
    void hash(uint16 *h, uint16 *h1, uint16 size);

    void addSegment(Segment * seg, size_t charOffset);
    void clear();
    size_t glyphLength() const { return m_glyphLength; }
    const Slot * first() const { return m_glyph; }
    const Slot * last() const { return m_glyph + (m_glyphLength - 1); }
    size_t length() const { return m_length; }
    const unsigned int *unicode() const { return m_unicode; }

    void log(size_t unicodeLength) const;

    CLASS_NEW_DELETE;
private:

    size_t m_length;
    size_t m_glyphLength;
    /** glyph ids resulting from cmap mapping from unicode to glyph before substitution
     * the length of this array is determined by the position in the SegCachePrefixEntry */
    unsigned int * m_unicode;
    /** slots after shapping and positioning */
    Slot * m_glyph;
    int16 * m_attr;
};

} // namespace graphite2

#endif
