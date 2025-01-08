/**
 * Module: Cars
 * Author: Riccardo_Morreti
 *
 * Point Role Play (c) Point-rp.ru 2023
 */


// #if defined Cars
//     #undef Cars
// #endif

#if defined this
    #undef this
#endif
#define this. _cars

private Cars.vehOwnerId[MAX_VEHICLES] = {INVALID_PLAYER_ID, ...};
private Cars.vehCountOwner[MAX_VEHICLES] = {0, ...};
private Cars.vehStringNumber[MAX_VEHICLES][64];
private Cars.vehStringRegion[MAX_VEHICLES][64];
private bool:Cars.vehKeyStatus[MAX_VEHICLES] = {true, ...};
private Cars.vehIndexAI[MAX_VEHICLES] = {-1, ...};
private Cars.rimsVehicle[MAX_VEHICLES];
private Cars.bodyKitVehicle[MAX_VEHICLES][11];
private Cars.tonerVehicle[MAX_VEHICLES][5];

stock Cars.GetVehicleIndexAI(vehicleid) return this.vehIndexAI[vehicleid];
stock Cars.SetVehicleIndexAI(vehicleid, index) {
    this.vehIndexAI[vehicleid] = index;
    return this.vehIndexAI[vehicleid];
}

stock Cars.SetParkingPosition(playerid, vehicleid, Float:vpos_x, Float:vpos_y, Float:vpos_z, Float:vpos_angle) {
    if (this.vehIndexAI[vehicleid] == -1) {
        return 0;
    }

    new model = GetVehicleModel(vehicleid);

    if (!model) {
        return 0;
    }

    new color1, color2;
    GetVehicleColours(vehicleid, color1, color2);

    new buffer[400];
    format(buffer, sizeof(buffer), "UPDATE `ownable_cars` SET `pos_x` = '%f', `pos_y` = '%f', `pos_z` = '%f', `angle` = '%f', `world` = '%d', `inter` = '%d' WHERE `id` = '%d'",
        vpos_x, vpos_y, vpos_z, vpos_angle, GetVehicleVirtualWorld(vehicleid), GetVehicleInterior(vehicleid), this.GetVehicleIndexAI(vehicleid));
    mysql_tquery(MySQLConnect, buffer);

    new vehId = CreateVehicle(model, vpos_x, vpos_y, vpos_z, vpos_angle, color1, color2, 86400);
    SetVehicleVirtualWorld(vehId, GetVehicleVirtualWorld(vehicleid));
    LinkVehicleToInterior(vehId, GetVehicleInterior(vehicleid));

    new bool:engine, bool:lights, bool:alarm, bool:doors, bool:bonnet, bool:boot, bool:objective;
    GetVehicleParamsEx(vehId, engine, lights, alarm, doors, bonnet, boot, objective);
    SetVehicleParamsEx(vehId, false, false, false, true, bonnet, boot, objective);

    Cars.SetNumberVehicleByVehId(vehId, this.vehStringNumber[vehicleid], this.vehStringRegion[vehicleid]);
    Cars.SetOwnableCar(vehId, PlayerInfo[playerid][pID]);
    Cars.SetVehicleIndexAI(vehId, this.GetVehicleIndexAI(vehicleid));
    Cars.SetVehicleCountOwner(vehId, 1);
    Cars.SetVehicleKeyStatus(vehId, this.GetVehicleKeyStatus(vehicleid));

    carMileage[vehId] = carMileage[vehicleid];
    carFuel[vehId] = carFuel[vehicleid];

    Tuning.Load(playerid, vehId);

    // -----
    Cars.SetNumberVehicleByVehId(vehicleid, "", "");
    Cars.SetOwnableCar(vehicleid, INVALID_PLAYER_ID);
    Cars.SetVehicleIndexAI(vehicleid, -1);
    Cars.SetVehicleCountOwner(vehicleid, 0);
    Cars.SetVehicleKeyStatus(vehicleid, true);

    carMileage[vehicleid] = 0.0;
    carFuel[vehicleid] = 0.0;

    if (IsValidVehicle(vehicleid)) {
        DestroyVehicle(vehicleid);
    }
    return 1;
}

stock Cars.GetPlayerNearVehicle(playerid) {
    for (new i = 0, j = GetVehiclePoolSize()+1; i < j; i++) {
        if (!IsValidVehicle(i)) continue;
        
        new Float:carPos[3];
        GetVehiclePos(i, PR_EXPAND_ARR3(carPos));

        if (IsPlayerInRangeOfPoint(playerid, 5.0, PR_EXPAND_ARR3(carPos))) {
            return i;
        }
    }

    return INVALID_VEHICLE_ID;
}

