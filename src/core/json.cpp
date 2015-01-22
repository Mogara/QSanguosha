/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Rara
    *********************************************************************/

#include "json.h"

#include <QStringList>
#include <QFile>
#include <QRect>
#include <QColor>
#include <QJsonDocument>

JsonDocument::JsonDocument()
    :valid(false)
{
}

JsonDocument::JsonDocument(const QVariant &var)
    :value(var), valid(true)
{
}

JsonDocument::JsonDocument(const JsonArray &array)
    :value(array), valid(true)
{
}

JsonDocument::JsonDocument(const JsonObject &object)
    :value(object), valid(true)
{
}

JsonDocument JsonDocument::fromFilePath(const QString &path, bool allowComment)
{
    QFile file(path);
    file.open(QFile::ReadOnly);
    return fromJson(file.readAll(), allowComment);
}

bool JsonUtils::isStringArray(const QVariant &var, unsigned from, unsigned to)
{
    if (!var.canConvert<JsonArray>())
        return false;

    JsonArray array = var.value<JsonArray>();

    if ((unsigned) array.length() <= to)
        return false;
    for (unsigned int i = from; i <= to; i++) {
        if (!array.at(i).canConvert<QString>())
            return false;
    }
    return true;
}

bool JsonUtils::isNumberArray(const QVariant &var, unsigned from, unsigned to)
{
    if (!var.canConvert<JsonArray>())
        return false;

    JsonArray array = var.value<JsonArray>();

    if ((unsigned) array.length() <= to)
        return false;
    for (unsigned int i = from; i <= to; i++) {
        if (!array.at(i).canConvert<int>())
            return false;
    }
    return true;
}

QVariant JsonUtils::toJsonArray(const QList<int> &intArray)
{
    JsonArray json;
    foreach (int number, intArray) {
        json << number;
    }
    return json;
}

QVariant JsonUtils::toJsonArray(const QStringList &stringArray)
{
    JsonArray json;
    foreach (const QString &string, stringArray) {
        json << string;
    }
    return json;
}


bool JsonUtils::tryParse(const QVariant &arg, int &result) {
    if (!arg.canConvert<int>()) return false;
    result = arg.toInt();
    return true;
}

bool JsonUtils::tryParse(const QVariant &arg, double &result) {
    if (arg.canConvert<double>())
        result = arg.toDouble();
    else
        return false;
    return true;
}

bool JsonUtils::tryParse(const QVariant &arg, bool &result) {
    if (!arg.canConvert<bool>()) return false;
    result = arg.toBool();
    return true;
}

bool JsonUtils::tryParse(const QVariant &var, QStringList &list)
{
    if (!var.canConvert<JsonArray>())
        return false;

    JsonArray array = var.value<JsonArray>();

    foreach (const QVariant &var, array) {
        if (!var.canConvert<QString>()) {
            return false;
        }
    }

    foreach (const QVariant &var, array) {
        list << var.toString();
    }

    return true;
}

bool JsonUtils::tryParse(const QVariant &var, QList<int> &list)
{
    if (!var.canConvert<JsonArray>())
        return false;

    JsonArray array = var.value<JsonArray>();

    foreach (const QVariant &var, array) {
        if (!var.canConvert<int>()) {
            return false;
        }
    }

    foreach (const QVariant &var, array) {
        list << var.toInt();
    }

    return true;
}

bool JsonUtils::tryParse(const QVariant &arg, Qt::Alignment &align) {
    if (!JsonUtils::isString(arg)) return false;
    QString alignStr = arg.toString().toLower();
    if (alignStr.contains("left"))
        align = Qt::AlignLeft;
    else if (alignStr.contains("right"))
        align = Qt::AlignRight;
    else if (alignStr.contains("center"))
        align = Qt::AlignHCenter;

    if (alignStr.contains("top"))
        align |= Qt::AlignTop;
    else if (alignStr.contains("bottom"))
        align |= Qt::AlignBottom;
    else if (alignStr.contains("center"))
        align |= Qt::AlignVCenter;

    return true;
}

bool JsonUtils::tryParse(const QVariant &arg, QRect &result) {
    JsonArray args = arg.value<JsonArray>();
    if (args.size() != 4) return false;

    result.setLeft(args[0].toInt());
    result.setTop(args[1].toInt());
    result.setWidth(args[2].toInt());
    result.setHeight(args[3].toInt());

    return true;
}

bool JsonUtils::tryParse(const QVariant &arg, QSize &result) {
    JsonArray args = arg.value<JsonArray>();
    if (args.size() != 2) return false;
    result.setWidth(args[0].toInt());
    result.setHeight(args[1].toInt());
    return true;
}

bool JsonUtils::tryParse(const QVariant &arg, QPoint &result) {
    JsonArray args = arg.value<JsonArray>();
    if (args.size() != 2) return false;
    result.setX(args[0].toInt());
    result.setY(args[1].toInt());
    return true;
}

bool JsonUtils::tryParse(const QVariant &arg, QColor &color) {
    JsonArray args = arg.value<JsonArray>();
    if (args.size() < 3) return false;

    color.setRed(args[0].toInt());
    color.setGreen(args[1].toInt());
    color.setBlue(args[2].toInt());
    color.setAlpha(args.size() > 3 ? args[3].toInt() : 255);

    return true;
}

QByteArray clearComment(const QByteArray &src)
{
    QByteArray result(src);
    int max = result.size() - 1;
    for (int i = 0; i < max; i++) {
        switch (result.at(i)) {
        case '/':
            if (result.at(i + 1) == '*') {//multi-line comment
                int offset = i;
                i++;
                while(i < max && (result.at(i) != '*' || result.at(i + 1) != '/')) {
                    i++;
                }

                int length = i + 2 - offset;
                result.remove(offset, length);
                i = offset;
                max -= length;

            } else if (result.at(i + 1) == '/') {//single-line comment
                int offset = i;
                i++;
                while(i < max + 1 && result.at(i) != '\n') {
                    i++;
                }

                int length = i + 1 - offset;
                result.remove(offset, length);
                i = offset;
                max -= length;
            }
            break;
        case '"'://string
            while(i < max + 1 && result.at(i) != '"') {
                if (result.at(i) == '\\') {
                    i += 2;
                } else {
                    i++;
                }
            }
            break;
        default:;
        }
    }
    return result;
}

QByteArray JsonDocument::toJson(bool isIndented) const
{
    QJsonDocument doc = QJsonDocument::fromVariant(value);
    return doc.toJson(isIndented ? QJsonDocument::Indented : QJsonDocument::Compact);
}

JsonDocument JsonDocument::fromJson(const QByteArray &json, bool allowComment)
{
    QJsonParseError error;
    QJsonDocument jsondoc = QJsonDocument::fromJson(allowComment ? clearComment(json) : json, &error);

    JsonDocument doc;
    if (error.error == QJsonParseError::NoError) {
        doc.value = jsondoc.toVariant();
        doc.valid = true;
    } else {
        doc.valid = false;
        doc.error = error.errorString();
    }
    return doc;
}
