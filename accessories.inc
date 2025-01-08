/**
 * Module: Accessories
 * Author: Tema_Kovalev, Riccardo_Morreti
 *
 * Point Role Play (c) Point-rp.ru 2024
 */


#if defined this
    #undef this
#endif
#define this. _acce

public OnGameModeInit()
{
    this.Init();
    #if defined acce_OnGameModeInit
        return acce_OnGameModeInit();
    #else
        return 1;
    #endif
}
#if defined _ALS_OnGameModeInit
    #undef OnGameModeInit
#else
    #define _ALS_OnGameModeInit
#endif

#define OnGameModeInit acce_OnGameModeInit
#if defined acce_OnGameModeInit
    forward acce_OnGameModeInit();
#endif

public OnGameModeExit()
{
    this.Exit();
    #if defined acce_OnGameModeExit
        return acce_OnGameModeExit();
    #else
        return 1;
    #endif
}
#if defined _ALS_OnGameModeExit
    #undef OnGameModeExit
#else
    #define _ALS_OnGameModeExit
#endif

#define OnGameModeExit acce_OnGameModeExit
#if defined acce_OnGameModeExit
    forward acce_OnGameModeExit();
#endif

private DB:Acce.dbHandle;
private Acce.pickup[2];

private Acce.Init () {
    Callback.Add(@OnDialogResponse, #this.OnDialogResponse);
    Callback.Add(@OnPickUpDynamicPickup, #this.OnPickUpDynamicPickup);

    if ((this.dbHandle = DB_Open("acce.db")) != DB:0) {
        printf("[SQLite] Add table=acce.db, handle=%d", _:this.dbHandle);

        DB_FreeResultSet(DB_ExecuteQuery(this.dbHandle, "CREATE TABLE IF NOT EXISTS `accessories`\
            (`id` INTEGER PRIMARY KEY AUTOINCREMENT, `owner_id` INTEGER, `index` INTEGER, `modelid` INTEGER, `boneid` INTEGER, `x` FLOAT, `y` FLOAT, `z` FLOAT, `rx` FLOAT, `ry` FLOAT,\
            `rz` FLOAT, `sx` FLOAT, `sy` FLOAT, `sz` FLOAT, `st_acce` INTEGER DEFAULT(0))"));
    }

    this.pickup[0] = CreateDynamicPickup(1318, 23, 153.9805,697.1035,12.7650, 0, 0);
    this.pickup[1] = CreateDynamicPickup(1318, 23, 1242.5327,-2768.9839,1001.0000, 10, 4);
    
    CreateDynamic3DTextLabel("Магазин аксессуаров", COLOR_ORANGE, 153.9805,697.1035,12.7650, 20.0);

    Trigger.Create(#this.BuyAcce, 129, 1.5, Float:{1238.7017,-2775.5806,1001.0000}, 0.0, 10);
}

CALLBACK Acce.BuyAcce (playerid, areaid) {
    SetPlayerPos(playerid, 1242.3075,-2780.0625,1001.0000);
    SetPlayerFacingAngle(playerid, 90.0);
    SetCameraBehindPlayer(playerid);

    SetPlayerVirtualWorld(playerid, playerid+1);
    SetPlayerInterior(playerid, 4);

    SetPlayerCameraPos(playerid, 1240.0925,-2780.0266,1001.0000);
    SetPlayerCameraLookAt(playerid, 1242.3075,-2780.0625,1001.0000);

    TogglePlayerControllable(playerid, false);

    this.CefBuy(playerid);
    return 1;
}

private Acce.Exit() {
    DB_Close(this.dbHandle);
}

CALLBACK Acce.OnPickUpDynamicPickup (playerid, pickupid) {
    if (pickupid == this.pickup[0]) {
        SetPlayerPos(playerid, 1240.8276,-2771.5818,1001.0000);
        SetPlayerFacingAngle(playerid, 180.0);
        SetCameraBehindPlayer(playerid);
        SetPlayerInterior(playerid, 4);
        SetPlayerVirtualWorld(playerid, 10);
        return 1;
    }

    if (pickupid == this.pickup[1]) {
        SetPlayerPos(playerid, 156.0049,696.1924,12.5686);
        SetPlayerFacingAngle(playerid, 250.0);
        SetCameraBehindPlayer(playerid);
        SetPlayerInterior(playerid, 0);
        SetPlayerVirtualWorld(playerid, 0);
        return 1;
    }
    return 1;
}

enum {
    DIALOG_PCOINS_ACCE = 30,
    DIALOG_EDIT_ACCE = 31
}

private Acce.ListItem[MAX_PLAYERS][sizeof(Acce.Info)];

CALLBACK Acce.OnDialogResponse (playerid, dialogid, response, listitem, inputtext[]) {
    if (dialogid == DIALOG_BUY_ACCE) {
        if (!response) return 1;

        this.OnCloseInterface(playerid);

        new id = GetPVarInt(playerid, #this.BuyId);

        if (PlayerInfo[playerid][pCash] < this.Info[id][this.MPrice]) {
            SendClientMessage(playerid, COLOR_GREY, " У вас недостаточно денег");
            return 1;
        }

        if (IsPlayerAttachedObjectSlotUsed(playerid, this.Info[id][this.Type])) {
            SendClientMessage(playerid, COLOR_GREY, " У вас занят текущий слот");
            return 1;
        }

        new buffer[256];
        format(buffer, sizeof(buffer), "SELECT * FROM `accessories` WHERE `modelid` = '%d'", this.Info[id][this.ModelID]);
        new DBResult:this.dbResult = DB_ExecuteQuery(this.dbHandle, buffer);

        if (DB_GetRowCount(this.dbResult)) {
            SendClientMessage(playerid, COLOR_GREY, " У вас уже куплен такой аксессуар");
            DB_FreeResultSet(this.dbResult);
            return 1;
        }

        PlayerInfo[playerid][pCash] -= this.Info[id][this.MPrice];

        this.GiveSQL(playerid, this.Info[id][this.Index], this.Info[id][this.ModelID], this.SearchBone(this.Info[id][this.Type]), 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
        SendClientMessage(playerid, COLOR_GREEN, " [Информация] "STR_WHITE"Вы успешно купили акссесуар");

        DB_FreeResultSet(this.dbResult);
    }
    return 1;
}

stock Acce.Use (playerid, this.ID) {
//    new this.ID = GetPVarInt(playerid, #this.EditAcce);
    new buffer[144];

    format(buffer, sizeof(buffer), "SELECT * FROM accessories WHERE owner_id=%d", PlayerInfo[playerid][pID]);
    new DBResult:this.DBResult = db_query(this.dbHandle, buffer);

    for (new i = 0; i < this.ID; i++) {
        DB_SelectNextRow(this.DBResult);
    }

    new id, index, modelid, boneid, Float:pos[9], st_acce;

    id = DB_GetFieldIntByName(this.DBResult, "id");
    pos[0] = DB_GetFieldFloatByName(this.DBResult, "x");
    pos[1] = DB_GetFieldFloatByName(this.DBResult, "y");
    pos[2] = DB_GetFieldFloatByName(this.DBResult, "z");
    pos[3] = DB_GetFieldFloatByName(this.DBResult, "rx");
    pos[4] = DB_GetFieldFloatByName(this.DBResult, "ry");
    pos[5] = DB_GetFieldFloatByName(this.DBResult, "rz");
    pos[6] = DB_GetFieldFloatByName(this.DBResult, "sx");
    pos[7] = DB_GetFieldFloatByName(this.DBResult, "sy");
    pos[8] = DB_GetFieldFloatByName(this.DBResult, "sz");
    index = DB_GetFieldIntByName(this.DBResult, "index");
    modelid = DB_GetFieldIntByName(this.DBResult, "modelid");
    boneid = DB_GetFieldIntByName(this.DBResult, "boneid");
    st_acce = DB_GetFieldIntByName(this.DBResult, "st_acce");

    DB_FreeResultSet(this.DBResult);

    if (st_acce == 1) {
        DB_FreeResultSet(this.DBResult);
        if (IsPlayerAttachedObjectSlotUsed(playerid, index)) RemovePlayerAttachedObject(playerid, index);

        buffer[0] = EOS;
        format(buffer, sizeof(buffer), "UPDATE `accessories` SET `st_acce` = '0' WHERE `id` = '%d'", id);
        DB_FreeResultSet(DB_ExecuteQuery(this.dbHandle, buffer));

        SendClientMessage(playerid, -1, " Аксессуар снят");
        return;
    }

    if (IsPlayerAttachedObjectSlotUsed(playerid, index)) {
        SendClientMessage(playerid, COLOR_GREY, " У вас занят текущий слот");
        return;
    }

    SetPlayerAttachedObject(playerid, index, modelid, boneid, pos[0], pos[1], pos[2], pos[3], pos[4], pos[5], pos[6], pos[7], pos[8]);
    SendClientMessage(playerid, -1, " Аксессуар надет");

    format(buffer, sizeof(buffer), "UPDATE `accessories` SET `st_acce` = '1' WHERE `id` = '%d'", id);
    DB_FreeResultSet(DB_ExecuteQuery(this.dbHandle, buffer));

    DB_FreeResultSet(this.DBResult);
    // EditAttachedObject(playerid, index);
    // SetPVarInt(playerid, #this.CreateAcce, id);
}

stock Acce.AutoUse (playerid) {
    new buffer[144];

    format(buffer, sizeof(buffer), "SELECT * FROM accessories WHERE `owner_id` = '%d' AND `st_acce` = '1'", PlayerInfo[playerid][pID]);
    new DBResult:this.DBResult = db_query(this.dbHandle, buffer);

    if (!DB_GetRowCount(this.DBResult)) {
        DB_FreeResultSet(this.DBResult);
        return;
    }

    for (new i = 0; i < DB_GetRowCount(this.DBResult); i++) {
        new index, modelid, boneid, Float:pos[9];
        pos[0] = DB_GetFieldFloatByName(this.DBResult, "x");
        pos[1] = DB_GetFieldFloatByName(this.DBResult, "y");
        pos[2] = DB_GetFieldFloatByName(this.DBResult, "z");
        pos[3] = DB_GetFieldFloatByName(this.DBResult, "rx");
        pos[4] = DB_GetFieldFloatByName(this.DBResult, "ry");
        pos[5] = DB_GetFieldFloatByName(this.DBResult, "rz");
        pos[6] = DB_GetFieldFloatByName(this.DBResult, "sx");
        pos[7] = DB_GetFieldFloatByName(this.DBResult, "sy");
        pos[8] = DB_GetFieldFloatByName(this.DBResult, "sz");
        index = DB_GetFieldIntByName(this.DBResult, "index");
        modelid = DB_GetFieldIntByName(this.DBResult, "modelid");
        boneid = DB_GetFieldIntByName(this.DBResult, "boneid");

        SetPlayerAttachedObject(playerid, index, modelid, boneid, pos[0], pos[1], pos[2], pos[3], pos[4], pos[5], pos[6], pos[7], pos[8]);

        DB_SelectNextRow(this.DBResult);
    }

    DB_FreeResultSet(this.DBResult);
}

stock Acce.Edit (playerid, this.ID) {
    //new this.ID = GetPVarInt(playerid, #this.EditAcce);
    new buffer[144];

    format(buffer, sizeof(buffer), "SELECT * FROM accessories WHERE id=%d", this.ID);
    new DBResult:this.DBResult = DB_ExecuteQuery(this.dbHandle, buffer);

    new index, modelid, boneid, Float:pos[9], st_acce;

    pos[0] = DB_GetFieldFloatByName(this.DBResult, "x");
    pos[1] = DB_GetFieldFloatByName(this.DBResult, "y");
    pos[2] = DB_GetFieldFloatByName(this.DBResult, "z");
    pos[3] = DB_GetFieldFloatByName(this.DBResult, "rx");
    pos[4] = DB_GetFieldFloatByName(this.DBResult, "ry");
    pos[5] = DB_GetFieldFloatByName(this.DBResult, "rz");
    pos[6] = DB_GetFieldFloatByName(this.DBResult, "sx");
    pos[7] = DB_GetFieldFloatByName(this.DBResult, "sy");
    pos[8] = DB_GetFieldFloatByName(this.DBResult, "sz");

    index = DB_GetFieldIntByName(this.DBResult, "index");
    modelid = DB_GetFieldIntByName(this.DBResult, "modelid");
    boneid = DB_GetFieldIntByName(this.DBResult, "boneid");
    st_acce = DB_GetFieldIntByName(this.DBResult, "st_acce");

    DB_FreeResultSet(this.DBResult);

    if (st_acce == 0) {
        SendClientMessage(playerid, COLOR_GREY, " У вас снят этот акссесуар");
        return;
    }

    if (IsPlayerAttachedObjectSlotUsed(playerid, index)) RemovePlayerAttachedObject(playerid, index);

    SetPlayerAttachedObject(playerid, index, modelid, boneid, pos[0], pos[1], pos[2], pos[3], pos[4], pos[5], pos[6], pos[7], pos[8]);
    EditAttachedObject(playerid, index);
}

stock Acce.OnPlayerEditAttachedObject (playerid, response, index, modelid, boneid, Float:fOffsetX, Float:fOffsetY, Float:fOffsetZ, Float:fRotX, Float:fRotY, 
Float:fRotZ, Float:fScaleX, Float:fScaleY, Float:fScaleZ) {
    if (!response)
    {
        if (IsValidPVar(playerid, #this.CreateAcce)) {
            DeletePVar(playerid, #this.CreateAcce);
            if (IsPlayerAttachedObjectSlotUsed(playerid, index)) RemovePlayerAttachedObject(playerid, index);
            SendClientMessage(playerid, COLOR_GREY, " Вы успешно отменили установку акссесуара");
            return 1;
        }
    }

    else if (response)
    {
        if (IsValidPVar(playerid, #this.CreateAcce)) {
            if (fScaleX > 2.0 || fScaleY > 2.0 || fScaleZ > 2.0) {
                if (IsPlayerAttachedObjectSlotUsed(playerid, index)) {
                    RemovePlayerAttachedObject(playerid, index);
                    SendClientMessage(playerid, COLOR_GREY, " Вы не можете установить акссесуар");
                    return 1;
                }
            }

            new this.ID = GetPVarInt(playerid, #this.CreateAcce);
            new buffer[512];

            format(buffer, sizeof buffer, "UPDATE accessories SET `boneid` = '%d', `modelid` = '%d', `x` = '%f', `y` = '%f', `z` = '%f', `rx` = '%f', `ry` = '%f', `rz` = '%f', `sx` = '%f', `sy` = '%f', `sz` = '%f',\
                st_acce=1 WHERE id=%d", boneid, modelid, fOffsetX, fOffsetY, fOffsetZ, fRotX, fRotY, fRotZ, fScaleX, fScaleY, fScaleZ, this.ID);
            DB_FreeResultSet(DB_ExecuteQuery(this.dbHandle, buffer));

            SendClientMessage(playerid, COLOR_GREEN, " [Информация] "STR_WHITE"Вы успешно установили акссесуар");
            return 1;
        }
    }
    return 0;
}

stock Acce.SearchBone (type) {
    switch (type)
    {
        case ACCESSORIES_TYPE_BALACLAVA, ACCESSORIES_TYPE_GLASSES, ACCESSORIES_TYPE_ACAP, ACCESSORIES_TYPE_CAP, ACCESSORIES_TYPE_HAT, ACCESSORIES_TYPE_MASK: 
            return 2;

        case ACCESSORIES_TYPE_BELT:
            return 7;

        case ACCESSORIES_TYPE_BAG: 
            return 1;

        case ACCESSORIES_TYPE_BAGS:
            return 8;
    }
    return 1;
}

stock Acce.GiveSQL (playerid, index, modelid, boneid, Float:fOffsetX, Float:fOffsetY, Float:fOffsetZ, Float:fRotX, Float:fRotY, 
Float:fRotZ, Float:fScaleX, Float:fScaleY, Float:fScaleZ) {
    new buffer[512];
    format(buffer, sizeof(buffer), "INSERT INTO accessories (`owner_id`, `index`, `modelid`, `boneid`, `x`, `y`, `z`, `rx`, `ry`, `rz`, `sx`, `sy`, `sz`) \
        VALUES ('%d', '%d', '%d', '%d', '%f', '%f', '%f', '%f', '%f', '%f', '%f', '%f', '%f')",
    PlayerInfo[playerid][pID], index, modelid, boneid, fOffsetX, fOffsetY, fOffsetZ, fRotX, fRotY, fRotZ, fScaleX, fScaleY, fScaleZ);
    printf(buffer);
    DB_FreeResultSet(DB_ExecuteQuery(this.dbHandle, buffer));
}

stock Acce.CefBuy (playerid) {
    new buffer[512], this.Quantity;

    for (new i = 0; i < sizeof(this.Info); i++) {
        if (!this.Info[i][this.State]) continue;
        if (this.Info[i][this.Type] != ACCESSORIES_TYPE_ACAP) continue;

        this.Quantity++;
        format(buffer, sizeof(buffer), "%s[%i,\"%s\",%i,0,%i],", buffer, this.Info[i][this.ModelID], this.Info[i][this.Name], this.Info[i][this.MPrice], this.Quantity);
    }

    strdel(buffer, strlen(buffer) - 1, strlen(buffer));
    SetPVarInt(playerid, #this.Page, 1);

    format(buffer, sizeof(buffer), "[[[\"Шапка\", 1],[\"Балаклава\", 2],[\"Сумка\", 3],[\"Рюкзак\", 4], [\"Маска\", 5],[\"Кепка\", 6],[\"Шляпа\", 7],[\"Очки\", 8],[\"Ремень\", 9],[\"Часы\", 10]],[%s]]",  buffer);
    SendClientEventWithArg(playerid, s_CefInterface, t_CefShow, 2, "Accessories", buffer);
}

stock Acce.OnChangeCategory (playerid, event) {
    SetPVarInt(playerid, #this.Page, event);

    static buffer[1500];
    new this.Quantity;

    for(new i = 0; i < sizeof(this.Info); i++) {
        if (!this.Info[i][this.State]) continue;
        if (this.Info[i][this.Type] != event) continue;

        this.Quantity++;
        format(buffer, sizeof(buffer), "%s[%i, \"%s\",%i,0,%i],", buffer, this.Info[i][this.ModelID], this.Info[i][this.Name], this.Info[i][this.MPrice], this.Quantity);
        this.ListItem[playerid][this.Quantity] = i;
    }

    strdel(buffer, strlen(buffer)-1, strlen(buffer));

    static buff[2000];
    format(buff, sizeof(buff), "interface('Accessories').setItems('[%s]')", buffer);
    SendClientEvent(playerid, s_CefInterface, t_CefUpdate, 1, buff);
    buff[0] = EOS;
    buffer[0] = EOS;
}

stock Acce.OnPlayerClickItem(playerid, item) {
    if (!IsValidPVar(playerid, #this.Page)) return;

    new id = this.ListItem[playerid][item];
    SetPVarInt(playerid, #this.BuyId, id);

    new buffer[256];
    format(buffer, sizeof(buffer), "Вы действительно хотите купить аксессуар \"%s\"?\nСтоимость: "STR_YELLOW"%d рублей", this.Info[id][this.Name], this.Info[id][this.MPrice]);
    ShowPlayerDialog(playerid, DIALOG_BUY_ACCE, DIALOG_STYLE_MSGBOX, "Подтверждение", buffer, "Купить", "Отмена");
}

stock Acce.OnCloseInterface (playerid) {
    SendClientEvent(playerid, s_CefInterface, t_CefHide, 1, "Accessories");

    SetPlayerPos(playerid, 1238.4243,-2777.9377,1001.0000);
	SetPlayerFacingAngle(playerid, 90.0);
    SetPlayerInterior(playerid, 4);
    SetPlayerVirtualWorld(playerid, 10);
	SetCameraBehindPlayer(playerid);

    for (new i = 0; i < 10; i++) {
        if (IsPlayerAttachedObjectSlotUsed(playerid, i)) {
            RemovePlayerAttachedObject(playerid, i);
        }
    }

    CancelEdit(playerid);
    TogglePlayerControllable(playerid, true);
}

CALLBACK Acce.ShowAcceDialog (playerid) {
    new buffer[128];
    format(buffer, sizeof(buffer), "SELECT * FROM accessories WHERE owner_id='%d'", PlayerInfo[playerid][pID]);
    new DBResult:this.DBResult = DB_ExecuteQuery(this.dbHandle, buffer);

    new rows = DB_GetRowCount(this.DBResult);

    if (!rows) {
        DB_FreeResultSet(this.DBResult);
        return -1;
    }

    for (new i = 0; i < DB_GetRowCount(this.DBResult); i++) {
        new modelid;
        modelid = DB_GetFieldIntByName(this.DBResult, "modelid");

        for (new j = 0; j < sizeof(this.Info); j++) {
            if (this.Info[j][this.ModelID] != modelid) continue;

            AddDialogListitem(playerid, this.Info[j][this.Name]);

            Dialog.SetCallbackMenu(playerid, i, "_acceItemResponse");
            Dialog.SetCommandText(playerid, i, "null");
            Dialog.SetErrorCallbackText(playerid, i, "null");
            Dialog.SetSendCommandStatus(playerid, i, false);
            Dialog.SetSubMenu(playerid, i, "acceMenu");
        }

        DB_SelectNextRow(this.DBResult);
    }
    return 1;
}

CALLBACK Acce.ItemResponse (playerid) {
    new buffer[128];
    format(buffer, sizeof(buffer), "SELECT * FROM accessories WHERE owner_id='%d'", PlayerInfo[playerid][pID]);
    new DBResult:this.DBResult = DB_ExecuteQuery(this.dbHandle, buffer);

    new rows = DB_GetRowCount(this.DBResult);

    if (!rows) {
        DB_FreeResultSet(this.DBResult);
        return 0;
    }

    for (new i = 0; i < GetPVarInt(playerid, #Dialog.listItem); i++) {
        DB_SelectNextRow(this.DBResult);
    }

    buffer[0] = EOS;

    new id, index, modelid, boneid, Float:pos[9];

    id = DB_GetFieldIntByName(this.DBResult, "id");
    pos[0] = DB_GetFieldIntByName(this.DBResult, "x");
    pos[1] = DB_GetFieldIntByName(this.DBResult, "y");
    pos[2] = DB_GetFieldIntByName(this.DBResult, "z");
    pos[3] = DB_GetFieldFloatByName(this.DBResult, "rx");
    pos[4] = DB_GetFieldFloatByName(this.DBResult, "ry");
    pos[5] = DB_GetFieldFloatByName(this.DBResult, "rz");
    pos[6] = DB_GetFieldFloatByName(this.DBResult, "sx");
    pos[7] = DB_GetFieldFloatByName(this.DBResult, "sy");
    pos[8] = DB_GetFieldFloatByName(this.DBResult, "sz");
    index = DB_GetFieldIntByName(this.DBResult, "index");
    modelid = DB_GetFieldIntByName(this.DBResult, "modelid");
    boneid = DB_GetFieldIntByName(this.DBResult, "boneid");

    DB_FreeResultSet(this.DBResult);

    if (pos[0] == 0.0 && pos[1] == 0.0 && pos[2] == 0.0 && pos[3] == 0.0 && pos[4] == 0.0 
    && pos[5] == 0.0 && pos[6] == 0.0 && pos[7] == 0.0 && pos[8] == 0.0) {
        if (IsPlayerAttachedObjectSlotUsed(playerid, index)) {
            SendClientMessage(playerid, COLOR_GREY, " У вас занят текущий слот");
            return 0;
        }

        SetPlayerAttachedObject(playerid, index, modelid, boneid);
        EditAttachedObject(playerid, index);
        SetPVarInt(playerid, #this.CreateAcce, id);

        Dialog.Clean(playerid);
        return 0;
    }
    return 1;
}

stock Acce.DonateAccessory (playerid)
{
    new this.Quantity, str[128];

    for (new i = 0; i < sizeof(this.Info); i++)
    {
        if (this.Info[i][this.State]) continue;

        this.Quantity++;
        format(str, sizeof(str), "[%i] %s\t%i PCoins", this.Quantity, this.Info[i][this.Name], this.Info[i][this.PPrice]);
        AddDialogListitem(playerid, str);

        Dialog.SetCallbackMenu(playerid, this.Quantity, "null");
        Dialog.SetCommandText(playerid, this.Quantity, "null");
        Dialog.SetErrorCallbackText(playerid, this.Quantity, "null");
        Dialog.SetSendCommandStatus(playerid, this.Quantity, false);
        Dialog.SetSubMenu(playerid, this.Quantity, "donateAcceConfirm");
    }
}