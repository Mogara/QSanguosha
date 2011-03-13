template <class T>
class QList{
public:
	QList();
	~QList();
	int length() const;
	T at(int i) const;
	void append(const T &elem);
	bool isEmpty() const;
};

%template(SPlayerList) QList<ServerPlayer *>;
%template(CardList) QList<const Card *>;

