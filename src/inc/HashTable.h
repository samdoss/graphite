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

#include "inc/Main.h"
#include "inc/FeatureVal.h"

namespace graphite2 {

#define delindex 0xFFFF
#define maxpercent 80

class HashElement
{
public:
    virtual bool cmp(HashElement *other) const = 0;
    virtual void hash(uint16 *h, uint16 *h1, uint16 size) const = 0;
};

template <class T> class HashEntry
{
public:
    HashEntry(uint16 h, T *dat, uint16 newness) : _h(h), _next_oldest(newness), _next_newest(delindex), _data(dat) {};
    ~HashEntry() {};

    uint16 hash() const { return _h; }
    void newest(uint16 offset) { _next_newest = offset; }
    uint16 newest() const { return _next_newest; }
    void oldest(uint16 offset) { _next_oldest = offset; }
    uint16 oldest() const { return _next_oldest; }
    T *data() const { return _data; }

    void del() { _next_oldest = _next_newest = delindex; _data = NULL; }
    bool isDel() const { return (_data == NULL && _next_oldest == delindex && _next_newest == delindex); }
    bool isEmpty() const { return (_data == NULL && !isDel()); }
    bool cmp(uint16 h, T *dat) const { return !isDel() && _h == h && _data->cmp(dat); }

    CLASS_NEW_DELETE;

private:
    uint16 _h;
    uint16 _next_oldest;
    uint16 _next_newest;
    T *_data;
};

template <class T> class HashTable
{
public:
    HashTable(uint16 size) : 
    _newest(delindex), _oldest(delindex), _level(0)
    {
        _size = size;
        if (_size > 0xFFFD) _size = 0xFFFD;
        else if (!(_size & 1)) ++_size;
        _full = _size * 100 / maxpercent;

        _table = grzeroalloc< HashEntry<T> >(_size);
    }
    ~HashTable() { free(_table); }

    T *insert(T *key)
    {
        uint16 h, h1;
        uint16 o, oldo = delindex;
        HashEntry<T> *t;

        key->hash(&h, &h1, _size);
        if (h1 == 0) ++h1;
        for (o = h, t = _table + o; !t->isEmpty(); t = _table + o)
        {
            if (!t->cmp(h, key))
            {
                if (t->isDel() && oldo == delindex) oldo = o;
                o = (o + h1) % _size;
                if (o == h)
                {
                    o = (o + 1) % _size;
                    if (--h1 == 0) h1 = _size - 1;
                }
            }
            else 
            {
                if (t->newest() != delindex) _table[t->newest()].oldest(t->oldest());
                if (t->oldest() != delindex) _table[t->oldest()].newest(t->newest());
                t->newest(delindex);
                t->oldest(_newest);
                _newest = o;
                return t->data();
            }
        }
        if (oldo != delindex) o = oldo;
        if (_newest != delindex) _table[_newest].newest(o);
        ::new(_table + o) HashEntry<T>(h, key, _newest);
        _newest = o;
        ++_level;
        if (_level > _full && _oldest != delindex)
        {
            uint16 next = _table[_oldest].newest();
            _table[_oldest].del();
            _oldest = next;
            _table[_oldest].oldest(delindex);
        }
        return key;
    }

    CLASS_NEW_DELETE;

private:
    HashEntry<T> *_table;
    uint16 _newest;
    uint16 _oldest;
    uint16 _size;
    uint16 _full;
    uint16 _level;
};

}

