    -- this script file contains the AI classes for gods

-- guixin, always invoke
sgs.ai_skill_invoke.guixin = true

-- shelie
sgs.ai_skill_invoke.shelie = true

local gongxin_skill={}
gongxin_skill.name="gongxin"
table.insert(sgs.ai_skills,gongxin_skill)
gongxin_skill.getTurnUseCard=function(self)
		local card_str = ("@GongxinCard=.")
		local gongxin_card = sgs.Card_Parse(card_str)
		assert(gongxin_card)
        return gongxin_card
end

sgs.ai_skill_use_func["GongxinCard"]=function(card,use,self)
    if self.player:usedTimes("GongxinCard")>0 then return end
    self:sort(self.enemies,"handcard")
    
    for _,enemy in ipairs(self.enemies) do  
        local cards = enemy:getHandcards()
			for _, acard in sgs.qlist(cards) do				
				if acard:getSuit() == sgs.Card_Heart and not acard:inherits("Shit") then
					use.card = card
					if use.to then use.to:append(enemy) end
					return
				end
			end
    end
end

local shenlubu_ai = SmartAI:newSubclass "shenlubu"

function shenlubu_ai:useTrickCard(card, use)
	if self.player:getMark("@wrath") > 0 then
		return super.useTrickCard(self, card, use)
	end
end

sgs.ai_skill_choice.wumou = "discard"


--wushen
wushen_skill={}
wushen_skill.name="wushen"
table.insert(sgs.ai_skills,wushen_skill)
wushen_skill.getTurnUseCard=function(self)
    local cards = self.player:getCards("he")	
    cards=sgs.QList2Table(cards)
	
	local red_card
	
	self:sortByUseValue(cards,true)
	
	for _,card in ipairs(cards)  do
		if card:getSuitString()=="heart" then--and (self:getUseValue(card)<sgs.ai_use_value["Slash"]) then
			red_card = card
			break
		end
	end

	if red_card then		
		local suit = red_card:getSuitString()
    	local number = red_card:getNumberString()
		local card_id = red_card:getEffectiveId()
		local card_str = ("slash:wushen[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)
		
		assert(slash)
        
        return slash
	end
end
local shenguanyu_ai = SmartAI:newSubclass "shenguanyu"

function shenguanyu_ai:askForCard(pattern,prompt)
	local card = super.askForCard(self, pattern, prompt)
	if card then return card end
	if pattern == "slash" then
		local cards = self.player:getCards("h")
		cards=sgs.QList2Table(cards)
		self:fillSkillCards(cards)
        self:sortByUseValue(cards,true)
		for _, card in ipairs(cards) do
			if card:getSuit() == sgs.Card_Heart then
				local suit = card:getSuitString()
				local number = card:getNumberString()
				local card_id = card:getEffectiveId()
				return ("slash:wushen[%s:%s]=%d"):format(suit, number, card_id)
			end
		end
	end
    
end
