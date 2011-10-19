sgs.ai_play={}

sgs.ai_play["Non-target"]=function(player,card,moves)
	local move={}
	move.name=card:className()
	move.from=player:objectName()
	move.to={}
	table.insert(move.to,player:objectName())
	move.card=card
	table.insert(moves,move)
end

sgs.ai_play["Slash"]=function(player,card,moves)
	local players=player:getRoom():getOtherPlayers(player)
	players=sgs.QList2Table(players)
	for _, someone in ipairs(players) do
		if player:canSlash(someone) then
			local move={}
			move.name=card:className()
			move.from=player:objectName()
			move.card=card
			move.to={}
			table.insert(move.to,someone:objectName())
			table.insert(moves,move)
		end
	end
end

sgs.ai_play["ThunderSlash"]=function(player,card,moves)
	sgs.ai_play["Slash"](player,card,moves)
end

sgs.ai_play["FireSlash"]=function(player,card,moves)
	sgs.ai_play["Slash"](player,card,moves)
end

sgs.ai_play["Equip"]=function(player,card,moves)
	sgs.ai_play["Non-target"](player,card,moves)
end

sgs.ai_play["SavageAssault"]=function(player,card,moves)
	sgs.ai_play["Non-target"](player,card,moves)
end

sgs.ai_play["ArcheryAttack"]=function(player,card,moves)
	sgs.ai_play["Non-target"](player,card,moves)
end

sgs.ai_play["AmazingGrace"]=function(player,card,moves)
	sgs.ai_play["Non-target"](player,card,moves)
end

sgs.ai_play["Analeptic"]=function(player,card,moves)
	sgs.ai_play["Non-target"](player,card,moves)
end

sgs.ai_play["Peach"]=function(player,card,moves)
	sgs.ai_play["Non-target"](player,card,moves)
end

sgs.ai_play["Lightning"]=function(player,card,moves)
	sgs.ai_play["Non-target"](player,card,moves)
end

sgs.ai_play["ExNihilo"]=function(player,card,moves)
	sgs.ai_play["Non-target"](player,card,moves)
end

sgs.ai_play["Dismantlement"]=function(player,card,moves)
	local players=player:getRoom():getOtherPlayers(player)
	players=sgs.QList2Table(players)

	for _, someone in ipairs(players) do
		if not someone:isAllNude() then

			local cards=someone:getCards("ej")
			cards=sgs.QList2Table(cards)

			for _,acard in ipairs(cards) do
				local move={}
				move.name=card:className()
				move.from=player:objectName()
				move.card=card
				move.to={}
				table.insert(move.to,someone:objectName())
				move.target=acard:className()
				table.insert(moves,move)
			end

			if not someone:isKongcheng() then
				local move={}
				move.name=card:className()
				move.from=player:objectName()
				move.card=card
				move.to={}
				table.insert(move.to,someone:objectName())
				move.target="hand"
				table.insert(moves,move)
			end
		end
	end
end

sgs.ai_play["Snatch"]=function(player,card,moves)
	local players=player:getRoom():getOtherPlayers(player)
	players=sgs.QList2Table(players)

	for _, someone in ipairs(players) do
		if (player:distanceTo(someone)==1) and (not someone:isAllNude()) then

			local cards=someone:getCards("ej")
			cards=sgs.QList2Table(cards)

			for _,acard in ipairs(cards) do
				local move={}
				move.name=card:className()
				move.from=player:objectName()
				move.card=card
				move.to={}
				table.insert(move.to,someone:objectName())
				move.target=acard:className()
				table.insert(moves,move)
			end

			if not someone:isKongcheng() then
				local move={}
				move.name=card:className()
				move.from=player:objectName()
				move.card=card
				move.to={}
				table.insert(move.to,someone:objectName())
				move.target="hand"
				table.insert(moves,move)
			end
		end

	end
end

sgs.ai_play["Duel"]=function(player,card,moves)
	local players=player:getRoom():getOtherPlayers(player)
	players=sgs.QList2Table(players)
	for _, someone in ipairs(players) do
			local move=sgs.ai_moveStruct(player,card,someone:objectName(),card:className())
			table.insert(moves,move)
	end
end

sgs.ai_play["Collateral"]=function(player,card,moves)
	local players=player:getRoom():getOtherPlayers(player)
	players=sgs.QList2Table(players)

	for _, someone in ipairs(players) do
		if someone:getWeapon() then
			local otherplayers=player:getRoom():getOtherPlayers(someone)
			otherplayers=sgs.QList2Table(otherplayers)

			for _, anotherone in ipairs(otherplayers) do
				if someone:canSlash(anotherone) then
					local move={}
					move.name=card:className()
					move.from=player:objectName()
					move.card=card
					move.to={}
					table.insert(move.to,someone:objectName())
					table.insert(move.to,anotherone:objectName())
					table.insert(moves,move)
				end
			end
		end
	end
end

sgs.ai_play["IronChain"]=function(player,card,moves)
	local players=player:getRoom():getAllPlayers()
	players=sgs.QList2Table(players)

	for _, someone in ipairs(players) do
			local otherplayers=player:getRoom():getOtherPlayers(someone)
			otherplayers=sgs.QList2Table(otherplayers)

			for _, anotherone in ipairs(otherplayers) do
					local move={}
					move.name=card:className()
					move.from=player:objectName()
					move.card=card
					move.to={}
					table.insert(move.to,someone:objectName())
					table.insert(move.to,anotherone:objectName())
					table.insert(moves,move)
			end
			local move=sgs.ai_moveStruct(player,card,someone:objectName(),card:className())
			table.insert(moves,move)
	end
	local move=sgs.ai_moveStruct(player,card,nil,card:className())
	table.insert(moves,move)
end

sgs.ai_play["FireAttack"]=function(player,card,moves)
	local players=player:getRoom():getOtherPlayers(player)
	players=sgs.QList2Table(players)
	for _, someone in ipairs(players) do
		if not player:isKongcheng() then
			local move=sgs.ai_moveStruct(player,card,someone:objectName(),card:className())
			table.insert(moves,move)
		end
	end
end

sgs.ai_moveStruct=function(player,card,to,name)
	local move={}
	move.name=name
	move.from=player:objectName()
	move.card=card
	if not to then move.to={}
	elseif type(to)=="table" then
		move.to=to
	else
		move.to={}
		table.insert(move.to,to)
	end
end
