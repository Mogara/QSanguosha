#ifndef JSON_H
#define JSON_H

#include <QVariantList>
#include <QVariantMap>

#include <json/json.h>

//Directly apply two containers of Qt here. Reimplement the 2 classes if necessary.
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
    static JsonDocument fromFilePath(const QString &path);

    inline bool isArray() const{return value.canConvert<JsonArray>();}
    inline bool isObject() const{return value.canConvert<JsonObject>();}
    inline bool isValid() const{return valid;}

    inline JsonArray array() const{return value.value<JsonArray>();}
    inline JsonObject object() const{return value.value<JsonObject>();}
    inline const QVariant& toVariant() const{return value;}
    inline const QString errorString() const{return error;}

protected:
    QVariant value;
    bool valid;
    QString error;
};

namespace JsonUtils{
    inline bool isNumber(const QVariant &var) {
        //three number types defined by JsonCPP
        return var.type() == QMetaType::Double || var.type() == QMetaType::Int || var.type() == QMetaType::UInt;
    }

    bool isStringArray(const QVariant &var, unsigned from, unsigned int to);
    bool isIntArray(const QVariant &var, unsigned from, unsigned int to);

    QVariant toJsonArray(const QList<int> &intArray);
    QVariant toJsonArray(const QStringList &stringArray);

    bool tryParse(const QVariant &, int &);
    bool tryParse(const QVariant &, double &);
    bool tryParse(const QVariant &, bool &);

    bool tryParse(const QVariant &var, QStringList &list);
    bool tryParse(const QVariant &var, QList<int> &list);
    bool tryParse(const QVariant &arg, QRect &result);
    bool tryParse(const QVariant &arg, QSize &result);
    bool tryParse(const QVariant &arg, QPoint &result);
    bool tryParse(const QVariant &arg, QColor &result);
    bool tryParse(const QVariant &arg, Qt::Alignment &align);
}

//@todo: these two functions are temporarily used to migrate the project from JsonCPP, and will soon be inaccessible.
Json::Value VariantToJsonValue(const QVariant &var);
QVariant JsonValueToVariant(const Json::Value &var);

#endif // JSON_H
