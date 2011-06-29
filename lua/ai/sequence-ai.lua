local pkg=sgs.ai_standard
pkg.basic_sequence=sgs.newStrategy{
	match=function(self,move)
		if sgs.ai_use_priority[move.card:className()] then return true
		else return false end
	end,
	evaluate=function(self,move)
		return sgs.ai_use_priority[move.card:className()]
	end,
	isSequence=true
}

pkg.basic_equip_sequence=sgs.newStrategy{
	match=function(self,move)
		return (move.card:inherits("Armor") and self.player:getArmor())  or
				(move.card:inherits("Weapon") and self.player:getWeapon()) or
				(move.card:inherits("DefensiveHorse") and self.player:getDefensiveHorse())  or
				(move.card:inherits("OffensiveHorse") and self.player:getOffensiveHorse())
	end,
	value=-2,
	override={pkg.basic_sequence},
}

pkg.xiaoji_equip_sequence=sgs.newStrategy{
	match=function(self,move)
		return move.card:inherits("EquipCard") and move.from:hasSkill("xiaoji")
	end,
	value=10,
	override={pkg.basic_sequence},
	importance=15,
	isSequence=true
}