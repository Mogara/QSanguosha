sgs.ai_card_intention={}
sgs.ai_carduse_intention={}
sgs.ai_assumed["rebel"]=0
sgs.ai_assumed["loyalist"]=0
sgs.ai_assumed["renegade"]=0
sgs.ai_renegade_suspect={}
sgs.ai_anti_lord={}
sgs.ai_lord_tolerance={}

sgs.ai_card_intention["general"]=function(to,level)
    if to:isLord() then
        return -level*2
    elseif sgs.ai_explicit[to:objectName()]=="loyalist" then
        return -level
    elseif sgs.ai_explicit[to:objectName()]=="loyalish" then
        return -level
    elseif sgs.ai_explicit[to:objectName()]=="rebel" then
        return level
    elseif sgs.ai_explicit[to:objectName()]=="rebelish" then
        return level
    else
        local nonloyals=sgs.ai_assumed["rebel"]--+sgs.ai_assumed["renegade"]
        local loyals=sgs.ai_assumed["loyalist"]
        if loyals+nonloyals<=1 then return 0 end
        
        local ratio
        if loyals<=0 then ratio=1
        elseif nonloyals<=0 then ratio =-1 
        
        else
             local ratio1=(-loyals+nonloyals-1)/(loyals+nonloyals)
             local ratio2=(-loyals+nonloyals+1)/(loyals+nonloyals)
             ratio=1-math.sqrt((1-ratio1)*(1-ratio2))
             --if ratio1*ratio1>ratio2*ratio2 then ratio=ratio1
             --else ratio=ratio2 end
             --ratio=ratio
        end
        
        --if level==80 then to:getRoom():output(ratio) end
        return level*ratio
    end
end

sgs.ai_carduse_intention["Indulgence"]=function(card,from,to,source)
    return sgs.ai_card_intention.general(to,120)
end

sgs.ai_carduse_intention["SupplyShortage"]=function(card,from,to,source)
    return sgs.ai_card_intention.general(to,120)
end

sgs.ai_card_intention["Slash"]=function(card,from,to,source)
    if sgs.ai_liuliEffect then
        sgs.ai_liuliEffect=false
        return 0
    end
    local modifier=0
    if sgs.ai_collateral then sgs.ai_collateral=false modifier=-40 end
    return sgs.ai_card_intention.general(to,80+modifier)
end

sgs.ai_card_intention["FireSlash"]=function(card,from,to,source)
    if sgs.ai_liuliEffect then
        sgs.ai_liuliEffect=false
        return 0
    end
    local modifier=0
    if sgs.ai_collateral then sgs.ai_collateral=false modifier=-40 end
    return sgs.ai_card_intention.general(to,80+modifier)
end

sgs.ai_card_intention["ThunderSlash"]=function(card,from,to,source)
    if sgs.ai_liuliEffect then
        sgs.ai_liuliEffect=false
        return 0
    end
    local modifier=0
    if sgs.ai_collateral then sgs.ai_collateral=false modifier=-40 end
    return sgs.ai_card_intention.general(to,80+modifier)
end

sgs.ai_card_intention["Peach"]=function(card,from,to,source)
        return sgs.ai_card_intention.general(to,-80)
end

sgs.ai_card_intention["Duel"]=function(card,from,to,source)
    if sgs.ai_lijian_effect then 
        sgs.ai_lijian_effect=false
        return 0 
    end
    return sgs.ai_card_intention.general(to,80)
end

sgs.ai_card_intention["Collateral"]=function(card,from,to,source)
    sgs.ai_collateral=true
    return sgs.ai_card_intention.general(to,80)
end

sgs.ai_card_intention["FireAttack"]=function(card,from,to,source)
    return sgs.ai_card_intention.general(to,80)
end

sgs.ai_card_intention["IronChain"]=function(card,from,to,source)
    --to:getRoom():output(to:isChained())
    if not to:isChained() then
        return sgs.ai_card_intention.general(to,80)
    else return sgs.ai_card_intention.general(to,-80)
    end
end

sgs.ai_card_intention["ArcheryAttack"]=function(card,from,to,source)
        --return sgs.ai_card_intention.general(to,40)
        return 0
end

sgs.ai_card_intention["SavageAssault"]=function(card,from,to,source)
        --return sgs.ai_card_intention.general(to,40)
        return 0
end

sgs.ai_card_intention["AmazingGrace"]=function(card,from,to,source)
        --return sgs.ai_card_intention.general(to,-20)
        return 0
end

