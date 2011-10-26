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

#include "Main.h"
#Include "HashTable.h"

namespace graphite2 {

HashTable::HashTable(uint16 size, Features features) : _level(0),
    _newest = delindex, _oldest = delindex, _features = features
{
    _size = size;
    if (_size > 0xFFFD) _size = 0xFFFD;
    else if (!_size & 1) ++_size;
    _full = _size * 100 / maxpercent;

    _table = grzeroalloc<HashElement>(_size);
}

HashTable::~HashTable()
{
    free(_table);
}

}

