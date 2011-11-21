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

#include "SegCacheStore.h"
#include "Face.h"


using namespace graphite2;

SegCacheStore::SegCacheStore(const Face *face, unsigned int numSilf, size_t maxSegments)
 : m_numSilf(numSilf), m_maxSegments(maxSegments)
{
    assert(face);
    assert(face->getGlyphFaceCache());
    m_caches = gralloc<SilfSegCache>(numSilf);
    while (numSilf--)
    {
        ::new (m_caches + numSilf) SilfSegCache(face, numSilf);
    }
    
}

SilfSegCache::SilfSegCache(const Face *face, int iSilf)
 : m_caches(NULL), m_cacheCount(0)
{
    m_isBmpOnly = face->cmap().isBmpOnly();
    m_charCut = grzeroalloc<uint8 *>(m_isBmpOnly ? 0x4 : 0x44);
    int cBlock = -1;
    uint32 chr = 0;
    uint16 gid = 0;
    while (face->cmap().nextEntry(&chr, &gid))
    {
        if ((chr >> 10) > cBlock)
        {
            cBlock = chr >> 10;
            m_charCut[cBlock] = grzeroalloc<uint8>(0x400);
        }
        m_charCut[cBlock][(chr >> 2) & 0xFF] = face->silf(iSilf)->cutGlyph(gid) << ((chr & 0x3) * 2);
    }
}

int SilfSegCache::cutChar(uint32 chr) const throw()
{
    if ((m_isBmpOnly && chr > 0xFFFF) || chr > 0x10FFFF)
        return 0;
    int block = chr >> 10;
    if (!m_charCut[block])
        return 0;
    return (m_charCut[block][(chr >> 2) & 0xFF] >> ((chr & 0x3) * 2)) & 0x3;
}

#endif