sgs.ai_card_intention["Dismantlement"]=function(card,from,to,source)
        if to:containsTrick("indulgence") or to:containsTrick("supply_shortage") then 
            sgs.ai_snat_disma_effect=true
            sgs.ai_snat_dism_from=from
            return 0 
        end
        return sgs.ai_card_intention.general(to,70)
end

sgs.ai_card_intention["Snatch"]=function(card,from,to,source)
        if to:containsTrick("indulgence") or to:containsTrick("supply_shortage") then 
            sgs.ai_snat_disma_effect=true
            sgs.ai_snat_dism_from=from
            return 0 
        end
        return sgs.ai_card_intention.general(to,70)
end

sgs.ai_card_intention["TuxiCard"]=function(card,from,to,source)
--        from:getRoom():output("a TuxiCard")
        return sgs.ai_card_intention.general(to,80)
end

sgs.ai_carduse_intention["LeijiCard"]=function(card,from,to,source)
--        from:getRoom():output("a LeijiCard")
        return sgs.ai_card_intention.general(to,80)
end

sgs.ai_carduse_intention["RendeCard"]=function(card,from,to,source)
--        from:getRoom():output("a RendeCard")
        return sgs.ai_card_intention.general(to,-70)
end

sgs.ai_carduse_intention["QingnangCard"]=function(card,from,to,source)
--        from:getRoom():output("a QingnangCard")
        return sgs.ai_card_intention.general(to,-100)
end

sgs.ai_card_intention["ShensuCard"]=function(card,from,to,source)
--        from:getRoom():output("a ShensuCard")
        return sgs.ai_card_intention.general(to,80)
end

sgs.ai_card_intention["QiangxiCard"]=function(card,from,to,source)
--        from:getRoom():output("a ShensuCard")
        return sgs.ai_card_intention.general(to,80)
end

sgs.ai_carduse_intention["LijianCard"]=function(card,from,to,source)
--        from:getRoom():output("a LijianCard")
        if not sgs.ai_lijian_effect then
            sgs.ai_lijian_effect=true
            return sgs.ai_card_intention.general(to,70)
        else
            sgs.ai_lijian_effect=false
            return 0
        end
end

sgs.ai_carduse_intention["JieyinCard"]=function(card,from,to,source)
--        from:getRoom():output("a JieyinCard")
        return sgs.ai_card_intention.general(to,-80)
end

sgs.ai_carduse_intention["HuangtianCard"]=function(card,from,to,source)
        sgs.ai_lord_tolerance[from:objectName()]=(sgs.ai_lord_tolerance[from:objectName()] or 0)+1
--        from:getRoom():output("a JieyinCard")
        return sgs.ai_card_intention.general(to,-80)
end

sgs.ai_carduse_intention["JiemingCard"]=function(card,from,to,source)
--        from:getRoom():output("a JieyinCard")
        return sgs.ai_card_intention.general(to,-80)
end

sgs.ai_carduse_intention["HaoshiCard"]=function(card,from,to,source)
--        from:getRoom():output("a HaoCard")
        return sgs.ai_card_intention.general(to,-80)
end

sgs.ai_carduse_intention["FanjianCard"]=function(card,from,to,source)
--        from:getRoom():output("a FanjianCard")
        return sgs.ai_card_intention.general(to,70)
end

sgs.ai_carduse_intention["TianyiCard"]=function(card,from,to,source)
--        from:getRoom():output("a FanjianCard")
        return sgs.ai_card_intention.general(to,70)
end

sgs.ai_carduse_intention["QuhuCard"]=function(card,from,to,source)
--        from:getRoom():output("a FanjianCard")
        return sgs.ai_card_intention.general(to,70)
end

sgs.ai_carduse_intention["LiuliCard"]=function(card,from,to,source)
--        from:getRoom():output("a LiuliCard")
        sgs.ai_liuliEffect=true
        return sgs.ai_card_intention.general(to,70)
end

sgs.ai_explicit={}
sgs.ai_royalty={}
sgs.ai_offensive_card=
{
    Slash=true,
    ThunderSlash=true,
    FireSlash=true,
    Duel=true,
    Collateral=true,
    Slash=true,
    FireAttack=true,
    Indulgence=true,
    SupplyShortage=true,
    IronChain=true,
}

sgs.ai_ambiguous_card=
{
   Dismantlement=true,
   Snatch=true,
   AmazingGrace=true,
   ArcheryAttack=true,
   SavageAssault=true,

}


function SmartAI:updateRoyalty(player)
end

