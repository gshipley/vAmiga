// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _BEAM_INC
#define _BEAM_INC

struct Beam
{
    int16_t v;
    int16_t h;

    Beam(int16_t v, int16_t h) : v(v), h(h) { }
    Beam(uint32_t cycle = 0) : Beam(cycle / HPOS_CNT, cycle % HPOS_CNT) { }

    Beam& operator=(const Beam& beam)
    {
        v = beam.v;
        h = beam.h;
        return *this;
    }

    bool operator==(const Beam& beam) const
    {
        return v == beam.v && h == beam.h;
    }

    bool operator!=(const Beam& beam) const
    {
        return v != beam.v || h != beam.h;
    }

    Beam& operator+=(const Beam& beam)
    {
        v += beam.v;
        h += beam.h;

        if (h >= HPOS_CNT) { h -= HPOS_CNT; v++; }
        else if (h < 0)    { h += HPOS_CNT; v--; }

        return *this;
    }

    Beam operator+(const Beam& beam) const
    {
        int16_t newv = v + beam.v;
        int16_t newh = h + beam.h;

        if (newh >= HPOS_CNT) { newh -= HPOS_CNT; newv++; }
        else if (newh < 0)    { newh += HPOS_CNT; newv--; }

        return Beam(newv, newh);
    }

    Beam operator+(const int i) const
    {
        return *this + Beam(i);
    }

    int operator-(const Beam& beam) const
    {
        return (v - beam.v) * HPOS_CNT + (h - beam.h);
    }

    Beam& operator++()
    {
        if (++h > HPOS_MAX) { v++; h = 0; }
        return *this;
    }

    Beam& operator--()
    {
        if (--h < 0) { v--; h = HPOS_MAX; }
        return *this;
    }
};

#endif
