#include "json.h"

JsonDocument::JsonDocument()
{
}

JsonDocument::JsonDocument(const QVariant &var)
    :value(var)
{
}

JsonDocument::JsonDocument(const JsonArray &array)
{
    value = array;
}

JsonDocument::JsonDocument(const JsonObject &object)
{
    value = object;
}

JsonArray JsonDocument::array() const
{
    return value.value<JsonArray>();
}

JsonObject JsonDocument::object() const
{
    return value.value<JsonObject>();
}

bool JsonDocument::isArray() const
{
    return value.canConvert<JsonArray>();
}

bool JsonDocument::isObject() const
{
    return value.canConvert<JsonObject>();
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
    QJsonDocument jsondoc = QJsonDocument::fromJson(json);
    JsonDocument doc;
    doc.value = jsondoc.toVariant();
    return doc;
}
#else

#include <json/json.h>
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
    reader.parse(json.constData(), root);
    return JsonValueToVariant(root);
}

#endif
