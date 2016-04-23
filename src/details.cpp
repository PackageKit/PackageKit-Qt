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

#include "details.h"

#include <QVariant>

PackageKit::Details::Details()
{

}

PackageKit::Details::Details(const QVariantMap &other) :
    QVariantMap(other)
{
}

QString PackageKit::Details::packageId() const
{
    return value(QLatin1String("package-id")).toString();
}

QString PackageKit::Details::description() const
{
    return value(QLatin1String("description")).toString();
}

PackageKit::Transaction::Group PackageKit::Details::group() const
{
    return static_cast<Transaction::Transaction::Group>(value(QLatin1String("group")).toUInt());
}

QString PackageKit::Details::summary() const
{
    return value(QLatin1String("summary")).toString();
}

QString PackageKit::Details::url() const
{
    return value(QLatin1String("url")).toString();
}

QString PackageKit::Details::license() const
{
    return value(QLatin1String("license")).toString();
}

qulonglong PackageKit::Details::size() const
{
    return value(QLatin1String("size")).toULongLong();
}
