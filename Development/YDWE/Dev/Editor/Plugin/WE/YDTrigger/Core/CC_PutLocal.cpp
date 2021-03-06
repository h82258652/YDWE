#include "CC_Include.h"

typedef struct _tag_LocalVarList
{
  enum VARTYPE_User_Defined type;
  char* name;
}
LocalVarList;

LocalVarList g_local_var_list[0x100];
int  g_local_var_list_top;
BOOL g_local_in_mainproc;

int _fastcall
CC_PutLocal_SaveAndCheck(DWORD This, DWORD OutClass, enum VARTYPE_User_Defined type, char* name)
{
  int i;

  for (i = 0; i < g_local_var_list_top; i++)
  {
    if (0 == BLZSStrCmp(name, g_local_var_list[i].name, 260))
      return FALSE;
  }

  if (g_local_var_list_top < 0x100)
  {
    g_local_var_list[g_local_var_list_top].name = name;
    g_local_var_list[g_local_var_list_top].type = type;
    g_local_var_list_top++;

    return TRUE;
  }

  return FALSE;
}

void _fastcall
CC_PutLocal_LocalVar(DWORD This, DWORD OutClass, enum VARTYPE_User_Defined type, char* name)
{
  char buff[260];
  char name_covert[260];

  if (CC_PutLocal_SaveAndCheck(This, OutClass, type, name))
  {
    CC_PutBegin();
    ConvertString(name, name_covert, 260);
    BLZSStrPrintf(buff, 260, "local %s ydl_%s", TypeName[type], name_covert);
    PUT_CONST(buff, 1);
    CC_PutEnd();
  }
}

void _fastcall
CC_PutLocal_SearchVar(DWORD This, DWORD OutClass)
{
  DWORD nVarCount, i;
  DWORD nVarClass;

  nVarCount = *(DWORD*)(This+0x128);
  for (i = 0; i < nVarCount; i++)
  {
    nVarClass = GetGUIVar_Class(This, i);
    if (nVarClass != 0)
    {
      switch (*(DWORD*)(nVarClass+0x138))
      {
        case CC_GUIID_YDWEGetAnyTypeLocalVariable:
          g_local_in_mainproc = TRUE;
          break;
        default:
          break;
      }
      CC_PutLocal_SearchVar(nVarClass, OutClass);
    }    
  }
}

void _fastcall
CC_PutLocal_Search(DWORD This, DWORD OutClass, DWORD isSearchHashLocal, LONG index)
{
  DWORD nItemCount, i;
  DWORD nItemClass;

  nItemCount = *(DWORD*)(This+0x0C);

  for (i = 0; i < nItemCount; i++)
  {
    nItemClass = ((DWORD*)(*(DWORD*)(This+0x10)))[i];
    if (*(DWORD*)(nItemClass+0x13C) == 0)
      continue;

    if ((index) != -1 && (*(LONG*)(nItemClass+0x154) != index))
      continue;

    if (isSearchHashLocal)
    {
      CC_PutLocal_SearchVar(nItemClass, OutClass);
    }

    switch (*(DWORD*)(nItemClass+0x138))
    {
      case CC_GUIID_IfThenElseMultiple:
      case CC_GUIID_ForLoopAMultiple:
      case CC_GUIID_ForLoopBMultiple:
      case CC_GUIID_ForLoopVarMultiple:
        CC_PutLocal_Search(nItemClass, OutClass, isSearchHashLocal, -1);
        break;      
      case CC_GUIID_YDWEForLoopLocVarMultiple:
        CC_PutLocal_LocalVar(nItemClass, OutClass, CC_TYPE_integer, ((char*)&GetGUIVar_Value(nItemClass, 0)));
        CC_PutLocal_Search(nItemClass, OutClass, isSearchHashLocal, -1);
        break;          
      case CC_GUIID_ForGroupMultiple:
      case CC_GUIID_ForForceMultiple:
      case CC_GUIID_EnumDestructablesInRectAllMultiple:
      case CC_GUIID_EnumDestructablesInCircleBJMultiple:
      case CC_GUIID_EnumItemsInRectBJMultiple:
        if (isSearchHashLocal)
        {
          CC_PutLocal_Search(nItemClass, OutClass, isSearchHashLocal, -1);
        }
        break; 
      case CC_GUIID_YDWEEnumUnitsInRangeMultiple:
          CC_PutLocal_LocalVar(nItemClass, OutClass, CC_TYPE_group, "group");
          CC_PutLocal_LocalVar(nItemClass, OutClass, CC_TYPE_unit, "unit");
          CC_PutLocal_Search(nItemClass, OutClass, isSearchHashLocal, 0);
          break;
      case CC_GUIID_YDWETimerStartMultiple:
        CC_PutLocal_LocalVar(nItemClass, OutClass, CC_TYPE_timer, "timer");
        CC_PutLocal_Search(nItemClass, OutClass, isSearchHashLocal, 0);
		break; 
	  case CC_GUIID_YDWERegisterTriggerMultiple:
		  CC_PutLocal_LocalVar(nItemClass, OutClass, CC_TYPE_trigger, "trigger");
		  CC_PutLocal_Search(nItemClass, OutClass, isSearchHashLocal, 0);
		  break; 
      default:
        break;           
    }
  }
}

void _fastcall
CC_PutLocal_Begin(DWORD This, DWORD OutClass, DWORD isTimer, DWORD isSearchHashLocal)
{
  g_local_var_list_top = 0;
  
  if (isSearchHashLocal)
  {
    g_local_in_mainproc = FALSE;
  }

  CC_PutLocal_Search(This, OutClass, isSearchHashLocal, -1);

  if (!isTimer && isSearchHashLocal)
  {
    CC_Put_TriggerLocalVar_Begin(OutClass);
  }
}

void _fastcall
CC_PutLocal_End(DWORD This, DWORD OutClass, DWORD isTimer, DWORD isEnd)
{
  int i;
  char buff[260];
  char name_covert[260];
  
  if (!isTimer)
  {
    CC_Put_TriggerLocalVar_End(OutClass);
  } 

  for (i = 0; i < g_local_var_list_top; i++)
  {
    switch (g_local_var_list[i].type)
    {
    case CC_TYPE_unit:
    case CC_TYPE_group:
    case CC_TYPE_timer:
	case CC_TYPE_trigger:
      CC_PutBegin();
      ConvertString(g_local_var_list[i].name, name_covert, 260);
      BLZSStrPrintf(buff, 260, "set ydl_%s = null", name_covert);
      PUT_CONST(buff, 1);
      CC_PutEnd();
      break;
    }
  }

  if (isEnd)
  {
    g_local_var_list_top = 0;
    g_local_in_mainproc = FALSE;
  }
}