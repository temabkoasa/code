
#if defined SRadar
    #undef SRadar   
#endif
#define SRadar. _sradar

#if defined this
    #undef this   
#endif
#define this. _sradar

#define SRADAR_INVALID_DIALOG   0
#define SRADAR_MAX_RADARS        30
#define SRADAR_SET_MIN_SPEED    60
#define SRADAR_SET_MAX_SPEED    120
#define SRADAR_OBJECT_MODELID   18880

private DB:SRadar.dbHandle;
private DBResult:SRadar.DBResult;

enum SRadar.structRadarInfo
{
	SRadar.sphere[2],
	SRadar.limitSpeed,
	SRadar.terrain[16],
	SRadar.object,
	Text3D:SRadar.text[64],
	Float:SRadar.pos[3],
    SRadar.creater_name[MAX_PLAYER_NAME]
}

private SRadar.Info[SRADAR_MAX_RADARS][SRadar.structRadarInfo];
private SRadar.totalRadar;



stock SRadar.Init() {
    if ((this.dbHandle = DB_Open("sradar.db")) != DB:0) {
        printf("[SQLite] Add table=sradar.db, handle=%d", _:this.dbHandle);

        DB_FreeResultSet(DB_ExecuteQuery(this.dbHandle, 
			"CREATE TABLE IF NOT EXISTS `tickets` (`id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, `uid` INTEGER, `count` INTEGER)"));
        
    }
}

