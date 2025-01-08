
#if defined ENABLE_PRP_CORE
    #include "include/sampex/all.inc"
    #include "include/plugins/all.inc"
#else
    #include <open.mp>
    #include <streamer>

    #define private stock static
    #define IsValidPVar GetPVarType

    #pragma warning disable 239
    #pragma warning disable 214

    #define CALLBACK%0(%1) forward %0(%1);public %0(%1)

    stock GetVehicleSpeed (vehicleid)
    {
        new Float:x, Float:y, Float:z;

        GetVehicleVelocity(vehicleid, x,y,z);

        return floatround(VectorSize(x,y,z) * 100.0);
    }

    stock cmdparam(first[], full[], length=sizeof first)
    {
        new space = strfind(full, " ");
        if(space == -1)
        {
            format(first, length, full);
            strdel(full, 0, strlen(full));
        }
        else
        {
            strmid(first, full, 0, space, length);
            strdel(full, 0, space + 1);
        }
    }
#endif

new player_name[MAX_PLAYERS][MAX_PLAYER_NAME];

#define getName(%0) player_name[%0] 

#include "colors.inc"
#include "sradar.inc"


public OnPlayerConnect(playerid)
{
    GetPlayerName(playerid, player_name[playerid], MAX_PLAYER_NAME);
    return 1;
}

public OnGameModeInit() {
    SRadar.Init();
    return 1;
}

public OnPlayerCommandText(playerid, cmdtext[]) {
    new cmd[32];
    cmdparam(cmd, cmdtext);

    if (!strcmp(cmd, "/sradar", true)) {
        return SRadar.cmd_sradar(playerid, cmdtext);
    }

    if (!strcmp(cmd, "/veh", true)) {
        new Float: pos_player[3];
        GetPlayerPos(playerid, pos_player[0], pos_player[1], pos_player[2]);

        new veh_id = CreateVehicle(strval(cmdtext), pos_player[0], pos_player[1], pos_player[2], 0.0, 1, 1, 5000);
        PutPlayerInVehicle(playerid, veh_id, 0);
    }

    if (!strcmp(cmd, "/givemoney", true)) {
        new money = strval(cmdtext);
        GivePlayerMoney(playerid, money);
    }
    return 2;
}

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[]) {
    SRadar.OnDialogResponse(playerid, dialogid, response, listitem, inputtext);
    return 1;
}

public OnPlayerEnterDynamicArea(playerid, areaid) {
    SRadar.OnEnterDynamicArea(playerid, areaid);
    return 1;
}

public OnPlayerEditDynamicObject(playerid, objectid, EDIT_RESPONSE:response, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz) {
    SRadar.OnPlayerEditDynamicObject(playerid, objectid, response, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz);
}

stock SendFormatMsg(playerid, color, const text[], {Float, _}:...)
{
    static args, buffer[144];
    buffer[0] = EOS;

    if ((args = numargs()) == 3)
    {
        SendClientMessage(playerid, color, text);
        return 1;
    }

    while (--args >= 3)
    {
        #emit LCTRL 5
        #emit LOAD.alt args
        #emit SHL.C.alt 2
        #emit ADD.C 12
        #emit ADD
        #emit LOAD.I
        #emit PUSH.pri
    }
    #emit PUSH.S text
    #emit PUSH.C 144
    #emit PUSH.C buffer
    #emit PUSH.S 8
    #emit SYSREQ.C format
    #emit LCTRL 5
    #emit SCTRL 4

    SendClientMessage(playerid, color, buffer);

    #emit RETN

    buffer[0] = EOS;
    return 1;
}