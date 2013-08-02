#ifndef UTIL_H
#define UTIL_H

struct lua_State;
class QVariant;

#include <QList>
#include <QStringList>

template<typename T>
void qShuffle(QList<T> &list){
    int i, n = list.length();
    for(i=0; i<n; i++){
        int r = qrand() % (n - i) + i;
        list.swap(i, r);
    }
}

// lua interpreter related
lua_State *CreateLuaState();
void DoLuaScript(lua_State *L, const char *script);
void DoLuaScripts(lua_State *L, const QStringList &scripts);

QVariant GetValueFromLuaState(lua_State *L, const char *table_name, const char *key);

class InfoRows: public QStringList{
public:
    InfoRows &add(const QString &key, const QString &value, bool shouldTranslate = false);

    QString toTableString() const{
        return QString("<table>%1</table>").arg(join(""));
    }
};

QString CreateLinkString(const QString &href, const QString &name);
QString CreateImgString(const QString &link);

QPixmap *GetMagatama(int index);
QPixmap *GetSmallMagatama(int index);

#endif // UTIL_H
