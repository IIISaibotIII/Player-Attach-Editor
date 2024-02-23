#if defined _attach_animations_inc
	#endinput
#endif
#define _attach_animations_inc

const ATTACH_ANIM_MAX_ROW_IN_PAGE 				= 20;
const ATTACH_ANIM_MAX_LENGHT_NAME				= 64;
const ATTACH_ANIM_INVALID_ANIM_ID				= -1;

static gAttachAnimationsPage[MAX_PLAYERS];
static gAttachAnimationsLibraryIndex[MAX_PLAYERS][ATTACH_ANIM_MAX_ROW_IN_PAGE];
static gAttachAnimationsArrayIndex[MAX_PLAYERS][ATTACH_ANIM_MAX_ROW_IN_PAGE];


public OnGuiAttachMenuClicked(playerid, typeid)
{
	if (typeid == GUI_ATTACH_MENU_ANIMATIONS)
	{
		gAttachAnimationsPage[playerid] = 0;
		Dialog_Show(playerid, "dAttachAnimationsList");
		return 1;
	}
	else if (typeid == GUI_ATTACH_MENU_ANIMATION_CLEAR)
	{
		ClearAnimations(playerid);
		attachmain_JsonSaveAnimation(playerid, -1);
		return 1;
	}
#if defined attachanim_OnGuiAttachMenu
	return attachanim_OnGuiAttachMenu(playerid, typeid);
#else
	return 1;
#endif
}
#if defined _ALS_OnGuiAttachMenu
	#undef OnGuiAttachMenuClicked
#else
	#define _ALS_OnGuiAttachMenu
#endif
#define OnGuiAttachMenuClicked attachanim_OnGuiAttachMenu
#if defined attachanim_OnGuiAttachMenu
	forward attachanim_OnGuiAttachMenu(playerid, typeid);
#endif


DialogCreate:dAttachAnimationsList(playerid)
{
	new length;
	new library_index;
	new name[ATTACH_ANIM_MAX_LENGHT_NAME + 1];
	new string[42 + 30 * 2 + (25 + (- 2 + sizeof(name))) * ATTACH_ANIM_MAX_ROW_IN_PAGE];
	
	language_GetDialogString(
		playerid,
		DG_ADD_ANIMATION,
		string
	);
	new JsonNode:node;
	new JsonNode:item;
	new JsonCallResult:result = JSON_ParseFile(json_file_name_animations, node);

	if (result == JSON_CALL_NO_ERR)
	{
		JSON_GetArray(node, "animations", node);
		JSON_ArrayLength(node, length);

		new page = gAttachAnimationsPage[playerid];
		new i = page * ATTACH_ANIM_MAX_ROW_IN_PAGE;
		new j;
		new index;

		for (; i < length; i++)
		{
			if (j++ == ATTACH_ANIM_MAX_ROW_IN_PAGE)
			{
				break;
			}
			JSON_ArrayObject(node, i, item);

			JSON_GetString(item, "name", name, sizeof(name));
			JSON_GetInt(item, "library_index", library_index);

			format(string, sizeof(string),
				"%s"cMain"» "cGrey"%s\n",
				string,
				name
			);
			gAttachAnimationsLibraryIndex[playerid][index] = library_index;
			gAttachAnimationsArrayIndex[playerid][index] = i;
			index++;
		}
		if (length > (page * ATTACH_ANIM_MAX_ROW_IN_PAGE) + ATTACH_ANIM_MAX_ROW_IN_PAGE)
		{
			strcat(string, ""cMain"»»»\n");

			if (page > 0)
			{
				strcat(string, ""cMain"«««");
			}
		}
		else if (page > 0)
		{
			strcat(string, ""cMain"«««");
		}
	}
	JSON_Cleanup(item);
	JSON_Cleanup(node);

	Dialog_Open(
		playerid,
		Dialog:dAttachAnimationsList,
		DIALOG_STYLE_LIST,
		!""cMain"PAEditor",
		string,
		!"»",
		!"X"
	);
	return 1;
}

DialogResponse:dAttachAnimationsList(playerid, response, listitem, inputtext[])
{
	if (!response)
	{
		gui_atatch_menu_ShowCursor(playerid);
		return 0;
	}
	if (!strcmp(inputtext, "»»»", false))
	{
		gAttachAnimationsPage[playerid]++;
		Dialog_Show(playerid, "dAttachAnimationsList");
		return 0;
	}
	else if (!strcmp(inputtext, "«««", false))
	{
		gAttachAnimationsPage[playerid]--;
		Dialog_Show(playerid, "dAttachAnimationsList");
		return 0;
	}
	if (listitem == 0)
	{
		Dialog_Show(playerid, "dAttachAddAnimation");
		return 0;
	}
	if (listitem == 1)
	{
		Dialog_Show(playerid, "dAttachAnimationsList");
		return 0;
	}
	new index_library = gAttachAnimationsLibraryIndex[playerid][listitem - 2];
	new index_array = gAttachAnimationsArrayIndex[playerid][listitem - 2];

	gAttachAnimationsLibraryIndex[playerid][0] = index_library;
	gAttachAnimationsArrayIndex[playerid][0] = index_array;

	new str_dialog[LANGUAGE_MAX_LENGHT_STRING_DL + 1];

	language_GetDialogString(
		playerid,
		DG_APPLY_ANIM_REMOVE_FROM_LIST,
		str_dialog
	);
	Dialog_Open(
		playerid,
		Dialog:dAttachAnimationAction,
		DIALOG_STYLE_LIST,
		!""cMain"PAEditor",
		str_dialog,
		!"»",
		!"«"
	);
	return 1;
}


