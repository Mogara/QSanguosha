--技能详解3：TriggerSkill 触发技

--许多技能都有“在xx时机，如果xx条件满足，那么执行xx效果”这样的描述。
--由于游戏技时机繁多，许多技能也都是相互类似的，我们在游戏的cpp部分有种类繁多的触发技定义方式。
--而基本的（也是万能的）触发技定义在Lua中是使用sgs.CreateTriggerSkill方法，该方法可以在lua\sgs_ex.lua中找到。

--CreateTriggerSkill需要以下参数：

--name, relate_to_place, can_preshow, frequency, limit_mark, is_battle_array, battle_array_type, events, view_as_skill, can_trigger, on_cost, on_effect, priority

--name和relate_to_place不再说明。

--can_preshow
--布尔类型，指定技能是否能预亮。
--默认值为(view_as_skill == nil)，也即如果此技能不带视为技则默认可预亮，否则默认不能预亮。

--frequency：
--Frequency枚举类型，技能的发动频率。
--执行askForSkillInvoke（询问技能发动）时，frequency会影响玩家做决定的方式。
--frequency也起到了技能分类以及用于增加技能提示显示的作用。
	--frequency可能的值有：
	--sgs.Skill_NotFrequent（非锁定技：该技能的askForSkillInvoke总是会弹出提示询问玩家是否发动）
	--sgs.Skill_Compulsory （锁定技：该技能会在显示上提示玩家这是一个锁定技能）
	--sgs.Skill_Limited （限定技：该技能会在显示上提示玩家这是一个限定技能）
	--sgs.Skill_Wake（觉醒技：该技能的默认优先度为3而不是2；该技能会在显示上提示玩家这是一个觉醒技）
--frequency的默认值为sgs.Skill_NotFrequent

--limit_mark:
--字符串类型，用于限定技，指定限定技所使用的标记名称。
--无默认值。

--is_battle_array:
--布尔类型，指示这个技能为阵法技。
--阵法技的view_as_skill要指向阵法召唤视为技，不能为空，也不能指向其他的视为技。
--阵法技需要额外设置battle_array_type这个量。

--battle_array_type:
--BattleArrayType::ArrayType枚举类型，有两种取值
--sgs.Formation：表示阵法技为队列阵法技
--sgs.Siege：表示阵法技为围攻阵法技
--无默认值，如果阵法技里没有定义该量，则会报错。

--events：
--Event枚举类型，或者一个包含Event枚举类型的lua表。代表该技能的触发时机。
--可用的Event列表请参考游戏代码中的struct.h文件。
--无默认值。

--view_as_skill：
--视为技类型，指定之后本技能将变为带视为技的触发技。
--有view_as_skill的技能，frequency不能为sgs.Skill_Wake
--界面上的按钮将不再指向触发技，而是指向本视为技
--阵法技里如果没有定义该量，则会报错

--can_trigger:
--lua函数，返回两个值，第一个用加号分割的技能名的字符串，第二个是技能的发动者（也就是源码里可以改变的那个ask_who）。
--关于第一个返回值，这里的用途是这样，比如姜维的遗志，可以返回"guanxing"来使遗志发动观星而不用在遗志里把观星整个重写一遍。
--对于遗计等等技能，如果一次受到两点伤害，可以返回"yiji+yiji"，这样的话，可以发动两次遗计。神智忘隙节命同理。
--第二个返回值，用于技能触发者和技能发动者不一致的情况，比如类似骁果的技能
--对于这种情况，用room:findPlayerBySkillName(self:objectName())找到技能的发动者，最后返回，类似这样：
local yuejin = room:findPlayerBySkillName("xiaoguo")
if yuejin and yuejin:isAlive() then
	return "xiaoguo", yuejin
