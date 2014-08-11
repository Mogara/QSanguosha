#ifndef JSON_H
#define JSON_H

#include <QVariantList>
#include <QVariantMap>

#include <json/json.h>

typedef QVariantList JsonArray;
typedef QVariantMap JsonObject;

class JsonDocument{
public:
    JsonDocument();
    JsonDocument(const QVariant &var);

    JsonDocument(const JsonArray &array);
    JsonDocument(const JsonObject &object);

    QByteArray toJson(bool isIndented = false) const;
    static JsonDocument fromJson(const QByteArray &json);

    bool isArray() const;
    bool isObject() const;

    JsonArray array() const;
    JsonObject object() const;

protected:
    QVariant value;
};

namespace JsonUtils{
    bool isStringArray(const JsonArray &array, unsigned from, unsigned int to);
    bool isIntArray(const JsonArray &array, unsigned from, unsigned int to);
}

Json::Value VariantToJsonValue(const QVariant &var);
QVariant JsonValueToVariant(const Json::Value &var);

#endif // JSON_H