DialogResponse:dAttachAnimationAction(playerid, response, listitem, inputtext[])
{
	if (!response)
	{
		Dialog_Show(playerid, "dAttachAnimationsList");
		return 0;
	}
	if (listitem == 0)
	{
		new anim_index = gAttachAnimationsLibraryIndex[playerid][0];
		
		attachanim_ApplyAnimationIndex(playerid, anim_index);
		attachmain_JsonSaveAnimation(playerid, anim_index);
		language_MessageInfo(playerid, MSG_ANIM_APPLIED_TO_CHARACTER);
	}
	else
	{
		new JsonNode:node;
		new JsonCallResult:result = JSON_ParseFile(json_file_name_animations, node);
		new index = gAttachAnimationsArrayIndex[playerid][0];

		if (result == JSON_CALL_NO_ERR)
		{
			language_MessageInfo(playerid, MSG_ANIM_REMOVED_SUCCESS);

			JSON_ArrayRemoveIndex(node, "animations", index);
			JSON_SaveFile(json_file_name_animations, node, 4);
		}
		JSON_Cleanup(node);
		gAttachAnimationsPage[playerid] = 0;
		Dialog_Show(playerid, "dAttachAnimationsList");
	}
	return 1;
}


DialogCreate:dAttachAddAnimation(playerid)
{
	new str_dialog[LANGUAGE_MAX_LENGHT_STRING_DL + 1];

	language_GetDialogString(
		playerid,
		DG_ADD_ANIM_ENTER_TITLE_INDEX,
		str_dialog
	);
	Dialog_Open(
		playerid,
		Dialog:dAttachAddAnimation,
		DIALOG_STYLE_INPUT,
		!""cMain"PAEditor",
		str_dialog,
		!"»",
		!"«"
	);
	return 1;
}

DialogResponse:dAttachAddAnimation(playerid, response, listitem, inputtext[])
{
	if (!response)
	{
		Dialog_Show(playerid, "dAttachAnimationsMenu");
		return 0;
	}
	new name[ATTACH_ANIM_MAX_LENGHT_NAME + 1];
	new library_index;

	if (sscanf(inputtext, "P<,.>s[*]d", ATTACH_ANIM_MAX_LENGHT_NAME, name, library_index))
	{
		Dialog_Show(playerid, "dAttachAddAnimation");
		return 0;
	}
	new JsonNode:item;
	new JsonNode:node;
	new JsonCallResult:result = JSON_ParseFile(json_file_name_animations, node);

	if (result == JSON_CALL_NO_ERR)
	{
		new length;
		new existing_library_index;
		
		JSON_GetArray(node, "animations", node);
		JSON_ArrayLength(node, length);

		for (new i; i < length; i++)
		{
			JSON_ArrayObject(node, i, item);
			JSON_GetInt(item, "library_index", existing_library_index);

			if (existing_library_index == library_index)
			{
				language_MessageError(playerid, MSG_ANIM_EXISTS_WITH_INDEX);

				Dialog_Show(playerid, "dAttachAddAnimation");
				JSON_Cleanup(node);
				JSON_Cleanup(item);
				return 0;
			}
		}
	}
	JSON_Cleanup(item);
	JSON_Cleanup(node);
	
	result = JSON_ParseFile(json_file_name_animations, node);

	if (result == JSON_CALL_NO_ERR)
	{
		JSON_ArrayAppend(node, "animations", JSON_Object(
			"name", JSON_String(name),
			"library_index", JSON_Int(library_index))
		);
		JSON_SaveFile(json_file_name_animations, node, 4);
	}
	JSON_Cleanup(node);
	
	gAttachAnimationsPage[playerid] = 0;
	Dialog_Show(playerid, "dAttachAnimationsList");
	return 1;
}

stock attachanim_ApplyAnimationIndex(playerid, index)
{
	if (index == ATTACH_ANIM_INVALID_ANIM_ID)
	{
		return 0;
	}
	new lib[24];
	new name[24];
	new anim_index = index;

	GetAnimationName(anim_index, lib, sizeof(lib), name, sizeof(name));

	ApplyAnimation(
		.playerid = playerid,
		.animlib = lib,
		.animname = name,
		.fDelta = 4.0,
		.loop = 1,
		.lockx = 0,
		.locky = 0,
		.freeze = 0,
		.time = 0,
		.forcesync = 0
	);
	return 1;
}