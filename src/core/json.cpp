#include "json.h"

#include <QStringlist>

Json::Value VariantToJsonValue(const QVariant &var)
{
    switch (var.type()) {
    //bool
    case QMetaType::Bool:
        return var.toBool();

    //number
    case QMetaType::Int:
        return var.toInt();
    case QMetaType::UInt:
        return var.toUInt();
    case QMetaType::Float:
        return var.toFloat();
    case QMetaType::Double:
        return var.toDouble();
    case QMetaType::LongLong:
        return var.toLongLong();
    case QMetaType::ULongLong:
        return var.toULongLong();

    //string
    case QMetaType::QString:
        return var.toString().toUtf8().constData();
    case QMetaType::QByteArray:
        return var.toByteArray().constData();

    //array
    case QMetaType::QStringList:{
        Json::Value array(Json::arrayValue);
        foreach (const QString &str, var.toStringList()) {
            array.append(str.toUtf8().constData());
        }
        return array;
    }
    case QMetaType::QVariantList:{
        Json::Value array(Json::arrayValue);
        foreach (const QVariant &value, var.toList()) {
            array.append(VariantToJsonValue(value));
        }
        return array;
    }

    //object
    case QMetaType::QVariantMap:{
        Json::Value object(Json::objectValue);
        QMapIterator<QString, QVariant> iter(var.toMap());
        while (iter.hasNext()) {
            iter.next();
            object[iter.key().toUtf8().constData()] = VariantToJsonValue(iter.value());
        }
        return object;
    }
    case QMetaType::QVariantHash:{
        Json::Value object(Json::objectValue);
        QHashIterator<QString, QVariant> iter(var.toHash());
        while (iter.hasNext()) {
            iter.next();
            object[iter.key().toUtf8().constData()] = VariantToJsonValue(iter.value());
        }
        return object;
    }

    //null
    default:
        return Json::Value::null;
    }
}

QVariant JsonValueToVariant(const Json::Value &var)
{
    switch (var.type()) {
    //number
    case Json::intValue:
        return var.asInt();
    case Json::uintValue:
        return var.asUInt();
    case Json::realValue:
        return var.asDouble();

    //string
    case Json::stringValue:
        return QString::fromUtf8(var.asCString());

    //boolean
    case Json::booleanValue:
        return var.asBool();

    //array
    case Json::arrayValue:{
        JsonArray array;
        for (Json::ArrayIndex i = 0; i < var.size(); i++) {
            array.append(JsonValueToVariant(var[i]));
        }
        return array;
    }

    //object
    case Json::objectValue:{
        JsonObject object;
        for (Json::ValueIterator iter = var.begin(); iter != var.end(); iter++) {
            object.insert(QString::fromUtf8(iter.memberName()), JsonValueToVariant(*iter));
        }
        return object;
    }

    //null
    default:
        return QVariant();
    }
}

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

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QJsonDocument>

QByteArray JsonDocument::toJson(bool isIndented) const
{
    QJsonDocument doc = QJsonDocument::fromVariant(value);
    return doc.toJson(isIndented ? QJsonDocument::Indented : QJsonDocument::Compact);
}

JsonDocument JsonDocument::fromJson(const QByteArray &json)
{
    QJsonParseError error;
    QJsonDocument jsondoc = QJsonDocument::fromJson(json, &error);

    JsonDocument doc;
    if (error.error == QJsonParseError::NoError) {
        doc.value = jsondoc.toVariant();
        doc.valid = true;
    } else {
        doc.valid = false;
    }
    return doc;
}

#else

QByteArray JsonDocument::toJson(bool isIndented)
{
    if (isIndented) {
        Json::StyledWriter writer;
        return writer.write(VariantToJsonValue(value)).c_str();
    } else {
        Json::FastWriter writer;
        return writer.write(VariantToJsonValue(value)).c_str();
    }
}

JsonDocument JsonDocument::fromJson(const QByteArray &json)
{
    Json::Value root;
    Json::Reader reader;
    JsonDocument doc;
    doc.valid = reader.parse(json.constData(), root);
    if (doc.valid)
        doc.value = JsonValueToVariant(root);
    return doc;
}

#endif

bool JsonUtils::isStringArray(const JsonArray &array, unsigned from, unsigned int to)
{
    if ((unsigned) array.length() <= to)
        return false;
    for (unsigned int i = from; i <= to; i++) {
        if (!array.at(i).canConvert<QString>())
            return false;
    }
    return true;
}

bool JsonUtils::isIntArray(const JsonArray &array, unsigned from, unsigned int to)
{
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

bool JsonUtils::tryParse(const JsonArray &val, QStringList &list)
{
    foreach (const QVariant &var, val) {
        if (!var.canConvert<QString>()) {
            return false;
        }
    }

    foreach (const QVariant &var, val) {
        list << var.toString();
    }

    return true;
}

bool JsonUtils::tryParse(const JsonArray &val, QList<int> &list)
{
    foreach (const QVariant &var, val) {
        if (!var.canConvert<int>()) {
            return false;
        }
    }

    foreach (const QVariant &var, val) {
        list << var.toInt();
    }

    return true;
}