end
--第二个返回值可以省略
--传入参数为self(技能对象),event(触发事件),room(房间对象),player(技能的触发者),data(事件数据)
--默认条件为“具有本技能并且存活，并且是技能触发者只发动一次本技能”
--在这里和身份局技能不同，国战要求把所有关于技能判断的部分全放到这里，还有一些锁定效果也要放到里面
--比如克己的判断出牌阶段是否使用或打出杀，要在CardUsed和CardResponded事件触发技的can_trigger里设置flag

--on_cost:
--lua函数，返回布尔值。执行技能的询问发动以及技能消耗。
--传入参数为：self,event,room,player,data，与can_trigger的参数名对应的意义一致
--默认值为返回true，而这个效果肯定是对于大多数技能来讲是不能用的，所以大家就尽量更改这个地方吧

--on_effect:
--lua函数，返回布尔值，执行技能的效果。
--有些防止部分事件发生的需要在本函数当中返回true，需要注意。
--传入参数为：self,event,room,player,data，与on_cost一致。
--如果需要区分不同的事件执行不同效果，请根据event参数使用条件语句。
--通常需要将事件数据(data)转为具体的游戏结构对象才能进行操作。你可以在源码的swig/qvariant.i文件中看到定义。
--无默认值。

--priority:
--整数值，代表本技能的优先度。
--如果本技能与其他技能（或规则）在同一个时机都触发，那么优先度影响这些技能或规则的执行顺序。
--优先度更大的技能（或规则）优先执行。游戏规则的优先度为0，典型的技能优先度为2
--觉醒技的优先度默认为3，其他情况下默认为2

-- **实例：

--以下是曹操奸雄的实现：

LuaJianxiong = sgs.CreateTriggerSkill{
	name = "LuaJianxiong" ,
	events = {sgs.Damaged} ,
	can_trigger = function(self, event, room, player, data)
		if table.contains(self:TriggerSkillTriggerable(event, room, player, data, player), self:objectName()) then
			local damage = data:toDamage() --这步通常是必要的。我们需要将data对象转成对应的数据类型来得到相应的信息。
			local card = damage.card
			return (card and (room:getCardPlace(card:getEffectiveId()) == sgs.Player_PlaceTable)) and self:objectName() or ""
			--注意LUA的and和or有返回值的作用，(x and a or b) 这条语句相当于如果x为真，返回a，否则返回b
		end
		return ""
	end ,
	on_cost = function(self, event, room, player, data)
		if player:askForSkillInvoke(self:objectName(), data) then --询问技能发动
			room:broadcastSkillInvoke(self:objectName()) --播放配音
			return true --表示技能已经消耗完成
			
			--对于一些有消耗的触发技，比如神智，就要把弃置手牌放进来
		end
		return false --表示没有执行消耗，技能不执行
	end ,
	on_effect = function(self, event, room, player, data)
		local damage = data:toDamage()
		player:obtainCard(damage.card) --获得该牌，没得说
		return false --表示运行流程不发生某种中断
		
		--一般来讲on_effect里最后一定要返回一个false，虽然不返回false的话返回空值之后被C++当成false来对待，但是我们要养成良好的
		--return false习惯，避免转型C++之后总忘。C++对于不返回值的函数会当作return true处理哦~
	end ,
}

--data参数是一个QVariant，根据不同的事件我们需要用不同的方法得到它原本的数据类型。
--对于Damaged事件（你受到了伤害），data对象的类型是DamageStruct，我们使用toDamage()得到DamageStruct。

--询问技能发动时，需要使用Room对象或者ServerPlayer的askForSkillInvoke方法。
--BroadcastSkillInvoke方法则可以播放技能的发动效果。（但是对技能发动效果本身没有影响）

--player:obtainCard(damage.card) 即让player得到造成伤害的card。

--在“某个阶段开始时可触发”的技能，或者“摸牌时改为xx”这样的技能，可以使用EventPhaseStart事件来触发，并对TriggerEvent对象进行判断进行触发控制。

--对于在复数个时机发动的触发技，我们需要使用条件语句。

--以下是李典“忘隙”技能的实现代码：