stock SRadar.cmd_sradar(playerid, params[]) {
    new key[10];
    cmdparam(key, params);

    if (!strlen(key))
    {
        SendClientMessage(playerid, -1, " Ââåäèòå: /sradar [key]. Äîñòóïíûå êëþ÷è: set, edit, del, ticket, info");
        return 2;
    }

    if (!strcmp(key, "set", true))
    {
        new param[2][64];
        cmdparam(param[0], params);
        cmdparam(param[1], params);
        
        if (!strlen(param[0]) || !strlen(param[1]))
        {
            SendClientMessage(playerid, -1, " Ââåäèòå: /sradar set [slimit] [íàçâàíèå ìåñòíîñòè]");
            return 2;
        }

		if (this.totalRadar >= SRADAR_MAX_RADARS) {
            SendClientMessage(playerid, COLOR_GREY, " Ïðåâûøåí ëèìèò êîëè÷åñòâà ðàäàðîâ");
            return 2;
        }

        new slimit = strval(param[0]);
		if (slimit < SRADAR_SET_MIN_SPEED || slimit > SRADAR_SET_MAX_SPEED) {
            SendClientMessage(playerid, COLOR_GREY, " Íåâåðíîå çíà÷åíèå ëèìèòà ñêîðîñòè");
            return 2;
        }
		
		new Float: position[4];
		GetPlayerPos(playerid, position[0], position[1], position[2]);
		GetPlayerFacingAngle(playerid, position[3]);
		
		if (IsValidDynamicObject(this.Info[this.totalRadar][this.object]))
            DestroyDynamicObject(this.Info[this.totalRadar][this.object]);

		if (IsValidDynamic3DTextLabel(this.Info[this.totalRadar][this.text]))
            DestroyDynamic3DTextLabel(this.Info[this.totalRadar][this.text]);

		if (IsValidDynamicArea(this.Info[this.totalRadar][this.sphere][0]))
            DestroyDynamicArea(this.Info[this.totalRadar][this.sphere][0]);

        if (IsValidDynamicArea(this.Info[this.totalRadar][this.sphere][1]))
            DestroyDynamicArea(this.Info[this.totalRadar][this.sphere][1]);
	
        new buffer[128];
        format(buffer, sizeof(buffer), "Ðàäàð ñêîðîñòè ¹%d íà %d (êì/÷){FFFFFF}\nÏîñòàâèë: {00FF00}%s", this.totalRadar, 
            slimit, getName(playerid));

    	this.Info[this.totalRadar][this.sphere][0] = CreateDynamicSphere(position[0], position[1], position[2], 100.0, 0, 0);
        this.Info[this.totalRadar][this.sphere][1] = CreateDynamicSphere(position[0], position[1], position[2], 50.0, 0, 0);

        this.Info[this.totalRadar][this.text] = CreateDynamic3DTextLabel(buffer, COLOR_ORANGE, position[0], position[1], position[2]+1.0, 15.0);
        this.Info[this.totalRadar][this.object] = CreateDynamicObject(SRADAR_OBJECT_MODELID, position[0], position[1], position[2]-2.5, 0.0, 0.0, position[3]);
        
        this.Info[this.totalRadar][this.limitSpeed] = slimit;

        this.Info[this.totalRadar][this.pos][0] = position[0];
        this.Info[this.totalRadar][this.pos][1] = position[1];
        this.Info[this.totalRadar][this.pos][2] = position[2];

        this.Info[this.totalRadar][this.terrain] = params[1];

        this.Info[this.totalRadar][this.creater_name] = getName(playerid);

        buffer[0] = EOS;

        format(buffer, sizeof (buffer), " [Speed Cam] %s ñîçäàë êàìåðó íà %d êì/÷ â ìåñòíîñòè %s", getName(playerid), slimit, param[1]);
        SendMessageToPoliceTeam(COLOR_GBLUE, buffer);

        buffer[0] = EOS;

        this.totalRadar++;
		return 1;
    }

    if (!strcmp(key, "edit", true))
    {
        new radarId_str[11];
        cmdparam(radarId_str, params);

        if (!strlen(radarId_str))
        {
            SendClientMessage(playerid, -1, " Ââåäèòå: /sradar edit [ID ðàäàðà]");
            return 2;
        }

        new radarId = strval(radarId_str);

        if (radarId > this.totalRadar) return SendClientMessage (playerid, COLOR_GREY, " Äàííîãî ðàäàðà íå ñóùåñòâóåò");

        if (!IsPlayerInRangeOfPoint(playerid, 100.0, this.Info[radarId][this.pos][0], this.Info[radarId][this.pos][1], this.Info[radarId][this.pos][2])) 
            return SendClientMessage(playerid, COLOR_GREY, " Ïîáëèçîñòè íåò ðàäàðà.");
        
        EditDynamicObject (playerid, this.Info[radarId][this.object]);

        Streamer_Update(playerid);
		
        return 1;
    }

	if (!strcmp(key, "del", true))
	{
        if (!this.totalRadar) {
            SendClientMessage(playerid, COLOR_GREY, " Íà êàðòå íå áûë ñîçäàí íè îäíîãî ðàäàð ñêîðîñòè");
            return 2;
        }

		for (new i; i < this.totalRadar; i++)
		{
			if (IsPlayerInRangeOfPoint(playerid, 20.0, this.Info[i][this.pos][0], this.Info[i][this.pos][1], this.Info[i][this.pos][2]))
			{
				if (IsValidDynamicArea(this.Info[i][this.sphere][0]))
                    DestroyDynamicArea(this.Info[i][this.sphere][0]);

                if (IsValidDynamicArea(this.Info[i][this.sphere][1]))
                    DestroyDynamicArea(this.Info[i][this.sphere][1]);

				if (IsValidDynamic3DTextLabel(this.Info[i][this.text]))
                    DestroyDynamic3DTextLabel(this.Info[i][this.text]);
                
				if (IsValidDynamicObject(this.Info[i][this.object]))
                    DestroyDynamicObject(this.Info[i][this.object]);
				
				this.totalRadar--;

                new buffer[128];

				buffer[0] = EOS;

                format(buffer, sizeof (buffer), " [Speed Cam] %s óäàëèë êàìåðó â ðàéîíå %s", getName(playerid), this.Info[i][this.terrain]);
                SendMessageToPoliceTeam(COLOR_GBLUE, buffer);

				SendClientMessage(playerid, -1, " Âû óñïåøíî óäàëèëè ðàäàðû");
                return 1;
			}
            else SendClientMessage(playerid, COLOR_GREY, " Â ðàäèóñå 20ì íåò íè îäíîãî ðàäàðà");
		}

        
        return 2;
	}

	if (!strcmp(key, "info", true))
	{
		ShowPlayerDialog(playerid, 2, DIALOG_STYLE_LIST, "Ïàíåëü ðàäàðîâ", "Àêòèâíûå êàìåðû", "Âûáðàòü", "Îòìåíà");
        return 1;
	}

    if (!strcmp(key, "ticket", true))
    {
        new buffer[128];
        
        format (buffer, sizeof (buffer), "SELECT * FROM `tickets` WHERE `uid` = '%d'", 1);

        this.DBResult = DB_ExecuteQuery (this.dbHandle, buffer);

        buffer[0] = EOS;

        if (!DB_GetRowCount(this.DBResult)) return SendClientMessage(playerid, COLOR_GREY, " Ó âàñ íåò øòðàôîâ");
        
        format(buffer, sizeof (buffer), "{3399FF}ID\tÐàçìåð\n");

        new str[32], id, count;

        for(new i = 0; i < DB_GetRowCount(this.DBResult); i++)
        {
            id = DB_GetFieldIntByName(this.DBResult, "id");
            count = DB_GetFieldIntByName(this.DBResult, "count");

            format(str, sizeof str, "%d\t%d ðóáëåé\n", id, count);
            strcat(buffer, str);

            DB_SelectNextRow(this.DBResult);
        }
        
        ShowPlayerDialog (playerid, 3, DIALOG_STYLE_TABLIST_HEADERS, "Àêòèâíûå êàìåðû", buffer, "Âûéòè", "");

    }
    return 2;
}