stock bool:Cars.GetVehicleKeyStatus(vehicleid) return this.vehKeyStatus[vehicleid];
stock bool:Cars.SetVehicleKeyStatus(vehicleid, bool:status) {
    this.vehKeyStatus[vehicleid] = status;
    return this.vehKeyStatus[vehicleid];
}

stock Cars.GetOwnableCarById(vehicleid) return this.vehOwnerId[vehicleid];
stock Cars.SetOwnableCar(vehicleid, owner_id) {
    this.vehOwnerId[vehicleid] = owner_id;
    return this.vehOwnerId[vehicleid];
}

stock Cars.FindPlayerOwnableCar(vehicleid) {
    foreach (i) {
        if (PlayerInfo[i][pID] == this.vehOwnerId[vehicleid]) {
            return i;
        }
    }

    return INVALID_PLAYER_ID;
}

stock Cars.GetVehicleCountOwner(vehicleid) return this.vehCountOwner[vehicleid];
stock Cars.SetVehicleCountOwner(vehicleid, count) { 
    this.vehCountOwner[vehicleid] = count;
    return this.vehCountOwner[vehicleid];
}

stock Cars.GetNumberVehicleByVehId(vehicleid, vnumber[]) {
    if (strlen(this.vehStringNumber[vehicleid]) && strcmp(this.vehStringNumber[vehicleid], "------")) format(vnumber, 64, "%s|%s", this.vehStringNumber[vehicleid], this.vehStringRegion[vehicleid]);
}

stock Cars.SetNumberVehicleByVehId(vehicleid, vnumber[], vregion[]) {
    format(this.vehStringNumber[vehicleid], 64, "%s", vnumber);
    format(this.vehStringRegion[vehicleid], 64, "%s", vregion);
}

stock Cars.GetNameVehicleByModelId(id, vehName[])
{
    for(new i; i < sizeof(VehicleInfo); i++)
    {
        if (id == VehicleInfo[i][vModelId])
        {
            format(vehName, 64, "%s", VehicleInfo[i][vName]);
            break;
        }
    }
}

stock Cars.CheckCarPriceByModelId(id)
{
    new price;
    for(new i; i < sizeof(VehicleInfo); i++)
    {
        if (id == VehicleInfo[i][vModelId])
        {
            price = VehicleInfo[i][vPrice];
            return price;
        }
    }
    return price;
}

stock Cars.CheckCarShopByModelId(id)
{
    new status;
    for(new i; i < sizeof(VehicleInfo); i++)
    {
        if (id == VehicleInfo[i][vModelId])
        {
            status = VehicleInfo[i][vShop];
            return status;
        }
    }
    return status;
}

stock custom_CreateVehicle(modelid, Float:x, Float:y, Float:z, Float:angle, color1, color2, respawn_delay, bool:addSiren = false)
{
    new veh = CreateVehicle(modelid, x, y, z, angle, color1, color2, respawn_delay, addSiren);

    vehColors[veh][0] = color1;
    vehColors[veh][1] = color2;

	this.SetDefaultSettings(veh);
    return veh;
}
#if    defined    _ALS_CreateVehicle
    #undef    CreateVehicle
#else
    #define    _ALS_CreateVehicle
#endif
#define    CreateVehicle(%0)    custom_CreateVehicle(%0)

stock custom_AddStaticVehicleEx(modelid, Float:x, Float:y, Float:z, Float:angle, color1, color2, respawn_delay, bool:addsiren = false)
{
    new veh = AddStaticVehicleEx(modelid, x, y, z, angle, color1, color2, respawn_delay, addsiren);

    vehColors[veh][0] = color1;
    vehColors[veh][1] = color2;

	this.SetDefaultSettings(veh);
    return veh;
}
#if    defined    _ALS_AddStaticVehicleEx
    #undef    AddStaticVehicleEx
#else
    #define    _ALS_AddStaticVehicleEx
#endif
#define    AddStaticVehicleEx(%0)    custom_AddStaticVehicleEx(%0)

stock custom_AddStaticVehicle(modelid, Float:x, Float:y, Float:z, Float:angle, color1, color2)
{
    new veh = AddStaticVehicle(modelid, x, y, z, angle, color1, color2);

    vehColors[veh][0] = color1;
    vehColors[veh][1] = color2;

    this.SetDefaultSettings(veh);
    return veh;
}
#if    defined    _ALS_AddStaticVehicle
    #undef    AddStaticVehicle
