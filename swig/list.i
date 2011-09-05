template <class T>
class QList{
public:
	QList();
	~QList();
	int length() const;
	void append(const T &elem);
	void prepend(const T &elem);
	bool isEmpty() const;
	bool contains ( const T & value ) const;
	T first() const;
	T last() const;
	void removeAt(int i);
	bool removeOne ( const T & value );
};

%extend QList{
	T at(int i) const{
		return $self->value(i);
	}
}

%template(SPlayerList) QList<ServerPlayer *>;
%template(PlayerList)  QList<const Player *>;
%template(CardList) QList<const Card *>;
%template(IntList) QList<int>;
%template(SkillList) QList<const Skill *>;
%template(ItemList) QList<CardItem *>;