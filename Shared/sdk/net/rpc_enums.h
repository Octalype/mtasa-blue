/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        Shared/sdk/net/rpc_enums.h
 *  PURPOSE:     Lua RPC enums
 *
 *  Multi Theft Auto is available from http://www.multitheftauto.com/
 *
 *****************************************************************************/

#pragma once

/*
    HEY, YOU!

    ONLY ADD ENUMS TO THE END OF THIS FILE,
    ABOVE `NUM_RPC_FUNCS`.

    THANKS.
*/
enum eElementRPCFunctions
{
    DONT_USE_0 = 0,
    SET_TIME,
    SET_WEATHER,
    SET_WEATHER_BLENDED,
    SET_MINUTE_DURATION,

    SET_ELEMENT_PARENT,
    SET_ELEMENT_DATA,
    SET_ELEMENT_POSITION,
    SET_ELEMENT_VELOCITY,
    SET_ELEMENT_INTERIOR,
    SET_ELEMENT_DIMENSION,
    ATTACH_ELEMENTS,
    DETACH_ELEMENTS,
    SET_ELEMENT_ALPHA,
    SET_ELEMENT_NAME,
    SET_ELEMENT_HEALTH,
    SET_ELEMENT_MODEL,
    SET_ELEMENT_ATTACHED_OFFSETS,

    SET_PLAYER_MONEY,
    SHOW_PLAYER_HUD_COMPONENT,
    FORCE_PLAYER_MAP,
    SET_PLAYER_NAMETAG_TEXT,
    REMOVE_PLAYER_NAMETAG_COLOR,
    SET_PLAYER_NAMETAG_COLOR,
    SET_PLAYER_NAMETAG_SHOWING,

    SET_PED_ARMOR,
    SET_PED_ROTATION,
    GIVE_PED_JETPACK,
    REMOVE_PED_JETPACK,
    REMOVE_PED_CLOTHES,
    SET_PED_GRAVITY,
    SET_PED_CHOKING,
    SET_PED_FIGHTING_STYLE,
    SET_PED_MOVE_ANIM,
    WARP_PED_INTO_VEHICLE,
    REMOVE_PED_FROM_VEHICLE,
    SET_PED_DOING_GANG_DRIVEBY,
    SET_PED_ANIMATION,
    SET_PED_ANIMATION_PROGRESS,
    SET_PED_ON_FIRE,
    SET_PED_HEADLESS,
    SET_PED_FROZEN,
    RELOAD_PED_WEAPON,

    DESTROY_ALL_VEHICLES,
    FIX_VEHICLE,
    BLOW_VEHICLE,
    SET_VEHICLE_ROTATION,
    SET_VEHICLE_TURNSPEED,
    SET_VEHICLE_COLOR,
    SET_VEHICLE_LOCKED,
    SET_VEHICLE_DOORS_UNDAMAGEABLE,
    SET_VEHICLE_SIRENE_ON,
    SET_VEHICLE_LANDING_GEAR_DOWN,
    SET_HELICOPTER_ROTOR_SPEED,
    ADD_VEHICLE_UPGRADE,
    ADD_ALL_VEHICLE_UPGRADES,
    REMOVE_VEHICLE_UPGRADE,
    SET_VEHICLE_DAMAGE_STATE,
    SET_VEHICLE_OVERRIDE_LIGHTS,
    SET_VEHICLE_ENGINE_STATE,
    SET_VEHICLE_DIRT_LEVEL,
    SET_VEHICLE_DAMAGE_PROOF,
    SET_VEHICLE_PAINTJOB,
    SET_VEHICLE_FUEL_TANK_EXPLODABLE,
    SET_VEHICLE_WHEEL_STATES,
    SET_VEHICLE_FROZEN,
    SET_TRAIN_DERAILED,
    SET_TRAIN_DERAILABLE,
    SET_TRAIN_DIRECTION,
    SET_TRAIN_SPEED,
    SET_TAXI_LIGHT_ON,
    SET_VEHICLE_HEADLIGHT_COLOR,
    SET_VEHICLE_DOOR_OPEN_RATIO,

    GIVE_WEAPON,
    TAKE_WEAPON,
    TAKE_ALL_WEAPONS,
    SET_WEAPON_AMMO,
    SET_WEAPON_SLOT,

    DESTROY_ALL_BLIPS,
    SET_BLIP_ICON,
    SET_BLIP_SIZE,
    SET_BLIP_COLOR,
    SET_BLIP_ORDERING,

    DESTROY_ALL_OBJECTS,
    SET_OBJECT_ROTATION,
    MOVE_OBJECT,
    STOP_OBJECT,

    DESTROY_ALL_RADAR_AREAS,
    SET_RADAR_AREA_SIZE,
    SET_RADAR_AREA_COLOR,
    SET_RADAR_AREA_FLASHING,

    DESTROY_ALL_MARKERS,
    SET_MARKER_TYPE,
    SET_MARKER_COLOR,
    SET_MARKER_SIZE,
    SET_MARKER_TARGET,
    SET_MARKER_ICON,

