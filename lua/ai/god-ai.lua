-- this script file contains the AI classes for gods

-- guixin, always invoke
sgs.ai_skill_invoke.guixin = true

-- shelie
sgs.ai_skill_invoke.shelie = true

local shenlumeng_ai = SmartAI:newSubclass "shenlumeng"
shenlumeng_ai:setOnceSkill("gongxin")

function shenlumeng_ai:activate(use)
    if not self.gongxin_used then
        self:sort(self.enemies, "handcard")
        
        for _, enemy in ipairs(self.enemies) do
            local cards = enemy:getHandcards()
			for _, card in sgs.qlist(cards) do				
				if card:getSuit() == sgs.Card_Heart and not card:inherits("Shit") then
					use.card = sgs.Card_Parse("@GongxinCard=.")
					use.to:append(enemy)

					self.gongxin_used = true
					return
				end
			end
        end
    end

	super.activate(self, use)
end

local shenlubu_ai = SmartAI:newSubclass "shenlubu"

function shenlubu_ai:useTrickCard(card, use)
	if self.player:getMark("@wrath") > 0 then
		return super.useTrickCard(self, card, use)
	end
end

sgs.ai_skill_choice.wumou = "discard"
