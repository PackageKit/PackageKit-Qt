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

#ifndef PACKAGEKIT_BITFIELD_H
#define PACKAGEKIT_BITFIELD_H

#include <QtGlobal>
#include <QMetaType>

#include <packagekitqt_global.h>

namespace PackageKit {

class PACKAGEKITQT_LIBRARY Bitfield
{
public:
    Bitfield ();
    Bitfield (qulonglong val);
    ~Bitfield ();

    qulonglong operator& (qulonglong mask) const;
    qulonglong operator&= (qulonglong mask);
    qulonglong operator| (qulonglong mask) const;
    qulonglong operator|= (qulonglong mask);

    Bitfield operator& (Bitfield mask) const;
    Bitfield operator&= (Bitfield mask);
    Bitfield operator| (Bitfield mask) const;
    Bitfield operator|= (Bitfield mask);

    Bitfield& operator= (const Bitfield& other);
    bool operator==(const Bitfield &other);

private:
    qulonglong m_val = 0;
};

} // End namespace PackageKit

Q_DECLARE_METATYPE(PackageKit::Bitfield)

#endif
