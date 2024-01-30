class CEN_TacticalPingEditorComponentClass: SCR_BaseEditorComponentClass
{
}

class CEN_TacticalPingEditorComponent : SCR_BaseEditorComponent
{
	[Attribute(defvalue: "1000", desc: "Maximum pointing distance in meters")]
	protected float m_fMaxPointingDistance;
	
	[Attribute(defvalue: "10", desc: "Range of the ping in meters. Only players in range will see it.")]
	protected float m_fPingRange;
	
	[Attribute(defvalue: "6", desc: "Lifetime of ping in seconds")]
	protected float m_fPingLifetime;
	
	[Attribute(defvalue: "1.5", desc: "Cooldown in seconds until next ping can be sent")]
	protected float m_fPingCooldown;
	
	[Attribute(desc: "Effects of the ping")]
	protected ref array<ref SCR_BaseEditorEffect> m_EffectsTacticalPing;
	
	protected ref ScriptInvoker Event_OnPingEntityRegister = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnPingEntityUnregister = new ScriptInvoker;
	protected float m_fLastPingTime = 0;
	
	void SendPing(vector targetPos, SCR_EditableEntityComponent target = null)
	{
		float currentCooldown = m_fPingCooldown - (GetGame().GetWorld().GetWorldTime() - m_fLastPingTime) / 1000;
		if (m_fLastPingTime > 0 && currentCooldown > 0)
		{
			SCR_NotificationsComponent.SendLocal(ENotification.ACTION_ON_COOLDOWN, currentCooldown * 100);
			return;
		};
		m_fLastPingTime = GetGame().GetWorld().GetWorldTime();
		
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if (!player)
			return;

		Rpc(RpcAsk_SendPing, GetGame().GetPlayerController().GetPlayerId(), player.GetOrigin(), targetPos, Replication.FindId(target));
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SendPing(int reporterID, vector reporterPos, vector targetPos, RplId targetID)
	{
		if (RplSession.Mode() != RplMode.Dedicated)
			RpcDo_SendPing(reporterID, reporterPos, targetPos, targetID);
		
		Rpc(RpcDo_SendPing, reporterID, reporterPos, targetPos, targetID);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_SendPing(int reporterID, vector reporterPos, vector targetPos,  RplId targetID)
	{
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if (!player || vector.Distance(player.GetOrigin(), reporterPos) > m_fPingRange)
			return;
		
		SCR_EditableEntityComponent target = SCR_EditableEntityComponent.Cast(Replication.FindItem(targetID));
		set<SCR_EditableEntityComponent> targets = new set<SCR_EditableEntityComponent>;
		if (target)
			targets.Insert(target);			
		
		CEN_TacticalPingEditorComponent localInstance = CEN_TacticalPingEditorComponent.Cast(CEN_TacticalPingEditorComponent.GetInstance(CEN_TacticalPingEditorComponent, true));
		SCR_BaseEditorEffect.Activate(m_EffectsTacticalPing, localInstance, targetPos, targets);
		
		if (target)
		{
			SCR_NotificationsComponent.SendLocal(ENotification.CEN_TACTICAL_PING_TARGET_ENTITY, reporterID, targetID);
			localInstance.OnPingEntityRegister(target);
		}
		else
		{
			SCR_NotificationsComponent.SendLocal(ENotification.CEN_TACTICAL_PING, targetPos, reporterID);
		}
	}
	
	override void EOnEffect(SCR_BaseEditorEffect effect)
	{
		SCR_EntityEditorEffect entityEffect = SCR_EntityEditorEffect.Cast(effect);
		if (!entityEffect)
			return;
		
		OnPingEntityRegister(SCR_EditableEntityComponent.GetEditableEntity(entityEffect.GetEntity()));
	}
	
	protected void OnPingEntityRegister(SCR_EditableEntityComponent pingEntity)
	{
		Event_OnPingEntityRegister.Invoke(-1, pingEntity);
		GetGame().GetCallqueue().CallLater(Expire, m_fPingLifetime*1000, false, pingEntity);
	}
	
	protected void Expire(SCR_EditableEntityComponent pingEntity)
	{
		if (!pingEntity)
			return;
		
		Event_OnPingEntityUnregister.Invoke(-1, pingEntity);
		
		if (SCR_EditorPingEntity.Cast(pingEntity.GetOwner()))			
			pingEntity.Delete();
	}
	
	float GetMaxPointingDistance()
	{
		return m_fMaxPointingDistance;
	}
	
	ScriptInvoker GetOnPingEntityRegister()
	{
		return Event_OnPingEntityRegister;
	}

	ScriptInvoker GetOnPingEntityUnregister()
	{
		return Event_OnPingEntityUnregister;
	}
	
}