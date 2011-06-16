template <class T>
class QList{
public:
	QList();
	~QList();
	int length() const;
	void append(const T &elem);
	void prepend(const T &elem);
	bool isEmpty() const;
	T first() const;
	T last() const;
	void removeAt(int i);
};

%extend QList{
	T at(int i) const{
		return $self->value(i);
	}
}

%template(SPlayerList) QList<ServerPlayer *>;
%template(CardList) QList<const Card *>;
%template(IntList) QList<int>;

