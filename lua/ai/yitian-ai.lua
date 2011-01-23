-- fanji
sgs.ai_skill_invoke.fanji = true

-- danlao
sgs.ai_skill_invoke.danlao = true

-- lianli
sgs.ai_skill_use["@lianli"] = function(self, prompt)
	self:sort(self.friends)
	
	for _, friend in ipairs(self.friends) do
		if friend:getGeneral():isMale() then
			return "@LianliCard=.->" .. friend:objectName()
		end
	end
	
	return "."	
end

sgs.ai_skill_invoke.tongxin = true