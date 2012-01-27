--[[
大家好我是 William915。
从这个文件开始讲解AI的编写方法。

三国杀的 AI 由太阳神上与 hypercross 共同编写，之后经过他们及 donle，宇文天启和本人的发展。
目前最新的 AI 稳定版本为 V0.7，这一文档中提及到的所有行号均以该版本为基准。
在论坛上可以看到反馈 AI 的问题的帖子远比反映程序的问题的帖子要多得多，这表明了 AI 的复杂性。
熟悉 AI 发展历史的朋友一定知道有那么一段时间 AI 常常会改好了这个又坏了那个，这是因为影响 AI 表现的因素太多。
一言以蔽之，AI 编写与修改是牵一发而动全身的。

目前的 AI 架构还不完善，随着架构的逐步完善文档也会逐步的调整。而架构的完善，需要你们和我们的共同努力。

也许您已经知道在开始编写 MOD 和 LUA 扩展之前需要一定的准备，AI 也是如此，这些准备包括：
熟悉三国杀本身的源代码（知道如何去检索自己需要的东西就够了），包括sanguosha_wrap.cxx文件
熟悉 MOD 或 LUA 扩展的源代码并作适当修改（下面将具体介绍）。可以说，没有源代码作参考是很难编写 AI 的。
熟悉 smart-ai 中的各个函数的用法。（下面将具体介绍）
阅读 lua/ai 文件夹下的各个 AI 文件以对 AI 编写形成一个大致的概念。

如果您对 AI 还不熟悉，又想比较快上手，那么最好的方法是参考已有的 AI。
其实在编写 Lua 扩展的时候也是一样，照葫芦画瓢是最好的方法。

++ AI 需要做什么？
AI 所做的事情只有一件：作决定。如果您想知道有哪些地方需要编写 AI，只要想一下游戏过程中什么地方您需要作决定就行了。
已有的 AI 文件提供了大部分情况下作决定的策略，因此大家只需要把精力集中在与扩展的武将的技能相关的决定上就可以了。
对于 MOD 的编写者，当然还需要为扩展的卡牌编写相应的策略。

随着下面介绍的逐步深入，相信大家对于 AI 的这一特点会有更深的体会。

在编写 AI 之前，还要知道：现在的 AI 是基于技能的，而不是基于武将的。
因此，我们不需要给姜维觉醒后获得的观星额外写 AI，只要把诸葛亮的观星 AI 写好就行了。

++ 为 AI 而修改技能代码
实际上如果在写技能时没有为 AI 考虑的话，有一些技能的 AI 甚至根本无法编写。因为 AI 所能获得的信息是很有限的。
最常用的传递信息给 AI 的方法是通过 data 参数。例如 src/package/thicket.cpp 第 124 行附近颂威的代码：
foreach(ServerPlayer *p, players){
	QVariant who = QVariant::fromValue(p);
	if(p->hasLordSkill("songwei") && player->askForSkillInvoke("songwei", who)){
		...

对照 serverplayer.h 可以看到，askForSkillInvoke 里面的第 2 个参数就是 data，这个 data 就是给 AI 用的。
上面的代码如果写成 lua，则是这样：
]]

for _, p in sgs.qlist(players) do
	local who = sgs.QVariant()
	who:setValue(p)
	if p:hasLordSkill("songwei") and player:askForSkillInvoke("songwei", who) then
	-- ...
	end
end

--正是因为在编写技能时传入了 data，我们在 thicket-ai.lua 中才能根据 data 判断是否需要颂威（第 55 至 58 行）。

sgs.ai_skill_invoke.songwei = function(self, data)
	local who = data:toPlayer()
	return self:isFriend(who)
end

--[[
因此，在开始编写 AI 之前，请相应修改您的程序代码以便 AI 正常工作。
给 AI 传递数据的另外一个方法是通过 tag。例子可见鬼才，不再赘述。

++ 如何载入自己写的 AI？
方法很简单，只要找到您的扩展包的名字，例如为 extension。
则只要在 lua/ai 文件夹下新增一个文件 extension-ai.lua 并把相应的 AI 代码放到这一文件内即可。

对于扩展包的名字，cpp 扩展应查找形如这样的代码：ThicketPackage::ThicketPackage():Package("thicket")
而 lua 扩展则应根据第一行：]]
module("extensions.moligaloo", package.seeall) 
 
--[[上面两个例子相应的 AI 文件名分别应该为 thicket-ai.lua 和 moligaloo-ai.lua

++ 万一需要修改已有的 AI 文件？
虽然这次 AI 架构的编写力求做到对所有的扩展都不需要修改 smart-ai， 但是有一些情况可能还是需要修改已有的 AI 文件。
例如有一个像奇才一样的技能，那么在目前的版本里只能通过修改 SmartAI.getDistanceLimit 来实现，但是这并不意味着需要修改 smart-ai.lua 文件。
事实上，您只要在自己的 AI 文档里头重新写一遍 SmartAI.getDistanceLimit 就可以了。这时原来的 getDistanceLimit 会被您所写的覆盖掉。

++ Lua 基础知识
Lua 语言的基础知识可以通过查阅 manual.luaer.cn 获得。下面重点介绍一些与 AI 编写关系比较紧密的和容易混淆的 Lua 知识。
如果你还没有编写过 AI，可以先跳过这一部分。

首先要记住Lua是大小写敏感的。SmartAI跟smartai不是同一个东西。

点，冒号与方括号：
这是最容易混淆的地方之一。这三种符号都用于对 Lua 里头的表作索引。下面三种写法是等价的：]]
example:blah(foo),
example.blah(example, foo)
example["blah"](example, foo)

--[[
nil, false 与 0：
任何一个变量在初始化之前都是 nil。当一个函数没有返回任何值的时候，返回值也是 nil。C 中的 NULL 在 LUA 中被映射为 nil
false 与 0 在 lua 里头是两个不同的值。]]
if a then blah end

--[[上面的代码当 a 为 nil 和 false 的时候，blah 不会执行，但是当 a 为 0 的时候，blah 会被执行。

给熟悉 C 的朋友提个醒：
Lua 里头没有 switch，没有 continue，没有 goto，请不要在代码里使用这些关键字。
Lua 里头没有函数重载的说法，以下两种写法是等价的：]]
function blah(a, b, c) end
blah = function(a, b, c) end

--因此如果有下面的代码：
function blah(a,b,c) blahblah end
function blah(a,b) ... end

--[[则相当于给全局变量 blah 赋了两次值。结果第一行代码没有任何作用，blahblah 也不会被执行。

表（table）与列表（QList）
这是两个完全不同的类型，但是很容易混淆。Lua 所能直接处理的是前者，但是通过调用room里头的函数获得的往往是后者。
两者的转换可以通过下面代码来进行，这种转换是单向的：]]
Table = sgs.QList2Table(QList)

--[[两者的差别列出如下：
			表t					列表l
索引		t[i]				l:at(i-1)
长度		#t					l:length()
插入		table.insert(t,foo)	l:append(foo)
迭代算子	ipairs(t)			sgs.qlist(l)

++ AI 中用到的表
在 AI 编写中主要会用到两个表，一个是sgs，该表含有大量由 SWIG 提供的成员函数，另外一个表是 SmartAI。
在具体的 AI 代码中出现的 self，实际上都是在操作 SmartAI 这个表。

sgs 表的常用元素：
sgs.Sanguosha 指向 Engine 对象
sgs.qlist(l) 是 QList 对象的迭代算子
sgs.reverse(t) 将一个表的元素反序，得到反序后新的表。
例如 t == {a, b, c} 则 sgs.reverse(t) == {c, b, a}
sgs.QList2Table(l) 将一个列表转换为表。

SmartAI 表的常用元素：
SmartAI.player 指向 AI 对应的 ServerPlayer 对象
SmartAI.room 指向 AI 所在的房间（Room 对象）
SmartAI.role 是 AI 对应身份的字符串（主公为 "lord"，忠臣为 "loyalist"，反贼为 "rebel"，内奸为 "renegade"）
SmartAI.friends_noself 是一个包含 AI 的所有友方 ServerPlayer 对象指针的表（不包括自身）
SmartAI.friends 是一个包含 AI 的所有友方 ServerPlayer 对象指针的表（包括自身）
SmartAI.enemies 是一个包含 AI 的所有敌方 ServerPlayer 对象指针的表（包括自身）

这两个表的其它重要元素将在后续文档中介绍。

++ 调试 AI 的基本方法。
为了调试 AI，您必须通过点击“启动服务器”然后点击“启动游戏”的方式来启动游戏，不能通过“单机启动”。
调试 AI 的基本方法是通过在服务器端输出信息。输出信息的基本方法有三种：]]
self:log(message)
self.room:output(message)
self.room:writeToConsole(message)

--[[其中前两种是等价的。与第三种的区别在于，前两种仅当 config.ini 中有 DebugOutput=true 时才会输出，后一种无论什么情况都会输出。
这里的 message 是一个字符串。

加入以下代码，则可以了解函数被调用的过程。]]
self:log(debug.traceback())
