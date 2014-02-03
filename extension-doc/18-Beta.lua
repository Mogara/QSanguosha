--[[ 本文档用于介绍在 beta 版中引入的更新
本文档针对的 beta 版本为 20120203(V0.7 Beta 1)
本文档假定阅读者已经有一定的编写 AI 的基础，仅作十分简略的介绍。

新的函数与表使得 AI 的可扩展性大大增强。
* sgs.ai_slash_prohibit：表，由 SmartAI.slashProhibit 载入
% 元素名称：技能名
% 元素：function(self, to, card)
%% self: SmartAI
%% to: ServerPlayer*，拥有元素名称所描述的技能
%% card: Card*
%% 返回值：布尔值，true表明在策略上不宜对 to 使用【杀】 card。
本表设置后，以后编写技能将基本不需修改 SmartAI.slashProhibit

* sgs.ai_cardneed：表，由 SmartAI.getCardNeedPlayer （新函数，见下面的描述） 载入
% 元素名称：技能名
% 元素；function(friend, card, self)
%% friend：ServerPlayer*，拥有元素名称所描述的技能
%% card: Card*
%% self: SmartAI
%% 返回值：布尔值，true 表明友方玩家 friend 需要卡牌 card

标准元素：sgs.ai_cardneed.equip 和 sgs.ai_cardneed.bignumber，详见 smart-ai.lua

* SmartAI:getCardNeedPlayer(cards)；在 cards 中找到一张用于仁德/遗计/任意类似技能的卡牌及使用对象
% cards：表，包含可用的卡牌
% 返回值：一个二元组 Card*, ServerPlayer*，表示卡牌及其使用对象。
引入这一个表和一个函数之后，以后想要刘备认识您编写的新技能而按照您的需要仁德卡牌给您，就变得十分简单了。

* sgs.ai_skill_pindian：表，用于 SmartAI.askForPindian
% 元素名称：reason
% 元素：function(minusecard, self, requstor, maxcard, mincard)
%% minusecard：自己手牌中使用价值最小的卡牌
%% self：SmartAI
%% requestor：ServerPlayer*，请求来源
%% maxcard：手牌中使用价值低于 6 的卡牌中点数最大者
%% mincard：手牌中使用价值低于 6 的卡牌中点数最小者
%% 返回值：Card*，一般可以直接用参数中的一个返回。
若为 nil，表示采用默认策略。
默认策略即：若 requestor 是友方，返回 mincard，否则返回 maxcard。
因此一般只要指明何时返回 minusecard 即可。

* sgs.ai_skill_suit：表，用于 SmartAI.askForSuit
% 元素名称：reason（新修改的源代码中加入了这一参数，如果需要用到本表请自行编译源码）
% 元素：function(self)
%% 返回值：Card::Suit

* sgs.ai_trick_prohibit：表，用于 SmartAI.trickProhibit
% 元素名称：技能名
% 元素：function(card, self, to)
%% 返回值：布尔值，表示是否存在由锦囊牌 card 对含有由元素名称所描述的技能的对象 to 的禁止技
加入这一表之后，随着日后 AI 的进一步完善，帷幕等技能将不用修改 SmartAI（目前仍存在一定的缺陷）

* sgs.ai_slash_weaponfilter：表，用于 SmartAI.useCardSlash
% 元素名称：武器的对象名
% 元素：function(to, self)
%% 返回值：布尔值，表示如果准备对 to 使用【杀】，是否应该首先安装由元素名称描述的武器。
这一个表是装备牌 AI DIY 接口的开端。

上面这些表的例子基本上可以在 smart-ai.lua，standard-ai.lua 和 standard_cards-ai.lua 找到，从略。]]
