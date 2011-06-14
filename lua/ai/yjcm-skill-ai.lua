--xianzhen

local xianzhen_skill={}
xianzhen_skill.name="xianzhen"
table.insert(sgs.ai_skills,xianzhen_skill)
xianzhen_skill.getTurnUseCard=function(self)
    
    if self.xianzhen_used then 
        local card_str = "@XianzhenSlashCard=."
	    local card = sgs.Card_Parse(card_str)
	    return card
	end
    
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
    
    local max_card = self:getMaxCard()
    if not max_card then return nil end
	local max_point = max_card:getNumber()
	
	local slashNum=self:getSlashNumber(self.player)
	if max_card:inherits("Slash") then slashNum=slashNum-1 end
	
	if slashNum<2 then return nil end
	
	self:sort(self.enemies, "hp")
	
	for _, enemy in ipairs(self.enemies) do
	
	    local enemy_max_card = self:getMaxCard(enemy)
		if enemy_max_card and max_point > enemy_max_card:getNumber() then
		    
		    local slash=self:getSlash()
		    local dummy_use={}
            dummy_use.isDummy=true
            
            local no_distance=true
		    self:useBasicCard(slash,dummy_use,no_distance)
		    
		    if dummy_use.card then 
		        
		    
		        local card_id = max_card:getEffectiveId()
			    local card_str = "@XianzhenCard=" .. card_id
			    local card = sgs.Card_Parse(card_str)

		        return card
		    end
		end
	end
	
end

sgs.ai_skill_use_func["XianzhenSlashCard"]=function(card,use,self)
	local target = self.player:getTag("XianzhenTarget"):toPlayer() 
    if self:getSlash() and not target:isDead() and not (target:hasSkill("kongcheng") and target:isKongcheng()) then
        use.card=card
    end
end

sgs.ai_skill_use_func["XianzhenCard"]=function(card,use,self)

	self:sort(self.enemies, "handcard")
	local max_card = self:getMaxCard(self.player)
	local max_point = max_card:getNumber()
	
	for _, enemy in ipairs(self.enemies) do
	    local enemy_max_card = self:getMaxCard(enemy)
		if not (enemy:hasSkill("kongcheng") and enemy:getHandcardNum() == 1) 
			and (enemy_max_card and max_point > enemy_max_card:getNumber()) then
		    if use.to then 
		        self.xianzhen_used = true
		        use.to:append(enemy)
		        
            end
            use.card=card
            break
		end
	end
end

local masu_ai = SmartAI:newSubclass "masu"
--masu_ai:setOnceSkill("masu")
function masu_ai:activate(use)
--local xinzhan_skill={}
--xinzhan_skill.name="xinzhan"
--table.insert(sgs.ai_skills,xinzhan_skill)
--xinzhan_skill.getTurnUseCard=function(self,inclusive)
	self:log("get")
	if not self.player:hasUsed("XinzhanCard") and self.player:getHandcardNum() > self.player:getHp() then
		if use.to then 
			use.card = sgs.Card_Parse("@XinzhanCard=.") 
		return 
		end
	end
	super.activate(self, use)
end

--sgs.ai_skill_use_func["XinzhanCard"]=function(card,use,self)
--		use.card=card
--end
