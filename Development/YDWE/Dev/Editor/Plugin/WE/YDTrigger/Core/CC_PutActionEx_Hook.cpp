#include "CC_Include.h"

extern int g_mother_id;
extern char* g_handle_string;

extern BOOL g_bDisableSaveLoadSystem;
BOOL g_bYDWEEnumUnitsInRangeMultipleFlag = FALSE;

BOOL _fastcall  CC_PutAction_SearchVar(DWORD This, DWORD OutClass);

void _fastcall 
    CC_PutActionEx_ForLoop_GetVarName(DWORD This, DWORD cc_guiid_type, char* varname)
{
    char varname_t[260];

    switch (cc_guiid_type)
    {
    case CC_GUIID_ForLoopA:
    case CC_GUIID_ForLoopAMultiple:
        BLZSStrCopy(varname, "bj_forLoopAIndex", 260);
        break;
    case CC_GUIID_ForLoopB:
    case CC_GUIID_ForLoopBMultiple:
        BLZSStrCopy(varname, "bj_forLoopBIndex", 260);
        break;
    case CC_GUIID_ForLoopVar:
    case CC_GUIID_ForLoopVarMultiple:
        CC_GetGlobalVar((*(DWORD**)(This+0x12C))[0], 0, varname, 260);
        break;
    case CC_GUIID_YDWEForLoopLocVarMultiple:
        BLZSStrPrintf(varname_t, 260, STRING_YDWE_LOCAL"%s", ((char*)&GetGUIVar_Value(This, 0)));
        ConvertString(varname_t, varname, 260);
        break;
    }
}

void _fastcall 
    CC_PutActionEx_ForLoop(DWORD This, DWORD OutClass, char* name, DWORD cc_guiid_type)
{
    char buff[260];
    char varname[260];

    CC_PutActionEx_ForLoop_GetVarName(This, cc_guiid_type, varname);

    CC_PutBegin();

    switch (cc_guiid_type)
    {
    case CC_GUIID_ForLoopA:
    case CC_GUIID_ForLoopB:
    case CC_GUIID_ForLoopAMultiple:
    case CC_GUIID_ForLoopBMultiple:
        BLZSStrPrintf(buff, 260, "set %s = ", varname);
        PUT_CONST(buff, 0);
        PUT_VAR(This, 0);
        PUT_CONST("", 1);

        BLZSStrPrintf(buff, 260, "set %sEnd = ", varname);
        PUT_CONST(buff, 0);
        PUT_VAR(This, 1);
        PUT_CONST("", 1);
        break;
    case CC_GUIID_ForLoopVar:
    case CC_GUIID_ForLoopVarMultiple:
    case CC_GUIID_YDWEForLoopLocVarMultiple:
        BLZSStrPrintf(buff, 260, "set %s = ", varname);
        PUT_CONST(buff, 0);
        PUT_VAR(This, 1);
        PUT_CONST("", 1);
        break;
    }

    PUT_CONST("loop", 1);

    CC_PutBegin();
    switch (cc_guiid_type)
    {
    case CC_GUIID_ForLoopA:
    case CC_GUIID_ForLoopAMultiple: 
    case CC_GUIID_ForLoopB:
    case CC_GUIID_ForLoopBMultiple:   
        BLZSStrPrintf(buff, 260, "exitwhen %s > %sEnd", varname, varname);
        PUT_CONST(buff, 1);
        break;
    case CC_GUIID_ForLoopVar:
    case CC_GUIID_ForLoopVarMultiple:
    case CC_GUIID_YDWEForLoopLocVarMultiple:
        BLZSStrPrintf(buff, 260, "exitwhen %s > ", varname);
        PUT_CONST(buff, 0);
        PUT_VAR(This, 2);
        PUT_CONST("", 1);
        break;
    }
    CC_PutEnd();

    switch (cc_guiid_type)
    {
    case CC_GUIID_ForLoopA:
    case CC_GUIID_ForLoopB:  
        CC_PutVar_Code(This, OutClass, name, 2, CC_GUI_TYPE_ACTION);
        break;
    case CC_GUIID_ForLoopVar: 
        CC_PutVar_Code(This, OutClass, name, 3, CC_GUI_TYPE_ACTION);
        break;
    case CC_GUIID_ForLoopAMultiple: 
    case CC_GUIID_ForLoopBMultiple: 
    case CC_GUIID_ForLoopVarMultiple:
    case CC_GUIID_YDWEForLoopLocVarMultiple:
        CC_PutBlock_Action(This, OutClass, name, 0);
        break;
    }  

    CC_PutBegin();
    BLZSStrPrintf(buff, 260, "set %s = %s + 1", varname, varname);
    PUT_CONST(buff, 1);
    CC_PutEnd();

    PUT_CONST("endloop", 1);
    CC_PutEnd();
}