LuaWangxi = sgs.CreateTriggerSkill{
	name = "LuaWangxi" ,
	events = {sgs.Damage, sgs.Damaged} ,
	can_trigger = function(self, event, room, player, data)
		if not table.contains(self:TriggerSkillTriggerable(event, room, player, data, player), self:objectName()) then return "" end
		local damage = data:toDamage()
		local target = nil
		if event == sgs.Damage then
			target = damage.to
		else
			target = damage.from
		end
		if (not target) or (not target:isAlive()) or (target:objectName() == player:objectName()) then return "" end
				--注意LUA的指针不能直接使用相等比较，会返回false的哦
		
		local trigger_list = {}  --LUA表，在LUA里非常常用的一个数据结构，基本万能
		for i = 1, damage.damage, 1 do
			table.insert(trigger_list, self:objectName()) --在LUA表中插入一个元素
		end
		return table.concat(trigger_list, "+") --table.concat用于所有元素均为字符串的情况，返回字符串的连接，中间用由第二个参数指定的字符分割
	end ,
	on_cost = function(self, event, room, player, data)
		local damage = data:toDamage()
		local target = nil
		if event == sgs.Damage then
			target = damage.to
		else
			target = damage.from
		end
		local _data = sgs.QVariant() --QVariant类型，神杀的所有data，所有Property，和所有Tag全都用到的类型。
									--注意这里不能像C++一样所有的都用静态函数fromValue来初始化的
		_data:setValue(target) --在LUA中用setValue给QVariant赋值
		
		if player:askForSkillInvoke(self:objectName(), _data) then
		--特别注意这个_data传入，大部分情况下用于AI。如果这个参数为ServerPlayer类型，则会在界面上提示发动目标
		--如果这个参数为字符串，则该函数不提示发动技能，而是出现一个提示框，内容由你自己去翻译。
			room:broadcastSkillInvoke(self:objectName())
			return true
		end
	end ,
	on_effect = function(self, event, room, player, data)
		local damage = data:toDamage()
		local target = nil
		if event == sgs.Damage then
			target = damage.to
		else
			target = damage.from
		end
		
		local players = sgs.SPlayerList() --服务器玩家QList，非常有用的结构体
		players:append(target)
		players:append(player) --QList类型使用方法append来添加成员
		room:sortByActionOrder(players) --room的函数，具体查看sanguosha.i可以看到，用来给玩家排序的
		room:drawCards(players, 1, self:objectName()) --执行摸牌，不解释
		return false
	end ,
}

--sgs.CreateTriggerSkill有4个远房亲戚：他们是：
sgs.CreateMasochismSkill(spec) --创建卖血技能，时机为sgs.Damaged
sgs.CreateGameStartSkill(spec) --创建游戏开始时技能，时机为sgs.GameStart
sgs.CreateDrawCardsSkill(spec) --创建摸牌技能，时机为sgs.DrawNCards
sgs.CreatePhaseChangeSkill(spec) --创建阶段转换（其实是阶段开始）技能，时机为sgs.EventPhaseStart
--这些技能的on_effect均被代替，具体为：
sgs.CreateMasochismSkill(spec)
--on_effect被代替为on_damaged，参数为：
--self(技能本身), player(受到伤害的ServerPlayer), damage(伤害结构体)
--不接受返回值
sgs.CreateGameStartSkill(spec)
--on_effect被代替为on_gamestart，参数为：
--self(技能本身), player(触发的ServerPlayer)
--不接受返回值
sgs.CreateDrawCardsSkill(spec)
--on_effect被代替为draw_num_func，参数为：
--self(技能本身), player(摸牌的ServerPlayer), n(之前的摸牌数)
--返回值为整型，代表修正之后的摸牌数
sgs.CreatePhaseChangeSkill(spec)
--on_effect被代替为on_phasechange，参数为：
--self(技能本身), player(触发的ServerPlayer)
--返回值为布尔型，与on_effect相同

