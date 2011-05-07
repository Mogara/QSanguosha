function SmartAI:getCurrentState()
    local state={}
    local players=self.room:getAllPlayers()
    players=sgs.QList2Table(players)

    for _,player in ipairs(players) do
        state[player:objectName()]=self:getPlayerInfo(player)
    end

    state["chance"]=1

    state["current"]=self.room:getCurrent():objectName()

    return state
end

sgs.ai_skill_index={}

function SmartAI:getPlayerInfo(player)
    local playerInfo={}

    playerInfo["hand"]=player:getHandcardNum()

    playerInfo["life"]=player:getHp()

    playerInfo["face"]=player:faceUp()

    playerInfo["chain"]=player:isChained()

    playerInfo["equip"]=sgs.QList2Table(player:getEquips())

    playerInfo["skill"]={}
    for _,index in ipairs(sgs.ai_skill_index) do
        if player:hasSkill(index) then
            table.insert(playerInfo["skill"],index)
        end
    end
    return playerInfo
end

function SmartAI:evaluatePlayer(player)

    local players=self.room:getAllPlayers()
    players=sgs.QList2Table(players)

    player["slash_cover"]={}
    for _,aplayer in ipairs(players) do
        if player:canSlash(aplayer) then table.insert(player["slash_cover"],aplayer:objectName()) end
    end

    player["slash_covered"]={}
    for _,aplayer in ipairs(players) do
        if aplayer:canSlash(player) then table.insert(player["slash_covered"],aplayer:objectName()) end
    end

    player["nexting"]={}
    for _,aplayer in ipairs(players) do
        if player:distanceTo(aplayer)==1 then table.insert(player["nexting"],aplayer:objectName())end
    end

    player["nexted"]={}
    for _,aplayer in ipairs(players) do
        if aplayer:distanceTo(player)==1 then table.insert(player["nexted"],aplayer:objectName())end
    end

    player["strength"]=player.life+player.hand
    player["potential"]=#player.slash_cover
    player["danger"]=#player.slash_covered
end

function SmartAI:getMoves()
    local cards=self.player:getHandcards()
    cards=sgs.QList2Table(cards)
    local moves={}
    for _,card in ipairs(cards) do

        local type = card:getTypeId()
        if type == sgs.Card_Equip then
            sgs.ai_play["Equip"](self.player,card,moves)
        elseif sgs.ai_play[card:className()] then
            sgs.ai_play[card:className()](self.player,card,moves)
        end
    end
    local selfplayer=self:getPlayerInfo(self.player)
    for _,skillName in ipairs(selfplayer) do
        if sgs.ai_skillUse[skillname] then
            sgs.ai_skillUse[skillname](self.player,moves)
        end
    end
    return moves
end

function SmartAI:printMoves(moves)
    self.room:output("moves------------")
    for _,move in ipairs(moves) do
        local from=self:getPlayer(move.from)
        for _,toName in ipairs(move.to) do
        local to=self:getPlayer(toName)
        self.room:output(move.card:className().." "..
                         from:getGeneralName().." "..
                         to:getGeneralName().." "..
                         (move.target or ""))

        end
    end
    self.room:output("endofmoves--------")
end

function SmartAI:getPlayer(objectName)
    local players=self.room:getAllPlayers()
    players=sgs.QList2Table(players)
    for _,player in ipairs(players) do
        if player:objectName()==objectName then return player end
    end
    return nil
end

function SmartAI:getPlayerByGeneral(general)
    local players=self.room:getAllPlayers()
    players=sgs.QList2Table(players)
    for _,player in ipairs(players) do
        if player:getGeneralName()==general then return player end
    end
    return nil
end    
    
function SmartAI:log(outString)
    self.room:output(outString)
end

function SmartAI:predictState(state,move)
    local score=0

    local antiDecisions=triggeredDecisions(state,move)
    local newState=applyMove(state,move)
    if not antiDecisions then return evaluateState(newState) end

    for _,decision in ipairs(antiDecisions) do
        --newState.chance=antimove.chance or newState.chance
        --local potentials=predictState(newState,antimove)
        local potential=0
        for _,choice in ipairs(decision) do
            newState.chance=newState.chance/(#decision)
            potential=potential+predictState(newState,choice)
        end
    end

    return potentialStates
end

function SmartAI:triggeredDecisions(state,move)
    local decisions={}
    if move.name=="Slash" then
        local decision={}
        local takeDamage=self:moveStruct(self:getPlayer(move.from),move.card,move.to,"damage")
        local playJink=self:moveStruct(self:getPlayer(move.from),nil,move.to,"Jink")
        table.insert(decision,takeDamage)
        table.insert(decision,playJink)
        table.insert(decisions,decision)
    end
    return decisions
end

function SmartAI:evaluateState(state)
    local score=0
    local players=self.room:getAllPlayers()
    players=sgs.QList2Table(players)

    for _,player in ipairs(players) do
        local name=player:objectName()
        evaluatePlayer(state[name])

        if self:isFriend(player) then
            score=score+state[name][strength]
        else
            score=score-state[name][strength]
        end
    end
    return score*state.chance
end

function SmartAI:copyState(oldstate)
    local state={}
    local players=self.room:getAllPlayers()
    players=sgs.QList2Table(players)

    for _,player in ipairs(players) do
        state[player:objectName()]=oldstate[player:objectName()]
    end

    state["chance"]=oldstate["chance"]

    state["current"]=oldstate["current"]
    return state
end
