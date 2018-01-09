/*
 * This file is part of the QPackageKit project
 * Copyright (C) 2010-2016 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "bitfield.h"

using namespace PackageKit;

Bitfield::Bitfield ()
{
}

Bitfield::Bitfield (qulonglong val) : m_val (val)
{
}

Bitfield::~Bitfield ()
{
}

qulonglong Bitfield::operator& (qulonglong mask) const
{
    return m_val & (1ULL << mask);
}

qulonglong Bitfield::operator&= (qulonglong mask)
{
	m_val &= (1ULL << mask);
	return m_val;
}

qulonglong Bitfield::operator| (qulonglong mask) const
{
	return m_val | (1ULL << mask);
}

qulonglong Bitfield::operator|= (qulonglong mask)
{
	m_val |= (1ULL << mask);
	return m_val;
}

Bitfield Bitfield::operator& (Bitfield mask) const
{
	return m_val & mask.m_val;
}

Bitfield Bitfield::operator&= (Bitfield mask)
{
	m_val &= mask.m_val;
	return m_val;
}

Bitfield Bitfield::operator| (Bitfield mask) const
{
	return m_val | mask.m_val;
}

Bitfield Bitfield::operator|= (Bitfield mask)
{
	m_val |= mask.m_val;
	return m_val;
}

Bitfield& Bitfield::operator= (const Bitfield& other)
{
    if (this == &other)
        return *this;

    m_val = other.m_val;

    return *this;
}

bool Bitfield::operator==(const Bitfield &other)
{
    return m_val == other.m_val;
}
