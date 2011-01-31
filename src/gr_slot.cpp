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
    If not, write to the Free Software Foundation, Inc., 59 Temple Place, 
    Suite 330, Boston, MA 02111-1307, USA or visit their web page on the 
    internet at http://www.fsf.org/licenses/lgpl.html.
*/
#include "graphite2/Segment.h"
#include "Segment.h"
#include "Slot.h"
#include "Font.h"


extern "C" {


const gr_slot* gr_slot_next_in_segment(const gr_slot* p/*not NULL*/)
{
    assert(p);
    return static_cast<const gr_slot*>(p->next());
}

const gr_slot* gr_slot_prev_in_segment(const gr_slot* p/*not NULL*/)
{
    assert(p);
    return static_cast<const gr_slot*>(p->prev());
}

const gr_slot* gr_slot_attached_to(const gr_slot* p/*not NULL*/)        //returns NULL iff base. If called repeatedly on result, will get to a base
{
    assert(p);
    return static_cast<const gr_slot*>(p->attachTo());
}


const gr_slot* gr_slot_first_attachment(const gr_slot* p/*not NULL*/)        //returns NULL iff no attachments.
{        //if slot_first_attachment(p) is not NULL, then slot_attached_to(slot_first_attachment(p))==p.
    assert(p);
    return static_cast<const gr_slot*>(p->firstChild());
}

    
const gr_slot* gr_slot_next_sibling_attachment(const gr_slot* p/*not NULL*/)        //returns NULL iff no more attachments.
{        //if slot_next_sibling_attachment(p) is not NULL, then slot_attached_to(slot_next_sibling_attachment(p))==slot_attached_to(p).
    assert(p);
    return static_cast<const gr_slot*>(p->nextSibling());
}


unsigned short gr_slot_gid(const gr_slot* p/*not NULL*/)
{
    assert(p);
    return p->glyph();
}


float gr_slot_origin_X(const gr_slot* p/*not NULL*/)
{
    assert(p);
    return p->origin().x;
}


float gr_slot_origin_Y(const gr_slot* p/*not NULL*/)
{
    assert(p);
    return p->origin().y;
}


float gr_slot_advance_X(const gr_slot* p/*not NULL*/, const gr_face *face, const gr_font *font)
{
    assert(p);
    float scale = 1.0;
    float res = p->advance();
    if (font)
    {
        scale = font->scale();
        if (face && font->isHinted())
            res = (res - face->advance(p->gid())) * scale + font->advance(p->gid());
        else
            res = res * scale;
    }
    return res;
}

float gr_slot_advance_Y(const gr_slot *p/*not NULL*/, const gr_face *face, const gr_font *font)
{
    assert(p);
    float res = p->advancePos().y;
    if (font && (face || !face))
        return res * font->scale();
    else
        return res;
}
        
int gr_slot_before(const gr_slot* p/*not NULL*/)
{
    assert(p);
    return p->before();
}


int gr_slot_after(const gr_slot* p/*not NULL*/)
{
    assert(p);
    return p->after();
}

int gr_slot_attr(const gr_slot* p/*not NULL*/, const gr_segment* pSeg/*not NULL*/, gr_attrCode index, gr_uint8 subindex)
{
    assert(p);
    return p->getAttr(pSeg, index, subindex);
}


int gr_slot_can_insert_before(const gr_slot* p/*not NULL*/)
{
    assert(p);
    return (p->isInsertBefore())? 1 : 0;
}


int gr_slot_original(const gr_slot* p/*not NULL*/)
{
    assert(p);
    return p->original();
}


#if 0       //what should this be
size_t id(const gr_slot* p/*not NULL*/)
{
    return (size_t)p->id();
}
#endif


} // extern "C"