CALLBACK SRadar.OnPlayerEditDynamicObject(playerid, objectid, EDIT_RESPONSE:response, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz)
{
    if (response == EDIT_RESPONSE_CANCEL) return 1;
    if (response == EDIT_RESPONSE_FINAL)
    {
        for (new i = 0; i < this.totalRadar; i++)
        {
            if (IsPlayerInRangeOfPoint(playerid, 20.0, this.Info[i][this.pos][0], this.Info[i][this.pos][1], this.Info[i][this.pos][2]))
            {
                new radarId = i;

                if (IsValidDynamicObject(this.Info[radarId][this.object]))
                    DestroyDynamicObject(this.Info[radarId][this.object]);

                if (IsValidDynamic3DTextLabel(this.Info[radarId][this.text]))
                    DestroyDynamic3DTextLabel(this.Info[radarId][this.text]);

				if (IsValidDynamicArea(this.Info[radarId][this.sphere][0]))
                    DestroyDynamicArea(this.Info[radarId][this.sphere][0]);

                if (IsValidDynamicArea(this.Info[radarId][this.sphere][1]))
                    DestroyDynamicArea(this.Info[radarId][this.sphere][1]);

                this.Info[radarId][this.pos][0] = x;
                this.Info[radarId][this.pos][1] = y;
                this.Info[radarId][this.pos][2] = z;
                
                new buffer[128];
                format(buffer, sizeof(buffer), "Ðàäàð ñêîðîñòè ¹%d íà %d (êì/÷){FFFFFF}\nÏîñòàâèë: {00FF00}%s", radarId,
                    this.Info[radarId][this.limitSpeed], getName(playerid));

                this.Info[radarId][this.object] = CreateDynamicObject(SRADAR_OBJECT_MODELID, x, y, z-2.5, 0.0, 0.0, rx);
                this.Info[radarId][this.text] = CreateDynamic3DTextLabel(buffer, COLOR_ORANGE, x, y, z+1.0, 15.0);
                
                this.Info[radarId][this.sphere][0] = CreateDynamicSphere(x, y, z, 100.0, 0, 0);
                this.Info[radarId][this.sphere][1] = CreateDynamicSphere(x, y, z, 50.0, 0, 0);
                
                SendClientMessage(playerid, -1, " Âû óñïåøíî èçìåíèëè ïîçèöèè êàìåðû");
            }
        }
    }
    
    return 1;
}

CALLBACK SRadar.OnEnterDynamicArea(playerid, areaid)
{
    if (GetPlayerState(playerid) == PLAYER_STATE_DRIVER) {
        for (new i; i < this.totalRadar; i++)
        {
            if (areaid == this.Info[i][this.sphere][0])
                SendClientMessage(playerid, COLOR_YELLOW, " SMS: Íà äàííîì ó÷àñòêå äîðîãè îãðàíè÷èòåëü ñêîðîñòè, áóäü âíèìàòåëåí è ñíèçü ñêîðîñòü!");

            if (areaid == this.Info[i][this.sphere][1])
            {
                if (GetVehicleSpeed(GetPlayerVehicleID(playerid)) > this.Info[i][this.limitSpeed])
                {
                    new formula = 20000*(GetVehicleSpeed(GetPlayerVehicleID(playerid))-this.Info[i][this.limitSpeed]);
                    new buffer[144];

                    format(buffer, sizeof(buffer), "INSERT INTO `tickets` (`uid`, `count`) VALUES ('1', '%d')", formula);
                    DB_FreeResultSet(DB_ExecuteQuery(this.dbHandle, buffer));

                    buffer[0] = EOS;

                    format(buffer, sizeof(buffer), " SMS: Âàø àâòîìîáèëü íàðóøèë ñêîðîñòíîé ðåæèì. Øòðàô â ðàçìåðå %d ðóáëåé", formula);
                    SendClientMessage(playerid, COLOR_YELLOW, buffer);
                    return 1;
                }
            }
        }
    }
	return 1;
}