function SmartAI:printRoyalty()
        player=self.player
        self.room:output(player:getGeneralName().." "..sgs.ai_royalty[player:objectName()].." "..(sgs.ai_explicit[player:objectName()] or " "))
end

function SmartAI:refreshRoyalty(player,intention)
    if player:isLord() then return end
    local name=player:objectName()

        if (intention>=70) or (intention<=-70) then
            if sgs.ai_royalty[name]*intention<0 then
            sgs.ai_royalty[name]=sgs.ai_royalty[name]/2
            self:refreshRoyalty(player,0)
            sgs.ai_renegade_suspect[name]=(sgs.ai_renegade_suspect[name] or 0)+1
            end
        end
        
        if ((sgs.ai_anti_lord[name] or 0)-2)>(sgs.ai_lord_tolerance[name] or 0) then 
            if intention>0 then intention=intention/5 end
        end
        sgs.ai_royalty[name]=sgs.ai_royalty[name]+intention
        
        if sgs.ai_explicit[name]=="loyalish" then
            sgs.ai_assumed["loyalist"]=sgs.ai_assumed["loyalist"]+1
        elseif sgs.ai_explicit[name]=="loyalist" then
            sgs.ai_assumed["loyalist"]=sgs.ai_assumed["loyalist"]+1
        elseif sgs.ai_explicit[name]=="rebelish" then
            sgs.ai_assumed["rebel"]=sgs.ai_assumed["rebel"]+1
        elseif sgs.ai_explicit[name]=="rebel" then
            sgs.ai_assumed["rebel"]=sgs.ai_assumed["rebel"]+1
        end
        sgs.ai_explicit[name]=nil
        
    if sgs.ai_royalty[name]<=-160 then
        if not sgs.ai_explicit[name] then sgs.ai_assumed["rebel"]=sgs.ai_assumed["rebel"]-1 end
        sgs.ai_explicit[name]="rebel"
        sgs.ai_royalty[name]=-160
    elseif sgs.ai_royalty[name]<=-70 then
        if not sgs.ai_explicit[name] then sgs.ai_assumed["rebel"]=sgs.ai_assumed["rebel"]-1 end
        sgs.ai_explicit[name]="rebelish"
    elseif sgs.ai_royalty[name]>=160 then
        if not sgs.ai_explicit[name] then sgs.ai_assumed["loyalist"]=sgs.ai_assumed["loyalist"]-1 end
        sgs.ai_explicit[name]="loyalist"
        sgs.ai_royalty[name]=160
    elseif sgs.ai_royalty[name]>=70 then
        if not sgs.ai_explicit[name] then sgs.ai_assumed["loyalist"]=sgs.ai_assumed["loyalist"]-1 end
        sgs.ai_explicit[name]="loyalish"
    elseif sgs.ai_explicit[name] then
    end
end

function SmartAI:printAssume()
	self.room:output(sgs.ai_assumed["rebel"])
	self.room:output(sgs.ai_assumed["loyalist"])
	self.room:output("----")
end

function SmartAI:singleRole()
    local roles=0
    local theRole
    local selfexp=sgs.ai_explicit[self.player:objectName()]
    if selfexp=="loyalish" then selfexp="loyalist"
    elseif selfexp=="rebelish" then selfexp="rebel"
    end
    local selftru=self.role
    
    if (self.role~="lord") and (self.role~="renegade") then sgs.ai_assumed[self.role]=sgs.ai_assumed[self.role]-1 end
    if selfexp then sgs.ai_assumed[selfexp]=(sgs.ai_assumed[selfexp] or 0)+1 end
    	
    
    if sgs.ai_assumed["rebel"]>0 then
        roles=roles+1
        theRole="rebel"
    end
    if sgs.ai_assumed["loyalist"]>-1 then
        roles=roles+1
        theRole="loyalist"
    end
    
    if (self.role~="lord") and (self.role~="renegade") then sgs.ai_assumed[self.role]=sgs.ai_assumed[self.role]+1 end
    if selfexp then sgs.ai_assumed[selfexp]=sgs.ai_assumed[selfexp]-1 end
    
    
    if roles==1 then
        if sgs.ai_assumed["loyalist"]==sgs.ai_assumed["rebel"] then return nil end
        return theRole
    end
    return nil
end

function SmartAI:getInflictTargets()
    
end

function SmartAI:getProtectTarget()
    
end

function SmartAI:getSupportTarget()
    
end

function SmartAI:getWeakenTarget()
    
end

function SmartAI:printCards(cards)
    local string=""
    for _,card in ipairs(cards) do
        string=string.." "..card:className()
    end
    self.room:output(string)
end