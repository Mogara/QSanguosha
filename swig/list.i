template <class T>
class QList{
public:
	QList();
	~QList();
	int length() const;
	T at(int i) const;
	void append(const T &elem);
	void prepend(const T &elem);
	bool isEmpty() const;
	T first() const;
	T last() const;
	void removeAt(int i) const;
};

%template(SPlayerList) QList<ServerPlayer *>;
%template(CardList) QList<const Card *>;
%template(IntList) QList<int>;