CALLBACK SRadar.OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    if (dialogid == 3)
    {
        if (!response)
            return 1;
        
        new buffer[256], count;

        new id = strval(inputtext);

        format(buffer, sizeof buffer, "SELECT * FROM `tickets` WHERE `id`='%d'", id);
        this.DBResult = DB_ExecuteQuery(this.dbHandle, buffer);

        if (DB_GetRowCount(this.DBResult)) count = DB_GetFieldIntByName(this.DBResult, "count");
        else return 1;

        SetPVarInt(playerid, #this.ticketCount, count);
        SetPVarInt(playerid, #this.ticketId, id);

        format(buffer, sizeof buffer, "Âû äåéñòâèòåëüíî õîòèòå îïëàòèòü øòðàô ¹%d\nÎáùàÿ ñòîèìîñòü ñîñòàâëÿåò %d ðóáëåé.", id, count);
        ShowPlayerDialog(playerid, 4, DIALOG_STYLE_MSGBOX, "Îïëàòà øòðàôà", buffer, "Ïðîäîëæèòü", "Îòìåíà");
    }
    if (dialogid == 4)
    {
        if(!response) return 1;

        if (!IsValidPVar(playerid, #this.ticketCount))
            return 1;

        if (!IsValidPVar(playerid, #this.ticketId))
            return 1;

        new value = GetPVarInt(playerid, #this.ticketCount);
        new id = GetPVarInt(playerid, #this.ticketId);

        if (GetPlayerMoney(playerid) < value) 
            return SendClientMessage(playerid, COLOR_GREY, " Ó âàñ íåäîñòàòî÷íî äåíåã");

        GivePlayerMoney(playerid, -value);
        SendFormatMsg(playerid, COLOR_GREEN, " [Èíôîðìàöèÿ] "STR_WHITE"Âû óñïåøíî îïëàòèëè øòðàô â ðàçìåðå %d ðóáëåé", value);

        new buffer[128];

        format(buffer, sizeof buffer, "DELETE FROM `tickets` WHERE `id` = '%d'", id);
        DB_ExecuteQuery(this.dbHandle, buffer);

        DeletePVar(playerid, #this.ticketCount);
        DeletePVar(playerid, #this.ticketId);

    }
	if (dialogid == 2)
	{
        if (!response) 
            return 1;
            
		switch (listitem)
		{
			case 0:
			{
				if (this.totalRadar == 0) {
                    SendClientMessage(playerid, COLOR_GREY, " Ñïèñîê ïóñò");
                    return 1;
                }

                new str[64];
                static buffer[512];
                buffer[0] = EOS;

				format(buffer, sizeof (buffer), "{3399FF}Êòî ïîñòàâèë\tËèìèò ñêîðîñòè\tÌåñòíîñòü\n");

                for (new i = 0; i < this.totalRadar; i++)
				{
					format(str, sizeof(str), "%s\t%d\t%s\n", this.Info[i][this.creater_name], this.Info[i][this.limitSpeed], this.Info[i][this.terrain]);
					strcat(buffer, str);
				}

				ShowPlayerDialog(playerid, 0, DIALOG_STYLE_TABLIST_HEADERS, "Àêòèâíûå êàìåðû", buffer, "Îòìåíà", "");

                buffer[0] = EOS;
			}
		}
	}
	return 1;
}

stock SendMessageToPoliceTeam(color, message[])
{
    for (new i; i < MAX_PLAYERS; i++)
    {
        if(!IsPlayerConnected(i)) return 1;
        // Íàïèñàòü ïðîâåðêó íà Ôðàêöèþ ïîëèöèè if(!g_player[i][pTeam] != 6) return 1;
        SendClientMessage(i, color, message);
    }
    return 1;
}

