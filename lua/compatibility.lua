-- upward compatibility & downward compatibility

-- for niepan
sgs.DamagedProceed = sgs.DamagedProceed or sgs.Predamaged
sgs.DamageProceed = sgs.DamageProceed or sgs.Predamage
sgs.DamageForseen = sgs.DamageForseen or sgs.Predamaged
sgs.DamageCaused = sgs.DamageCaused or sgs.Predamage
sgs.DamageInflicted = sgs.DamageInflicted or sgs.Predamage

-- for alpha
sgs.Place_PlaceHand = sgs.Place_PlaceHand or sgs.Place_Hand
sgs.Place_PlaceEquip = sgs.Place_PlaceEquip or sgs.Place_Equip
sgs.Place_PlaceDelayedTrick = sgs.Place_PlaceDelayedTrick or sgs.Place_Judging
sgs.Place_PlaceJudge = sgs.Place_PlaceJudge or sgs.Place_Judging
sgs.Place_PlaceSpecial = sgs.Place_PlaceSpecial or sgs.Place_Special
sgs.Place_DiscardPile = sgs.Place_DiscardPile or sgs.Place_DiscardedPile
sgs.Place_DrawPile = sgs.Place_DrawPile or sgs.Place_DrawPile
sgs.Place_PlaceTable = sgs.Place_PlaceTable or sgs.Place_DiscardedPile
sgs.Place_PlaceUnknown = sgs.Place_PlaceUnknown or sgs.Place_Special

sgs.EventPhaseStart = sgs.EventPhaseStart or sgs.PhaseChange
sgs.EventPhaseEnd = sgs.EventPhaseEnd or sgs.PhaseChange
sgs.EventPhaseChanging = sgs.EventPhaseChanging or sgs.PhaseChange
sgs.MaxHpChanged = sgs.MaxHpChanged or sgs.MaxHpLost
sgs.FinishRetrial = sgs.FinishRetrial or sgs.RetrialDone
sgs.GameFinished = sgs.GameFinished or sgs.GameOverJudge
sgs.CardsMoveOneTime = sgs.CardsMoveOneTime or sgs.CardMoving
sgs.PostCardEffected = sgs.PostCardEffected or sgs.CardEffected
sgs.ConfirmDamage = sgs.ConfirmDamage or sgs.Predamage
sgs.PreHpReduced = sgs.PreHpReduced or sgs.HpLost
sgs.PostHpReduced = sgs.PostHpReduced or sgs.HpLost
