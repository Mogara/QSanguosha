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
    QVariant toJsonArray(const QList<int> &intArray);
    QVariant toJsonArray(const QStringList &stringArray);

    bool tryParse(const JsonArray &val, QStringList &list);
}


//@todo: these two functions are temporarily used to migrate the project from JsonCPP, and will soon be inaccessible.
Json::Value VariantToJsonValue(const QVariant &var);
QVariant JsonValueToVariant(const Json::Value &var);

#endif // JSON_H
