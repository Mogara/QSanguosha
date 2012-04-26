#ifndef UTIL_H
#define UTIL_H

struct lua_State;
class QVariant;

#include <QList>

template<typename T>
void qShuffle(QList<T> &list){
    int i, n = list.length();
    for(i=0; i<n; i++){
        int r = qrand() % (n - i) + i;
        list.swap(i, r);
    }
}

QVariant GetValueFromLuaState(lua_State *L, const char *table_name, const char *key);

#endif // UTIL_H