#else
    #define    _ALS_AddStaticVehicle
#endif
#define    AddStaticVehicle(%0)    custom_AddStaticVehicle(%0)

stock custom_DestroyVehicle(veh)
{
    if (IsValidVehicle(veh)) {
        this.SetVehicleCountOwner(veh, 0);
        this.SetVehicleIndexAI(veh, 0);
        this.SetVehicleKeyStatus(veh, false);
        this.SetOwnableCar(veh, INVALID_PLAYER_ID);
        this.SetNumberVehicleByVehId(veh, "", "");

        carFuel[veh] = 0.0;
        carMileage[veh] = 0.0;
    }
    return DestroyVehicle(veh);
}
#if    defined    _ALS_DestroyVehicle
    #undef    DestroyVehicle
#else
    #define    _ALS_DestroyVehicle
#endif
#define    DestroyVehicle(%0)    custom_DestroyVehicle(%0)

private Cars.SetDefaultSettings(veh) {
    if (IsValidVehicle(veh)) {
        this.SetVehicleCountOwner(veh, 0);
        this.SetVehicleIndexAI(veh, 0);
        this.SetVehicleKeyStatus(veh, true);
        this.SetOwnableCar(veh, INVALID_PLAYER_ID);
        this.SetNumberVehicleByVehId(veh, "", "");

        new bool:engine, bool:lights, bool:alarm, bool:doors, bool:bonnet, bool:boot, bool:objective;
	    GetVehicleParamsEx(veh, engine, lights, alarm, doors, bonnet, boot, objective);
        SetVehicleParamsEx(veh, false, false, false, false, false, false, false);

        carFuel[veh] = 200.0;
        carMileage[veh] = 0.0;

        this.rimsVehicle[veh] = 0;

        for (new i = 0; i < 11; i++)
            this.bodyKitVehicle[veh][i] = 0;
    }
}

stock Cars.SetBodyKitVehicle(vehicleId, index, value) {
    if (index < 0 || index > 11) {
        return;
    }

    if (!IsValidVehicle(vehicleId)) {
        return;
    }

    this.bodyKitVehicle[vehicleId][index] = value;
}

stock Cars.SetRimsVehicle(vehicleId, value) {
    if (!IsValidVehicle(vehicleId)) {
        return;
    }

    this.rimsVehicle[vehicleId] = value;
}

stock Cars.GetBodyKitVehicle(vehicleId, index) {
    if (index < 0 || index > 11) {
        return 0;
    }

    if (!IsValidVehicle(vehicleId)) {
        return 0;
    }

    if (this.bodyKitVehicle[vehicleId][index] < 0 || this.bodyKitVehicle[vehicleId][index] == 2658)  {
        return 0;
    }

    return this.bodyKitVehicle[vehicleId][index];
}

stock Cars.GetRimsVehicle(vehicleId) {
    if (!IsValidVehicle(vehicleId)) {
        return 0;
    }

    if (this.rimsVehicle[vehicleId] < 0)  {
        return 0;
    }

    return this.rimsVehicle[vehicleId];
}

stock Cars.GetToner(vehicleId, color)
{
    if (vehicleId == INVALID_VEHICLE_ID) {
        return 0;
    }
    return this.tonerVehicle[vehicleId][color]; 
}

stock Cars.SetToner(vehicleId, procent, color)
{
    if (vehicleId == INVALID_VEHICLE_ID) {
        return 0;
    }
    return this.tonerVehicle[vehicleId][color] = procent;
}

stock SendPacket_Plate(playerid, vehicleid, type, number[], region[])
{
    new
        BitStream:bsn = BS_New();

    BS_WriteValue(bsn, PR_UINT8, ID_CEF_PACKET);
    BS_WriteValue(bsn, PR_UINT16, _:s_CefSync);
	BS_WriteValue(bsn, PR_UINT32, vehicleid);

	BS_WriteValue(bsn, PR_UINT32, 0);
	BS_WriteValue(bsn, PR_UINT16, 256);

	BS_WriteValue(bsn, PR_UINT32, type);
	BS_WriteValue(
		bsn,
		PR_UINT32, strlen(number),
		PR_STRING, number, strlen(number)
	);
	BS_WriteValue(
		bsn,
		PR_UINT32, strlen(region),
		PR_STRING, region, strlen(region)
	);

    PR_SendPacket(bsn, playerid);
    BS_Delete(bsn);
}