void _fastcall 
    CC_PutActionEx_Hook(DWORD This, DWORD EDX, DWORD OutClass, char* name, DWORD Type, DWORD Endl)
{
    char buff[260];

    switch (*(DWORD*)(This+0x138))
    {
    case CC_GUIID_IfThenElse:
        {
            CC_PutBegin();
            PUT_CONST("if (", 0);
            CC_PutVar_Code(This, OutClass, name, 0, CC_GUI_TYPE_CONDITION);
            PUT_CONST(") then", 1);
            CC_PutVar_Code(This, OutClass, name, 1, CC_GUI_TYPE_ACTION);
            PUT_CONST("else", 1);
            CC_PutVar_Code(This, OutClass, name, 2, CC_GUI_TYPE_ACTION);
            PUT_CONST("endif", 1);
            CC_PutEnd();

            break;
        }
    case CC_GUIID_IfThenElseMultiple:
        {
            CC_PutBegin();
            PUT_CONST("if (", 0);
            CC_PutBlock_Condition_And(This, OutClass, name, 0);
            PUT_CONST(") then", 1);
            CC_PutBlock_Action(This, OutClass, name, 1);
            PUT_CONST("else", 1);
            CC_PutBlock_Action(This, OutClass, name, 2);
            PUT_CONST("endif", 1);
            CC_PutEnd();

            break;
        }
    case CC_GUIID_YDWEEnumUnitsInRangeMultiple:
        {
            if (!g_bYDWEEnumUnitsInRangeMultipleFlag)
            {
                g_bYDWEEnumUnitsInRangeMultipleFlag = TRUE;

                CC_PutBegin();
                PUT_CONST("set ydl_group = CreateGroup()", 1);
                PUT_CONST("call YDTriggerEnumUintsInRange(ydl_group, ", 0); 
                PUT_VAR(This, 0);
                PUT_CONST(", ", 0); 
                PUT_VAR(This, 1);
                PUT_CONST(", ", 0); 
                PUT_VAR(This, 2);
                PUT_CONST(")", 1); 
                PUT_CONST("loop", 1);
                CC_PutBegin();
                PUT_CONST("set ydl_unit = FirstOfGroup(ydl_group)", 1);
                PUT_CONST("exitwhen ydl_unit == null", 1);
                PUT_CONST("call GroupRemoveUnit(ydl_group, ydl_unit)", 1);
                CC_PutEnd();
                CC_PutBlock_Action(This, OutClass, name, 0);
                PUT_CONST("endloop", 1);
                PUT_CONST("call DestroyGroup(ydl_group)", 1);
                CC_PutEnd();

                g_bYDWEEnumUnitsInRangeMultipleFlag = FALSE;
            }
        }
        break;
    case CC_GUIID_ForLoopA:
    case CC_GUIID_ForLoopB:
    case CC_GUIID_ForLoopVar:
    case CC_GUIID_ForLoopAMultiple:
    case CC_GUIID_ForLoopBMultiple:
    case CC_GUIID_ForLoopVarMultiple:
    case CC_GUIID_YDWEForLoopLocVarMultiple:
        { 
            CC_PutActionEx_ForLoop(This, OutClass, name, *(DWORD*)(This+0x138));
            break;
        }
    case CC_GUIID_ReturnAction:
        {
            if (g_mother_id == CC_GUIID_YDWETimerStartMultiple
				|| g_mother_id == CC_GUIID_YDWERegisterTriggerMultiple)
            {
                CC_PutLocal_End(This, OutClass, TRUE, FALSE);
            }
            else
            {
                CC_PutLocal_End(This, OutClass, FALSE, FALSE);
            }

            CC_PutBegin();
            PUT_CONST("return", 1);
            CC_PutEnd();
            break;
        }
    case CC_GUIID_YDWESetAnyTypeLocalVariable:
        {
            CC_PutBegin();

            if (g_mother_id == CC_GUIID_YDWETimerStartMultiple)
            {
                CC_Put_SetTimerParameters(This, OutClass, name, "GetExpiredTimer()");
			}
			else if (g_mother_id == (0x8000 | (int)CC_GUIID_YDWETimerStartMultiple))
			{
				CC_Put_SetTimerParameters(This, OutClass, name, g_handle_string);
			}
			else if (g_mother_id == CC_GUIID_YDWERegisterTriggerMultiple)
			{
				CC_Put_SetTimerParameters(This, OutClass, name, "GetTriggeringTrigger()");
			}
            else
            {
                CC_Put_TriggerLocalVar_Set(This, OutClass, name);
            }
            CC_PutEnd();
            break;  
        }
    case CC_GUIID_YDWETimerStartMultiple:
		{   
			CC_PutBegin();
			PUT_CONST("set "YDL_TIMER" = ", 0);
			PUT_VAR(This, 0);
			PUT_CONST("", 1);
			CC_PutEnd();

			CC_PutBlock_TimerParameters(This, OutClass, name, 0, YDL_TIMER);

			CC_PutBegin();
			PUT_CONST("call TimerStart("YDL_TIMER", ", 0);
			PUT_VAR(This, 1);
			PUT_CONST(", ", 0);
			PUT_VAR(This, 2);
			BLZSStrPrintf(buff, 260, ", function %sT)", name);
			PUT_CONST(buff, 1);
			CC_PutEnd();

            break;
        }
	case CC_GUIID_YDWETimerStartFlush:
		if (g_mother_id == CC_GUIID_YDWETimerStartMultiple)
		{
			g_bDisableSaveLoadSystem = FALSE;

			CC_PutBegin();
			PUT_CONST("call YDTriggerClearTable(YDTriggerH2I(GetExpiredTimer()))", 1);
			PUT_CONST("call DestroyTimer(GetExpiredTimer())", 1);
			CC_PutEnd();
		}
		else
		{
			ShowError(OutClass, "WESTRING_ERROR_YDTRIGGER_TIMER_FLUSH");
		}
		break;
    case CC_GUIID_TriggerSleepAction:
        CC_PutBegin();
        PUT_CONST("call TriggerSleepAction(", 0);
        PUT_VAR(This, 0);
        PUT_CONST(")", 1);
        if (g_mother_id != CC_GUIID_YDWETimerStartMultiple
			&& g_mother_id != CC_GUIID_YDWERegisterTriggerMultiple)
        {
            CC_Put_TriggerLocalVar_Sleep_End(OutClass);
		}
		else
		{
			ShowError(OutClass, "WESTRING_ERROR_YDTRIGGER_ILLEGAL_WAIT");
		}
        CC_PutEnd();
        break;
    case CC_GUIID_PolledWait:
        CC_PutBegin();
        PUT_CONST("call PolledWait(", 0);
        PUT_VAR(This, 0);
        PUT_CONST(")", 1);
		if (g_mother_id != CC_GUIID_YDWETimerStartMultiple
			&& g_mother_id != CC_GUIID_YDWERegisterTriggerMultiple)
        {
            CC_Put_TriggerLocalVar_Sleep_End(OutClass);
        }
		else
		{
			ShowError(OutClass, "WESTRING_ERROR_YDTRIGGER_ILLEGAL_WAIT");
		}
        CC_PutEnd();
        break;
    case CC_GUIID_YDWERegisterTriggerMultiple:
		{
			CC_PutBegin();
			PUT_CONST("set "YDL_TRIGGER" = ", 0);
			PUT_VAR(This, 0);
			PUT_CONST("", 1);
			CC_PutEnd();

			CC_PutBlock_TimerParameters(This, OutClass, name, 1, YDL_TRIGGER);

			// Event  
			CC_PutBlock_Event(This, OutClass, name, 0, YDL_TRIGGER);

			CC_PutBegin();
			BLZSStrPrintf(buff, 260, "call TriggerAddCondition("YDL_TRIGGER", Condition(function %sConditions))", name);
			PUT_CONST(buff, 1); 
            CC_PutEnd();

            break;
        }
	case CC_GUIID_YDWERegisterTriggerFlush:
		if (g_mother_id == CC_GUIID_YDWERegisterTriggerMultiple)
		{
			g_bDisableSaveLoadSystem = FALSE;

			CC_PutBegin();
			PUT_CONST("call YDTriggerClearTable(YDTriggerH2I(GetTriggeringTrigger()))", 1);
			PUT_CONST("call DestroyTrigger(GetTriggeringTrigger())", 1);
			CC_PutEnd();
		}
		else
		{
			ShowError(OutClass, "WESTRING_ERROR_YDTRIGGER_TRIGGER_FLUSH");
		}
		break;
    case CC_GUIID_YDWESaveAnyTypeDataByUserData:
        {
            CC_PutBegin();
            CC_Put_YDWESaveAnyTypeDataByUserData(This, OutClass, name);
            CC_PutEnd();
            break;
        }
    case CC_GUIID_YDWEFlushAnyTypeDataByUserData:
        {
            CC_PutBegin();
            CC_Put_YDWEFlushAnyTypeDataByUserData(This, OutClass, name);
            CC_PutEnd();
            break;
        }
    case CC_GUIID_YDWEFlushAllByUserData:
        {
            CC_PutBegin();
            CC_Put_YDWEFlushAllByUserData(This, OutClass, name);
            CC_PutEnd();
            break;
        }
	case CC_GUIID_YDWEActivateTrigger:
		{
			LPCSTR lpszTrigVar = (char*)&GetGUIVar_Value(This, 0);
			LPCSTR lpszMode = (char*)&GetGUIVar_Value(This, 1);

			if (0 == BLZSStrCmp("gg_trg_", lpszTrigVar, sizeof("gg_trg_")-1))
			{
				if (0 == BLZSStrCmp("OnOffOn", lpszMode, 0x7FFFFFFF))
				{
					CC_PutBegin();
					BLZSStrPrintf(buff, 260, "call ExecuteFunc(\"InitTrig_%s_Orig\")", lpszTrigVar+(sizeof("gg_trg_")-1));
					PUT_CONST(buff, 1); 
					CC_PutEnd();
				}
				else if (0 == BLZSStrCmp("OnOffOff", lpszMode, 0x7FFFFFFF))
				{
					CC_PutBegin();
					BLZSStrPrintf(buff, 260, "call InitTrig_%s_Orig()", lpszTrigVar+(sizeof("gg_trg_")-1));
					PUT_CONST(buff, 1); 
					CC_PutEnd();
				}
			}

			break;
		}
	case CC_GUIID_AddTriggerEvent:
		{
			DWORD nVarClass = GetGUIVar_Class(This, 1);
			if (!nVarClass)
			{
				return ;
			}

			CC_PutBegin();

			char szName[260];
			char szTrigName[260];

			CC_GetGUIName(nVarClass, 0, szName, 260);
			BLZSStrPrintf(buff, 260, "call %s(", szName);
			PUT_CONST(buff, 0);
			PUT_VAR(This, 0);

			DWORD nVar = *(DWORD*)(nVarClass+0x128);
			BLZSStrPrintf(szTrigName, 260, "%s002", name);
			for (DWORD i = 0; i < nVar; ++i)
			{
				PUT_CONST(", ", 0);
				CC_PutVar(nVarClass, 0, OutClass, szTrigName, i, 2, 1);
			}
			PUT_CONST(")", 1);

			CC_PutEnd();
		}
		break;
    default:
        {
            if (CC_PutAction_SearchVar(This, OutClass))
            {
				ShowError(OutClass, "WESTRING_ERROR_YDTRIGGER_ANYPLAYER");
            }
            CC_PutActionEx(This, EDX, OutClass, name, Type, Endl);
        }
        break;
    }
}

void _fastcall
    CC_PutAction(DWORD This, DWORD OutClass, char* name, DWORD index, DWORD Endl)
{
    char NewName[260];
    BLZSStrPrintf(NewName, 260, "%sFunc%03d", name, index+1);

    CC_PutActionEx_Hook(This, 0, OutClass, NewName, CC_GUI_TYPE_ACTION, Endl);
}