    DESTROY_ALL_PICKUPS,
    SET_PICKUP_TYPE,
    SET_PICKUP_VISIBLE,

    PLAY_SOUND,

    BIND_KEY,
    UNBIND_KEY,
    BIND_COMMAND,
    UNBIND_COMMAND,
    TOGGLE_CONTROL_ABILITY,
    TOGGLE_ALL_CONTROL_ABILITY,
    SET_CONTROL_STATE,
    FORCE_RECONNECT,

    SET_TEAM_NAME,
    SET_TEAM_COLOR,
    SET_PLAYER_TEAM,
    SET_TEAM_FRIENDLY_FIRE,

    SET_WANTED_LEVEL,

    SET_CAMERA_MATRIX,
    SET_CAMERA_TARGET,
    SET_CAMERA_INTERIOR,
    FADE_CAMERA,

    SHOW_CURSOR,
    SHOW_CHAT,

    SET_GRAVITY,
    SET_GAME_SPEED,
    SET_WAVE_HEIGHT,
    SET_SKY_GRADIENT,
    RESET_SKY_GRADIENT,
    SET_HEAT_HAZE,
    RESET_HEAT_HAZE,
    SET_BLUR_LEVEL,
    SET_FPS_LIMIT,
    SET_GARAGE_OPEN,
    RESET_MAP_INFO,
    SET_GLITCH_ENABLED,
    SET_CLOUDS_ENABLED,

    REMOVE_ELEMENT_DATA,

    SET_VEHICLE_HANDLING,
    SET_VEHICLE_HANDLING_PROPERTY,
    RESET_VEHICLE_HANDLING_PROPERTY,
    RESET_VEHICLE_HANDLING,

    TOGGLE_DEBUGGER,

    SET_ELEMENT_WATER_LEVEL,
    SET_ALL_ELEMENT_WATER_LEVEL,
    SET_WORLD_WATER_LEVEL,
    RESET_WORLD_WATER_LEVEL,
    SET_WATER_VERTEX_POSITION,

    SET_ELEMENT_DOUBLESIDED,
    SET_TRAFFIC_LIGHT_STATE,
    SET_VEHICLE_TURRET_POSITION,
    SET_OBJECT_SCALE,
    SET_ELEMENT_COLLISIONS_ENABLED,
    SET_JETPACK_MAXHEIGHT,

    SET_WATER_COLOR,
    RESET_WATER_COLOR,
    SET_ELEMENT_FROZEN,
    SET_LOW_LOD_ELEMENT,
    SET_BLIP_VISIBLE_DISTANCE,

    SET_INTERIOR_SOUNDS_ENABLED,
    SET_RAIN_LEVEL,
    SET_SUN_SIZE,
    SET_SUN_COLOR,
    SET_WIND_VELOCITY,
    SET_FAR_CLIP_DISTANCE,
    SET_FOG_DISTANCE,
    RESET_RAIN_LEVEL,
    RESET_SUN_SIZE,
    RESET_SUN_COLOR,
    RESET_WIND_VELOCITY,
    RESET_FAR_CLIP_DISTANCE,
    RESET_FOG_DISTANCE,

    SET_AIRCRAFT_MAXHEIGHT,

    SET_WEAPON_PROPERTY,

    SET_VEHICLE_VARIANT,

    REMOVE_WORLD_MODEL,
    RESTORE_WORLD_MODEL,
    RESTORE_ALL_WORLD_MODELS,

    TAKE_PLAYER_SCREEN_SHOT,
    SET_OCCLUSIONS_ENABLED,

    GIVE_VEHICLE_SIRENS,
    REMOVE_VEHICLE_SIRENS,
    SET_VEHICLE_SIRENS,

    SET_SYNC_INTERVALS,

    SET_JETPACK_WEAPON_ENABLED,

    FIRE_CUSTOM_WEAPON,
    SET_CUSTOM_WEAPON_STATE,
    SET_CUSTOM_WEAPON_AMMO,
    SET_CUSTOM_WEAPON_CLIP_AMMO,
    SET_CUSTOM_WEAPON_TARGET,
    RESET_CUSTOM_WEAPON_TARGET,
    SET_CUSTOM_WEAPON_FLAGS,
    SET_CUSTOM_WEAPON_FIRING_RATE,
    RESET_CUSTOM_WEAPON_FIRING_RATE,
    SET_WEAPON_OWNER,

    SET_AIRCRAFT_MAXVELOCITY,

    RESET_MOON_SIZE,
    SET_MOON_SIZE,

    SET_VEHICLE_PLATE_TEXT,
    SET_PROPAGATE_CALLS_ENABLED,

    SET_TRAIN_TRACK,
    SET_TRAIN_POSITION,

    SET_OBJECT_VISIBLE_IN_ALL_DIMENSIONS,

    SET_ELEMENT_ANGULAR_VELOCITY,

    SET_PED_ANIMATION_SPEED,
    SET_ELEMENT_ACCESS_LEVEL,

    NUM_RPC_FUNCS // Add above this line
};
