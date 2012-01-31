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

#include <graphite2/Segment.h>
#include "inc/CachedFace.h"
#include "inc/SegCacheStore.h"
#include "inc/SegCacheEntry.h"
#include "inc/Segment.h"
#include "inc/CharInfo.h"


using namespace graphite2;

CachedFace::CachedFace(const void* appFaceHandle/*non-NULL*/, gr_get_table_fn getTable2)
: Face(appFaceHandle, getTable2), m_cacheStore(0) 
{
}

CachedFace::~CachedFace()
{
    delete m_cacheStore;
}

bool CachedFace::setupCache(unsigned int cacheSize)
{
    m_cacheStore = new SegCacheStore(m_numSilf, cacheSize);
    return (m_cacheStore != NULL);
}


bool CachedFace::runGraphite(Segment *seg, const Silf *pSilf) const
{
    assert(pSilf);
    pSilf->runGraphite(seg, 0, pSilf->substitutionPass());

    if (!seg->charInfoCount()) return true;
    SilfSegCacheEntry * segCache = NULL;
    unsigned int silfIndex = 0;

    for (silfIndex = 0; silfIndex < m_numSilf; silfIndex++)
        if (&(m_silfs[silfIndex]) == pSilf)
            break;

    assert(silfIndex < m_numSilf);
    assert(m_cacheStore);
    segCache = m_cacheStore->getOrCreate(silfIndex, seg->getFeatures(0));
    // find where the segment can be broken
    Slot * subSegStartSlot = seg->first();
    Slot * subSegLastSlot = subSegStartSlot;
    Slot * subSegEndSlot = subSegStartSlot->next();
    int subSegStart = 0;
    for (unsigned int i = 0; i < seg->charInfoCount(); i++)
    {
        if (!subSegEndSlot || (i < seg->charInfoCount() - 1 && issplit(seg->charinfo(i))))
        {
            SegCacheEntry * entry = new SegCacheEntry(seg, subSegStart, i + 1);
            SegCacheEntry * result = segCache->hash()->insert(entry);
            if (result == entry)
            {
                unsigned int length = i - subSegStart + 1;
                SegmentScopeState scopeState = seg->setScope(subSegStartSlot, subSegLastSlot, length);
                pSilf->runGraphite(seg, 0, pSilf->numPasses());
                entry->addSegment(seg, subSegStart);
                seg->removeScope(scopeState);
            }
            else
            {
                delete entry;
                seg->splice(subSegStart, i - subSegStart + 1, subSegStartSlot, subSegLastSlot,
                    result->first(), result->glyphLength());
            }
            subSegStartSlot = subSegEndSlot;
            subSegStart = i + 1;
        }
        subSegLastSlot = subSegEndSlot;
        if (subSegEndSlot) subSegEndSlot = subSegEndSlot->next();
    }
    return true;
}

bool CachedFace::issplit(const CharInfo *c) const
{
    uint8 f = c->flags();
    uint  u = c->unicodeChar();
    if (f == 2) return true;
    if (f == 1) return false;
    if (u == 0x0020 || u == 0x200B) return true;
    return false;
}

#endif

