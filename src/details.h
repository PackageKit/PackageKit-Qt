/*
 * This file is part of the PackageKitQt project
 * Copyright (C) 2014-2016 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef DETAILS_H
#define DETAILS_H

#include <QVariantMap>

#include "transaction.h"

namespace PackageKit {

class PACKAGEKITQT_LIBRARY Details : public QVariantMap
{
public:
    Details();

    Details(const QVariantMap &other);

    QString packageId() const;

    QString description() const;

    Transaction::Group group() const;

    QString summary() const;

    QString url() const;

    QString license() const;

    qulonglong size() const;

};

}

#endif // DETAILS_H
