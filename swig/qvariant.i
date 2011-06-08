class QVariant{
public:
	QVariant();
	QVariant(int);
	QVariant(const char *);
	QVariant(bool);
	int toInt() const;
	QString toString() const;
	bool toBool() const;
};

%extend QVariant {

	void setValue(int value){
		$self->setValue(QVariant::fromValue(value));
	}

	DamageStruct toDamage() const{
		return $self->value<DamageStruct>();
	}
	
	void setValue(DamageStruct *damage){
		$self->setValue(QVariant::fromValue(*damage));
	}

	CardEffectStruct toCardEffect() const{
		return $self->value<CardEffectStruct>();
	}
	
	void setValue(CardEffectStruct *effect){
		$self->setValue(QVariant::fromValue(*effect));
	}
	
	SlashEffectStruct toSlashEffect() const{
		return $self->value<SlashEffectStruct>();
	}
	
	void setValue(SlashEffectStruct *effect){
		$self->setValue(QVariant::fromValue(*effect));
	}
	
	CardUseStruct toCardUse() const{
		return $self->value<CardUseStruct>();
	}
	
	void setValue(CardUseStruct *use){
		$self->setValue(QVariant::fromValue(*use));
	}
	
	CardMoveStruct toCardMove() const{
		return $self->value<CardMoveStruct>();
	}
	
	void setValue(CardMoveStruct *move){
		$self->setValue(QVariant::fromValue(*move));
	}
	
	const Card *toCard() const{
		return $self->value<CardStar>();
	}
	
	void setValue(const Card *card){
		$self->setValue(QVariant::fromValue(card));
	}
	
	ServerPlayer *toPlayer() const{
		return $self->value<PlayerStar>();
	}
	
	void setValue(ServerPlayer *player){
		$self->setValue(QVariant::fromValue(player));
	}
	
	DyingStruct toDying() const{
		return $self->value<DyingStruct>();
	}
	
	void setValue(DyingStruct *dying){
		$self->setValue(QVariant::fromValue(*dying));
	}

	DamageStar toDamageStar() const{
		return $self->value<DamageStar>();
	}

	void setValue(RecoverStruct *recover){
		$self->setValue(QVariant::fromValue(*recover));
	}

	RecoverStruct toRecover() const{
		return $self->value<RecoverStruct>();
	}

	JudgeStruct *toJudge() const{
		return $self->value<JudgeStar>();
	}
};