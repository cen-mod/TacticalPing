modded class SCR_EditorPingInfoDisplay : SCR_InfoDisplay
{
	protected CEN_TacticalPingEditorComponent m_pCEN_TacticalPingManager;
	
	protected void CEN_SendTacticalPing(float value, EActionTrigger reason)
	{
		if (SCR_EditorManagerEntity.IsOpenedInstance())
			return;
		
		//--- Position under cursor
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;
			
		//--- Not reliable, cursor is not hidden in center of the screen on client
		//int mouseX, mouseY;
		//WidgetManager.GetMousePos(mouseX, mouseY);
			
		int screenW = workspace.GetWidth();
		int screenH = workspace.GetHeight();
			
		BaseWorld world = GetGame().GetWorld();
		vector outDir;
		vector startPos = workspace.ProjScreenToWorld(workspace.DPIUnscale(screenW / 2), workspace.DPIUnscale(screenH / 2), outDir, world);
		outDir *= m_pCEN_TacticalPingManager.GetMaxPointingDistance();
	
		autoptr TraceParam trace = new TraceParam();
		trace.Start = startPos;
		trace.End = startPos + outDir;
		trace.Flags = TraceFlags.WORLD | TraceFlags.OCEAN | TraceFlags.ENTS;
		trace.LayerMask = TRACE_LAYER_CAMERA;
		float traceDis = world.TraceMove(trace, null);
		SCR_EditableEntityComponent	target = SCR_EditableEntityComponent.GetEditableEntity(trace.TraceEnt);
			
		if (!target && traceDis == 1)
			return; //--- No intersection
		
		m_pCEN_TacticalPingManager.SendPing(startPos + outDir * traceDis, target);
	}
	
	override event void OnStartDraw(IEntity owner)
	{
		super.OnStartDraw(owner);
		
		m_pCEN_TacticalPingManager = CEN_TacticalPingEditorComponent.Cast(CEN_TacticalPingEditorComponent.GetInstance(CEN_TacticalPingEditorComponent, true));
		if (!m_pCEN_TacticalPingManager) return;
		
		m_pCEN_TacticalPingManager.GetOnPingEntityRegister().Insert(OnPingEntityRegister);
		m_pCEN_TacticalPingManager.GetOnPingEntityUnregister().Insert(OnPingEntityUnregister);
		
		InputManager inputManager = GetGame().GetInputManager();
		inputManager.AddActionListener("TacticalPing", EActionTrigger.DOWN, CEN_SendTacticalPing);
		inputManager.AddActionListener("TacticalPingHold", EActionTrigger.DOWN, CEN_SendTacticalPing);
	}
	
	override event void OnStopDraw(IEntity owner)
	{
		super.OnStopDraw(owner);
		
		if (!m_pCEN_TacticalPingManager) return;
		
		m_pCEN_TacticalPingManager.GetOnPingEntityRegister().Remove(OnPingEntityRegister);
		m_pCEN_TacticalPingManager.GetOnPingEntityUnregister().Remove(OnPingEntityUnregister);
		
		InputManager inputManager = GetGame().GetInputManager();
		inputManager.RemoveActionListener("TacticalPing", EActionTrigger.DOWN, CEN_SendTacticalPing);
		inputManager.RemoveActionListener("TacticalPingHold", EActionTrigger.DOWN, CEN_SendTacticalPing);
	}
};