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

#ifndef DISABLE_SEGCACHE

#include "Main.h"
#include "CmapCache.h"
#include "HashTable.h"
#include "SegCacheEntry.h"

namespace graphite2 {

class Face;

class SilfSegCacheEntry
{
public:
    SilfSegCacheEntry(uint16 size, const Features &f) : m_hash(new HashTable<SegCacheEntry>(size)), m_feats(f) {};
    ~SilfSegCacheEntry()
    { delete m_hash; }

    bool isContextInit(uint16 gid, const Silf *pSilf) const { return pSilf->isContextInit(gid); }
    HashTable<SegCacheEntry> *hash() const { return m_hash; }
    Features features() const { return m_feats; }

    CLASS_NEW_DELETE
private:
    HashTable<SegCacheEntry> *m_hash;
    Features m_feats;
};

class SilfSegCache
{
public:
    SilfSegCache() : m_caches(NULL), m_cacheCount(0) {};
    ~SilfSegCache()
    {
        for (uint i = 0; i < m_cacheCount; ++i)
        { delete m_caches[i]; }
        free(m_caches);
    }
    SilfSegCacheEntry * getOrCreate(uint16 size, const Features & features)
    {
        for (size_t i = 0; i < m_cacheCount; i++)
        {
            if (m_caches[i]->features() == features)
                return m_caches[i];
        }
        SilfSegCacheEntry ** newData = gralloc<SilfSegCacheEntry *>(m_cacheCount+1);
        if (newData)
        {
            if (m_cacheCount > 0)
            {
                memcpy(newData, m_caches, sizeof(SilfSegCacheEntry *) * m_cacheCount);
                free(m_caches);
            }
            m_caches = newData;
            m_caches[m_cacheCount] = new SilfSegCacheEntry(size, features);
            m_cacheCount++;
            return m_caches[m_cacheCount - 1];
        }
        return NULL;
    }
    CLASS_NEW_DELETE
private:
    SilfSegCacheEntry **m_caches;
    size_t m_cacheCount;
};

class SegCacheStore
{
public:
    SegCacheStore(const Face *face, unsigned int numSilf, size_t maxSegments);
    ~SegCacheStore()
    {
        delete [] m_caches;
        m_caches = NULL;
    }
    SilfSegCacheEntry * getOrCreate(unsigned int i, const Features & features)
    {
        return m_caches[i].getOrCreate(m_maxSegments, features);
    }

    CLASS_NEW_DELETE
private:
    SilfSegCache * m_caches;
    uint8 m_numSilf;
    uint32 m_maxSegments;
    uint16 m_maxCmapGid;
    uint16 m_spaceGid;
    uint16 m_zwspGid;
};

} // namespace graphite2

#endif

