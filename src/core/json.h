#ifndef JSON_H
#define JSON_H

#include <QVariantList>
#include <QVariantMap>

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

#endif // JSON_H
