%{

#include "serverplayer.h"

static bool CompareByHandcard(ServerPlayer *a, ServerPlayer *b){
	int n1 = a->getHandcardNum();
	int n2 = b->getHandcardNum();
	
	return n1 < n2;
}

static bool CompareByHp(ServerPlayer *a, ServerPlayer *b){
	int n1 = a->getHp();
	int n2 = b->getHp();
	
	return n1 < n2;
}

%}

class CardList{
public:
	CardList();
	~CardList();
	int length() const;
	const Card *at(int i) const;
	void append(const Card *card);
};

class SPlayerList{
public:
	SPlayerList();
	~SPlayerList();
	int length() const;
	ServerPlayer *at(int i) const;
	void append(ServerPlayer *player);
};

%extend SPlayerList{
	void sortByHandcard(){
		qSort($self->begin(), $self->end(), CompareByHandcard);
	}
	
	void sortByHp(){
		qSort($self->begin(), $self->end(), CompareByHp);
	}
};