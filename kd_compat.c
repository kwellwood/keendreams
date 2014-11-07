/* Keen Dreams v2.00 Source Code
 * Copyright (C) 2014 David Gow <david@ingeniumdigital.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include <unistd.h>
#include <stdint.h>
#include "kd_def.h"


extern statetype s_score;
extern statetype s_flowerpower1;
extern statetype s_flowerpower2;
extern statetype s_boobusbomb1;
extern statetype s_boobusbomb2;
extern statetype s_bombexplode;
extern statetype s_bombexplode2;
extern statetype s_bombexplode3;
extern statetype s_bombexplode4;
extern statetype s_bombexplode5;
extern statetype s_bombexplode6;
extern statetype s_powerblink1;
extern statetype s_powerblink2;
extern statetype s_worldkeen;
extern statetype s_worldkeenwave1;
extern statetype s_worldkeenwave2;
extern statetype s_worldkeenwave3;
extern statetype s_worldkeenwave4;
extern statetype s_worldkeenwave5;
extern statetype s_worldkeenwait;
extern statetype s_worldkeensleep1;
extern statetype s_worldkeensleep2;
extern statetype s_worldwalk;
extern statetype s_keenzee1;
extern statetype s_keenzee2;
extern statetype s_keenzee3;
extern statetype s_keenstand;
extern statetype s_keenpauselook;
extern statetype s_keenyawn1;
extern statetype s_keenyawn2;
extern statetype s_keenyawn3;
extern statetype s_keenyawn4;
extern statetype s_keenwait1;
extern statetype s_keenwait2;
extern statetype s_keenwait3;
extern statetype s_keenwait4;
extern statetype s_keenwait5;
extern statetype s_keenwait6;
extern statetype s_keengosleep1;
extern statetype s_keengosleep2;
extern statetype s_keensleep1;
extern statetype s_keensleep2;
extern statetype s_keengetup;
extern statetype s_keendie1;
extern statetype s_keendie2;
extern statetype s_keendie3;
extern statetype s_keenlookup;
extern statetype s_keenduck;
extern statetype s_keendrop;
extern statetype s_keenpole;
extern statetype s_keenclimb1;
extern statetype s_keenclimb2;
extern statetype s_keenclimb3;
extern statetype s_keenslide1;
extern statetype s_keenslide2;
extern statetype s_keenslide3;
extern statetype s_keenslide4;
extern statetype s_keenpolethrow1;
extern statetype s_keenpolethrow2;
extern statetype s_keenpolethrow3;
extern statetype s_keenpolethrowup1;
extern statetype s_keenpolethrowup2;
extern statetype s_keenpolethrowup3;
extern statetype s_keenpolethrowdown1;
extern statetype s_keenpolethrowdown2;
extern statetype s_keenpolethrowdown3;
extern statetype s_keenwalk1;
extern statetype s_keenwalk2;
extern statetype s_keenwalk3;
extern statetype s_keenwalk4;
extern statetype s_keenthrow1;
extern statetype s_keenthrow2;
extern statetype s_keenthrow3;
extern statetype s_keenthrow4;
extern statetype s_keenthrowup1;
extern statetype s_keenthrowup2;
extern statetype s_keenthrowup3;
extern statetype s_keenjumpup1;
extern statetype s_keenjumpup2;
extern statetype s_keenjumpup3;
extern statetype s_keenjump1;
extern statetype s_keenjump2;
extern statetype s_keenjump3;
extern statetype s_keenairthrow1;
extern statetype s_keenairthrow2;
extern statetype s_keenairthrow3;
extern statetype s_keenairthrowup1;
extern statetype s_keenairthrowup2;
extern statetype s_keenairthrowup3;
extern statetype s_keenairthrowdown1;
extern statetype s_keenairthrowdown2;
extern statetype s_keenairthrowdown3;
extern statetype s_door;
extern statetype s_doorraise;
extern statetype s_flower1;
extern statetype s_flower2;
extern statetype s_flower3;
extern statetype s_flower4;
extern statetype s_flower5;
extern statetype s_flower6;
extern statetype s_poofto1;
extern statetype s_poofto2;
extern statetype s_poofto3;
extern statetype s_poofto4;
extern statetype s_pooffrom1;
extern statetype s_pooffrom2;
extern statetype s_pooffrom3;
extern statetype s_pooffrom4;
extern statetype s_pooffrom5;
extern statetype s_pooffrom6;
extern statetype s_pooffrom7;
extern statetype s_bonus;
extern statetype s_bonusrise;
extern statetype s_broccowalk1;
extern statetype s_broccowalk2;
extern statetype s_broccowalk3;
extern statetype s_broccowalk4;
extern statetype s_broccosmash1;
extern statetype s_broccosmash2;
extern statetype s_broccosmash3;
extern statetype s_broccosmash4;
extern statetype s_broccosmash5;
extern statetype s_broccosmash6;
extern statetype s_broccosmash7;
extern statetype s_broccosmash8;
extern statetype s_broccosmash9;
extern statetype s_tomatbounce;
extern statetype s_tomatbounce2;
extern statetype s_carrotwalk1;
extern statetype s_carrotwalk2;
extern statetype s_carrotwalk3;
extern statetype s_carrotwalk4;
extern statetype s_carrotleap;
extern statetype s_asparwalk1;
extern statetype s_asparwalk2;
extern statetype s_asparwalk3;
extern statetype s_asparwalk4;
extern statetype s_grapewait;
extern statetype s_grapefall;
extern statetype s_grapesit;
extern statetype s_graperise;
extern statetype s_taterwalk1;
extern statetype s_taterwalk2;
extern statetype s_taterwalk3;
extern statetype s_taterwalk4;
extern statetype s_taterattack1;
extern statetype s_taterattack2;
extern statetype s_taterattack3;
extern statetype s_cartroll1;
extern statetype s_cartroll2;
extern statetype s_frenchywalk1;
extern statetype s_frenchywalk2;
extern statetype s_frenchywalk3;
extern statetype s_frenchywalk4;
extern statetype s_frenchyrun1;
extern statetype s_frenchyrun2;
extern statetype s_frenchyrun3;
extern statetype s_frenchyrun4;
extern statetype s_frenchythrow1;
extern statetype s_frenchythrow2;
extern statetype s_frenchythrow3;
extern statetype s_fry1;
extern statetype s_fry2;
extern statetype s_melonside;
extern statetype s_melonsidespit;
extern statetype s_melonsidespit2;
extern statetype s_melondown;
extern statetype s_melondownspit;
extern statetype s_melondownspit2;
extern statetype s_melonseed1;
extern statetype s_melonseed2;
extern statetype s_melonseedd1;
extern statetype s_melonseedd2;
extern statetype s_squasherwalk1;
extern statetype s_squasherwalk2;
extern statetype s_squasherjump1;
extern statetype s_squasherjump2;
extern statetype s_squasherwait;
extern statetype s_apelwalk1;
extern statetype s_apelwalk2;
extern statetype s_apelwalk3;
extern statetype s_apelclimb1;
extern statetype s_apelclimb2;
extern statetype s_apelslide1;
extern statetype s_apelslide2;
extern statetype s_apelslide3;
extern statetype s_apelslide4;
extern statetype s_apelfall;
extern statetype s_peabrainfly;
extern statetype s_peabrainwalk1;
extern statetype s_peabrainwalk2;
extern statetype s_peabrainwalk3;
extern statetype s_peabrainwalk4;
extern statetype s_peapodwalk1;
extern statetype s_peapodwalk2;
extern statetype s_peapodwalk3;
extern statetype s_peapodwalk4;
extern statetype s_peapodspit1;
extern statetype s_peapodspit2;
extern statetype s_boobuswalk1;
extern statetype s_boobuswalk2;
extern statetype s_boobuswalk3;
extern statetype s_boobuswalk4;
extern statetype s_boobusjump;
extern statetype s_boobusdie;
extern statetype s_boobusdie2;
extern statetype s_boobusdie3;
extern statetype s_deathboom1;
extern statetype s_deathboom2;
extern statetype s_deathboom3;
extern statetype s_deathboom4;
extern statetype s_deathboom5;
extern statetype s_deathboom6;


void WriteI16(int file, int16_t i)
{
	write(file, &i, 2);
}

void WriteU16(int file, uint16_t i)
{
	write(file, &i, 2);
}

void WriteI32(int file, int32_t i)
{
	write(file, &i, 4);
}

void WriteU32(int file, uint32_t i)
{
	write(file, &i, 4);
}

int16_t ReadI16(int file)
{
	int16_t i;
	read(file, &i, 2);
	return i;
}

uint16_t ReadU16(int file)
{
	uint16_t i;
	read(file, &i, 2);
	return i;
}

int32_t ReadI32(int file)
{
	int32_t i;
	read(file, &i, 4);
	return i;
}

uint32_t ReadU32(int file)
{
	uint32_t i;
	read(file, &i, 4);
	return i;
}

uint16_t ObjToInt(objtype *obj)
{
	if (!obj) return 0;
	return (uint16_t)(obj - player);
}

objtype *IntToObj(uint16_t obj)
{
	if (!obj) return 0;
	return player + obj;
}

uint16_t StateToInt(statetype *state)
{
	if (state == &s_score) return 0x0BB4;
	if (state == &s_flowerpower1) return 0x0BD2;
	if (state == &s_flowerpower2) return 0x0BF0;
	if (state == &s_boobusbomb1) return 0x0C0E;
	if (state == &s_boobusbomb2) return 0x0C2C;
	if (state == &s_bombexplode) return 0x0C4A;
	if (state == &s_bombexplode2) return 0x0C68;
	if (state == &s_bombexplode3) return 0x0C86;
	if (state == &s_bombexplode4) return 0x0CA4;
	if (state == &s_bombexplode5) return 0x0CC2;
	if (state == &s_bombexplode6) return 0x0CE0;
	if (state == &s_powerblink1) return 0x0CFE;
	if (state == &s_powerblink2) return 0x0D1C;
	if (state == &s_worldkeen) return 0x0D3A;
	if (state == &s_worldkeenwave1) return 0x0D58;
	if (state == &s_worldkeenwave2) return 0x0D76;
	if (state == &s_worldkeenwave3) return 0x0D94;
	if (state == &s_worldkeenwave4) return 0x0DB2;
	if (state == &s_worldkeenwave5) return 0x0DD0;
	if (state == &s_worldkeenwait) return 0x0DEE;
	if (state == &s_worldkeensleep1) return 0x0E0C;
	if (state == &s_worldkeensleep2) return 0x0E2A;
	if (state == &s_worldwalk) return 0x0E48;
	if (state == &s_keenzee1) return 0x0E7E;
	if (state == &s_keenzee2) return 0x0E9C;
	if (state == &s_keenzee3) return 0x0EBA;
	if (state == &s_keenstand) return 0x0ED8;
	if (state == &s_keenpauselook) return 0x0EF6;
	if (state == &s_keenyawn1) return 0x0F14;
	if (state == &s_keenyawn2) return 0x0F32;
	if (state == &s_keenyawn3) return 0x0F50;
	if (state == &s_keenyawn4) return 0x0F6E;
	if (state == &s_keenwait1) return 0x0F8C;
	if (state == &s_keenwait2) return 0x0FAA;
	if (state == &s_keenwait3) return 0x0FC8;
	if (state == &s_keenwait4) return 0x0FE6;
	if (state == &s_keenwait5) return 0x1004;
	if (state == &s_keenwait6) return 0x1022;
	if (state == &s_keengosleep1) return 0x1040;
	if (state == &s_keengosleep2) return 0x105E;
	if (state == &s_keensleep1) return 0x107C;
	if (state == &s_keensleep2) return 0x109A;
	if (state == &s_keengetup) return 0x10B8;
	if (state == &s_keendie1) return 0x10D6;
	if (state == &s_keendie2) return 0x10F4;
	if (state == &s_keendie3) return 0x1112;
	if (state == &s_keenlookup) return 0x1130;
	if (state == &s_keenduck) return 0x114E;
	if (state == &s_keendrop) return 0x116C;
	if (state == &s_keenpole) return 0x118A;
	if (state == &s_keenclimb1) return 0x11A8;
	if (state == &s_keenclimb2) return 0x11C6;
	if (state == &s_keenclimb3) return 0x11E4;
	if (state == &s_keenslide1) return 0x1202;
	if (state == &s_keenslide2) return 0x1220;
	if (state == &s_keenslide3) return 0x123E;
	if (state == &s_keenslide4) return 0x125C;
	if (state == &s_keenpolethrow1) return 0x127A;
	if (state == &s_keenpolethrow2) return 0x1298;
	if (state == &s_keenpolethrow3) return 0x12B6;
	if (state == &s_keenpolethrowup1) return 0x12D4;
	if (state == &s_keenpolethrowup2) return 0x12F2;
	if (state == &s_keenpolethrowup3) return 0x1310;
	if (state == &s_keenpolethrowdown1) return 0x132E;
	if (state == &s_keenpolethrowdown2) return 0x134C;
	if (state == &s_keenpolethrowdown3) return 0x136A;
	if (state == &s_keenwalk1) return 0x1388;
	if (state == &s_keenwalk2) return 0x13A6;
	if (state == &s_keenwalk3) return 0x13C4;
	if (state == &s_keenwalk4) return 0x13E2;
	if (state == &s_keenthrow1) return 0x1400;
	if (state == &s_keenthrow2) return 0x141E;
	if (state == &s_keenthrow3) return 0x143C;
	if (state == &s_keenthrow4) return 0x145A;
	if (state == &s_keenthrowup1) return 0x1478;
	if (state == &s_keenthrowup2) return 0x1496;
	if (state == &s_keenthrowup3) return 0x14B4;
	if (state == &s_keenjumpup1) return 0x14D2;
	if (state == &s_keenjumpup2) return 0x14F0;
	if (state == &s_keenjumpup3) return 0x150E;
	if (state == &s_keenjump1) return 0x152C;
	if (state == &s_keenjump2) return 0x154A;
	if (state == &s_keenjump3) return 0x1568;
	if (state == &s_keenairthrow1) return 0x1586;
	if (state == &s_keenairthrow2) return 0x15A4;
	if (state == &s_keenairthrow3) return 0x15C2;
	if (state == &s_keenairthrowup1) return 0x15E0;
	if (state == &s_keenairthrowup2) return 0x15FE;
	if (state == &s_keenairthrowup3) return 0x161C;
	if (state == &s_keenairthrowdown1) return 0x163A;
	if (state == &s_keenairthrowdown2) return 0x1658;
	if (state == &s_keenairthrowdown3) return 0x1676;
	if (state == &s_door) return 0x1732;
	if (state == &s_doorraise) return 0x1750;
	if (state == &s_flower1) return 0x176E;
	if (state == &s_flower2) return 0x178C;
	if (state == &s_flower3) return 0x17AA;
	if (state == &s_flower4) return 0x17C8;
	if (state == &s_flower5) return 0x17E6;
	if (state == &s_flower6) return 0x1804;
	if (state == &s_poofto1) return 0x1822;
	if (state == &s_poofto2) return 0x1840;
	if (state == &s_poofto3) return 0x185E;
	if (state == &s_poofto4) return 0x187C;
	if (state == &s_pooffrom1) return 0x189A;
	if (state == &s_pooffrom2) return 0x18B8;
	if (state == &s_pooffrom3) return 0x18D6;
	if (state == &s_pooffrom4) return 0x18F4;
	if (state == &s_pooffrom5) return 0x1912;
	if (state == &s_pooffrom6) return 0x1930;
	if (state == &s_pooffrom7) return 0x194E;
	if (state == &s_bonus) return 0x196C;
	if (state == &s_bonusrise) return 0x198A;
	if (state == &s_broccowalk1) return 0x19C0;
	if (state == &s_broccowalk2) return 0x19DE;
	if (state == &s_broccowalk3) return 0x19FC;
	if (state == &s_broccowalk4) return 0x1A1A;
	if (state == &s_broccosmash1) return 0x1A38;
	if (state == &s_broccosmash2) return 0x1A56;
	if (state == &s_broccosmash3) return 0x1A74;
	if (state == &s_broccosmash4) return 0x1A92;
	if (state == &s_broccosmash5) return 0x1AB0;
	if (state == &s_broccosmash6) return 0x1ACE;
	if (state == &s_broccosmash7) return 0x1AEC;
	if (state == &s_broccosmash8) return 0x1B0A;
	if (state == &s_broccosmash9) return 0x1B28;
	if (state == &s_tomatbounce) return 0x1B46;
	if (state == &s_tomatbounce2) return 0x1B64;
	if (state == &s_carrotwalk1) return 0x1B82;
	if (state == &s_carrotwalk2) return 0x1BA0;
	if (state == &s_carrotwalk3) return 0x1BBE;
	if (state == &s_carrotwalk4) return 0x1BDC;
	if (state == &s_carrotleap) return 0x1BFA;
	if (state == &s_asparwalk1) return 0x1C18;
	if (state == &s_asparwalk2) return 0x1C36;
	if (state == &s_asparwalk3) return 0x1C54;
	if (state == &s_asparwalk4) return 0x1C72;
	if (state == &s_grapewait) return 0x1C90;
	if (state == &s_grapefall) return 0x1CAE;
	if (state == &s_grapesit) return 0x1CCC;
	if (state == &s_graperise) return 0x1CEA;
	if (state == &s_taterwalk1) return 0x1D08;
	if (state == &s_taterwalk2) return 0x1D26;
	if (state == &s_taterwalk3) return 0x1D44;
	if (state == &s_taterwalk4) return 0x1D62;
	if (state == &s_taterattack1) return 0x1D80;
	if (state == &s_taterattack2) return 0x1D9E;
	if (state == &s_taterattack3) return 0x1DBC;
	if (state == &s_cartroll1) return 0x1DDA;
	if (state == &s_cartroll2) return 0x1DF8;
	if (state == &s_frenchywalk1) return 0x1E16;
	if (state == &s_frenchywalk2) return 0x1E34;
	if (state == &s_frenchywalk3) return 0x1E52;
	if (state == &s_frenchywalk4) return 0x1E70;
	if (state == &s_frenchyrun1) return 0x1E8E;
	if (state == &s_frenchyrun2) return 0x1EAC;
	if (state == &s_frenchyrun3) return 0x1ECA;
	if (state == &s_frenchyrun4) return 0x1EE8;
	if (state == &s_frenchythrow1) return 0x1F06;
	if (state == &s_frenchythrow2) return 0x1F24;
	if (state == &s_frenchythrow3) return 0x1F42;
	if (state == &s_fry1) return 0x1F60;
	if (state == &s_fry2) return 0x1F7E;
	if (state == &s_melonside) return 0x1F9C;
	if (state == &s_melonsidespit) return 0x1FBA;
	if (state == &s_melonsidespit2) return 0x1FD8;
	if (state == &s_melondown) return 0x1FF6;
	if (state == &s_melondownspit) return 0x2014;
	if (state == &s_melondownspit2) return 0x2032;
	if (state == &s_melonseed1) return 0x2050;
	if (state == &s_melonseed2) return 0x206E;
	if (state == &s_melonseedd1) return 0x208C;
	if (state == &s_melonseedd2) return 0x20AA;
	if (state == &s_squasherwalk1) return 0x20C8;
	if (state == &s_squasherwalk2) return 0x20E6;
	if (state == &s_squasherjump1) return 0x2104;
	if (state == &s_squasherjump2) return 0x2122;
	if (state == &s_squasherwait) return 0x2140;
	if (state == &s_apelwalk1) return 0x215E;
	if (state == &s_apelwalk2) return 0x217C;
	if (state == &s_apelwalk3) return 0x219A;
	if (state == &s_apelclimb1) return 0x21B8;
	if (state == &s_apelclimb2) return 0x21D6;
	if (state == &s_apelslide1) return 0x21F4;
	if (state == &s_apelslide2) return 0x2212;
	if (state == &s_apelslide3) return 0x2230;
	if (state == &s_apelslide4) return 0x224E;
	if (state == &s_apelfall) return 0x226C;
	if (state == &s_peabrainfly) return 0x228A;
	if (state == &s_peabrainwalk1) return 0x22A8;
	if (state == &s_peabrainwalk2) return 0x22C6;
	if (state == &s_peabrainwalk3) return 0x22E4;
	if (state == &s_peabrainwalk4) return 0x2302;
	if (state == &s_peapodwalk1) return 0x2320;
	if (state == &s_peapodwalk2) return 0x233E;
	if (state == &s_peapodwalk3) return 0x235C;
	if (state == &s_peapodwalk4) return 0x237A;
	if (state == &s_peapodspit1) return 0x2398;
	if (state == &s_peapodspit2) return 0x23B6;
	if (state == &s_boobuswalk1) return 0x23D4;
	if (state == &s_boobuswalk2) return 0x23F2;
	if (state == &s_boobuswalk3) return 0x2410;
	if (state == &s_boobuswalk4) return 0x242E;
	if (state == &s_boobusjump) return 0x244C;
	if (state == &s_boobusdie) return 0x246A;
	if (state == &s_boobusdie2) return 0x2488;
	if (state == &s_boobusdie3) return 0x24A6;
	if (state == &s_deathboom1) return 0x24C4;
	if (state == &s_deathboom2) return 0x24E2;
	if (state == &s_deathboom3) return 0x2500;
	if (state == &s_deathboom4) return 0x251E;
	if (state == &s_deathboom5) return 0x253C;
	if (state == &s_deathboom6) return 0x255A;
	return 0;
}

statetype *IntToState(uint16_t state)
{
	switch(state)
	{
	case 0x0BB4: return &s_score;
	case 0x0BD2: return &s_flowerpower1;
	case 0x0BF0: return &s_flowerpower2;
	case 0x0C0E: return &s_boobusbomb1;
	case 0x0C2C: return &s_boobusbomb2;
	case 0x0C4A: return &s_bombexplode;
	case 0x0C68: return &s_bombexplode2;
	case 0x0C86: return &s_bombexplode3;
	case 0x0CA4: return &s_bombexplode4;
	case 0x0CC2: return &s_bombexplode5;
	case 0x0CE0: return &s_bombexplode6;
	case 0x0CFE: return &s_powerblink1;
	case 0x0D1C: return &s_powerblink2;
	case 0x0D3A: return &s_worldkeen;
	case 0x0D58: return &s_worldkeenwave1;
	case 0x0D76: return &s_worldkeenwave2;
	case 0x0D94: return &s_worldkeenwave3;
	case 0x0DB2: return &s_worldkeenwave4;
	case 0x0DD0: return &s_worldkeenwave5;
	case 0x0DEE: return &s_worldkeenwait;
	case 0x0E0C: return &s_worldkeensleep1;
	case 0x0E2A: return &s_worldkeensleep2;
	case 0x0E48: return &s_worldwalk;
	case 0x0E7E: return &s_keenzee1;
	case 0x0E9C: return &s_keenzee2;
	case 0x0EBA: return &s_keenzee3;
	case 0x0ED8: return &s_keenstand;
	case 0x0EF6: return &s_keenpauselook;
	case 0x0F14: return &s_keenyawn1;
	case 0x0F32: return &s_keenyawn2;
	case 0x0F50: return &s_keenyawn3;
	case 0x0F6E: return &s_keenyawn4;
	case 0x0F8C: return &s_keenwait1;
	case 0x0FAA: return &s_keenwait2;
	case 0x0FC8: return &s_keenwait3;
	case 0x0FE6: return &s_keenwait4;
	case 0x1004: return &s_keenwait5;
	case 0x1022: return &s_keenwait6;
	case 0x1040: return &s_keengosleep1;
	case 0x105E: return &s_keengosleep2;
	case 0x107C: return &s_keensleep1;
	case 0x109A: return &s_keensleep2;
	case 0x10B8: return &s_keengetup;
	case 0x10D6: return &s_keendie1;
	case 0x10F4: return &s_keendie2;
	case 0x1112: return &s_keendie3;
	case 0x1130: return &s_keenlookup;
	case 0x114E: return &s_keenduck;
	case 0x116C: return &s_keendrop;
	case 0x118A: return &s_keenpole;
	case 0x11A8: return &s_keenclimb1;
	case 0x11C6: return &s_keenclimb2;
	case 0x11E4: return &s_keenclimb3;
	case 0x1202: return &s_keenslide1;
	case 0x1220: return &s_keenslide2;
	case 0x123E: return &s_keenslide3;
	case 0x125C: return &s_keenslide4;
	case 0x127A: return &s_keenpolethrow1;
	case 0x1298: return &s_keenpolethrow2;
	case 0x12B6: return &s_keenpolethrow3;
	case 0x12D4: return &s_keenpolethrowup1;
	case 0x12F2: return &s_keenpolethrowup2;
	case 0x1310: return &s_keenpolethrowup3;
	case 0x132E: return &s_keenpolethrowdown1;
	case 0x134C: return &s_keenpolethrowdown2;
	case 0x136A: return &s_keenpolethrowdown3;
	case 0x1388: return &s_keenwalk1;
	case 0x13A6: return &s_keenwalk2;
	case 0x13C4: return &s_keenwalk3;
	case 0x13E2: return &s_keenwalk4;
	case 0x1400: return &s_keenthrow1;
	case 0x141E: return &s_keenthrow2;
	case 0x143C: return &s_keenthrow3;
	case 0x145A: return &s_keenthrow4;
	case 0x1478: return &s_keenthrowup1;
	case 0x1496: return &s_keenthrowup2;
	case 0x14B4: return &s_keenthrowup3;
	case 0x14D2: return &s_keenjumpup1;
	case 0x14F0: return &s_keenjumpup2;
	case 0x150E: return &s_keenjumpup3;
	case 0x152C: return &s_keenjump1;
	case 0x154A: return &s_keenjump2;
	case 0x1568: return &s_keenjump3;
	case 0x1586: return &s_keenairthrow1;
	case 0x15A4: return &s_keenairthrow2;
	case 0x15C2: return &s_keenairthrow3;
	case 0x15E0: return &s_keenairthrowup1;
	case 0x15FE: return &s_keenairthrowup2;
	case 0x161C: return &s_keenairthrowup3;
	case 0x163A: return &s_keenairthrowdown1;
	case 0x1658: return &s_keenairthrowdown2;
	case 0x1676: return &s_keenairthrowdown3;
	case 0x1732: return &s_door;
	case 0x1750: return &s_doorraise;
	case 0x176E: return &s_flower1;
	case 0x178C: return &s_flower2;
	case 0x17AA: return &s_flower3;
	case 0x17C8: return &s_flower4;
	case 0x17E6: return &s_flower5;
	case 0x1804: return &s_flower6;
	case 0x1822: return &s_poofto1;
	case 0x1840: return &s_poofto2;
	case 0x185E: return &s_poofto3;
	case 0x187C: return &s_poofto4;
	case 0x189A: return &s_pooffrom1;
	case 0x18B8: return &s_pooffrom2;
	case 0x18D6: return &s_pooffrom3;
	case 0x18F4: return &s_pooffrom4;
	case 0x1912: return &s_pooffrom5;
	case 0x1930: return &s_pooffrom6;
	case 0x194E: return &s_pooffrom7;
	case 0x196C: return &s_bonus;
	case 0x198A: return &s_bonusrise;
	case 0x19C0: return &s_broccowalk1;
	case 0x19DE: return &s_broccowalk2;
	case 0x19FC: return &s_broccowalk3;
	case 0x1A1A: return &s_broccowalk4;
	case 0x1A38: return &s_broccosmash1;
	case 0x1A56: return &s_broccosmash2;
	case 0x1A74: return &s_broccosmash3;
	case 0x1A92: return &s_broccosmash4;
	case 0x1AB0: return &s_broccosmash5;
	case 0x1ACE: return &s_broccosmash6;
	case 0x1AEC: return &s_broccosmash7;
	case 0x1B0A: return &s_broccosmash8;
	case 0x1B28: return &s_broccosmash9;
	case 0x1B46: return &s_tomatbounce;
	case 0x1B64: return &s_tomatbounce2;
	case 0x1B82: return &s_carrotwalk1;
	case 0x1BA0: return &s_carrotwalk2;
	case 0x1BBE: return &s_carrotwalk3;
	case 0x1BDC: return &s_carrotwalk4;
	case 0x1BFA: return &s_carrotleap;
	case 0x1C18: return &s_asparwalk1;
	case 0x1C36: return &s_asparwalk2;
	case 0x1C54: return &s_asparwalk3;
	case 0x1C72: return &s_asparwalk4;
	case 0x1C90: return &s_grapewait;
	case 0x1CAE: return &s_grapefall;
	case 0x1CCC: return &s_grapesit;
	case 0x1CEA: return &s_graperise;
	case 0x1D08: return &s_taterwalk1;
	case 0x1D26: return &s_taterwalk2;
	case 0x1D44: return &s_taterwalk3;
	case 0x1D62: return &s_taterwalk4;
	case 0x1D80: return &s_taterattack1;
	case 0x1D9E: return &s_taterattack2;
	case 0x1DBC: return &s_taterattack3;
	case 0x1DDA: return &s_cartroll1;
	case 0x1DF8: return &s_cartroll2;
	case 0x1E16: return &s_frenchywalk1;
	case 0x1E34: return &s_frenchywalk2;
	case 0x1E52: return &s_frenchywalk3;
	case 0x1E70: return &s_frenchywalk4;
	case 0x1E8E: return &s_frenchyrun1;
	case 0x1EAC: return &s_frenchyrun2;
	case 0x1ECA: return &s_frenchyrun3;
	case 0x1EE8: return &s_frenchyrun4;
	case 0x1F06: return &s_frenchythrow1;
	case 0x1F24: return &s_frenchythrow2;
	case 0x1F42: return &s_frenchythrow3;
	case 0x1F60: return &s_fry1;
	case 0x1F7E: return &s_fry2;
	case 0x1F9C: return &s_melonside;
	case 0x1FBA: return &s_melonsidespit;
	case 0x1FD8: return &s_melonsidespit2;
	case 0x1FF6: return &s_melondown;
	case 0x2014: return &s_melondownspit;
	case 0x2032: return &s_melondownspit2;
	case 0x2050: return &s_melonseed1;
	case 0x206E: return &s_melonseed2;
	case 0x208C: return &s_melonseedd1;
	case 0x20AA: return &s_melonseedd2;
	case 0x20C8: return &s_squasherwalk1;
	case 0x20E6: return &s_squasherwalk2;
	case 0x2104: return &s_squasherjump1;
	case 0x2122: return &s_squasherjump2;
	case 0x2140: return &s_squasherwait;
	case 0x215E: return &s_apelwalk1;
	case 0x217C: return &s_apelwalk2;
	case 0x219A: return &s_apelwalk3;
	case 0x21B8: return &s_apelclimb1;
	case 0x21D6: return &s_apelclimb2;
	case 0x21F4: return &s_apelslide1;
	case 0x2212: return &s_apelslide2;
	case 0x2230: return &s_apelslide3;
	case 0x224E: return &s_apelslide4;
	case 0x226C: return &s_apelfall;
	case 0x228A: return &s_peabrainfly;
	case 0x22A8: return &s_peabrainwalk1;
	case 0x22C6: return &s_peabrainwalk2;
	case 0x22E4: return &s_peabrainwalk3;
	case 0x2302: return &s_peabrainwalk4;
	case 0x2320: return &s_peapodwalk1;
	case 0x233E: return &s_peapodwalk2;
	case 0x235C: return &s_peapodwalk3;
	case 0x237A: return &s_peapodwalk4;
	case 0x2398: return &s_peapodspit1;
	case 0x23B6: return &s_peapodspit2;
	case 0x23D4: return &s_boobuswalk1;
	case 0x23F2: return &s_boobuswalk2;
	case 0x2410: return &s_boobuswalk3;
	case 0x242E: return &s_boobuswalk4;
	case 0x244C: return &s_boobusjump;
	case 0x246A: return &s_boobusdie;
	case 0x2488: return &s_boobusdie2;
	case 0x24A6: return &s_boobusdie3;
	case 0x24C4: return &s_deathboom1;
	case 0x24E2: return &s_deathboom2;
	case 0x2500: return &s_deathboom3;
	case 0x251E: return &s_deathboom4;
	case 0x253C: return &s_deathboom5;
	case 0x255A: return &s_deathboom6;
	}
	return 0;
}


void WriteObjStruct(int file, objtype *obj)
{
	WriteU16(file, obj->obclass);
	WriteU16(file, obj->active);
	WriteU16(file, obj->needtoreact);
	WriteU16(file, obj->needtoclip);
	WriteU16(file, obj->nothink);
	WriteU16(file, obj->x);
	WriteU16(file, obj->y);

	WriteI16(file, obj->xdir);
	WriteI16(file, obj->ydir);
	WriteI16(file, obj->xmove);
	WriteI16(file, obj->ymove);
	WriteI16(file, obj->xspeed);
	WriteI16(file, obj->yspeed);

	WriteI16(file, obj->ticcount);
	WriteI16(file, obj->ticadjust);
	WriteU16(file, StateToInt(obj->state));

	WriteI16(file, obj->shapenum);

	WriteU16(file, obj->left);
	WriteU16(file, obj->top);
	WriteU16(file, obj->right);
	WriteU16(file, obj->bottom);	// hit rectangle
	WriteU16(file, obj->midx);
	WriteU16(file, obj->tileleft);
	WriteU16(file, obj->tiletop);
	WriteU16(file, obj->tileright);
	WriteU16(file, obj->tilebottom);	// hit rect in tiles
	WriteU16(file, obj->tilemidx);

	WriteI16(file, obj->hitnorth);
	WriteI16(file, obj->hiteast);
	WriteI16(file, obj->hitsouth);
	WriteI16(file, obj->hitwest);	// wall numbers contacted

	WriteI16(file, obj->temp1);
	WriteI16(file, obj->temp2);
	WriteI16(file, obj->temp3);
	WriteI16(file, obj->temp4);

	WriteU16(file, 0);
	
	WriteU16(file, ObjToInt(obj->next));
	WriteU16(file, ObjToInt(obj->prev));

}

void ReadObjStruct(int file, objtype *obj)
{
	obj->obclass = ReadU16(file);
	obj->active = ReadU16(file);
	obj->needtoreact = ReadU16(file);
	obj->needtoclip = ReadU16(file);
	obj->nothink = ReadU16(file);
	obj->x = ReadU16(file);
	obj->y = ReadU16(file);

	obj->xdir = ReadI16(file);
	obj->ydir = ReadI16(file);
	obj->xmove = ReadI16(file);
	obj->ymove = ReadI16(file);
	obj->xspeed = ReadI16(file);
	obj->yspeed = ReadI16(file);

	obj->ticcount = ReadI16(file);
	obj->ticadjust = ReadI16(file);
	obj->state = IntToState(ReadU16(file));

	obj->shapenum = ReadI16(file);

	obj->left = ReadU16(file);
	obj->top = ReadU16(file);
	obj->right = ReadU16(file);
	obj->bottom = ReadU16(file);	// hit rectangle
	obj->midx = ReadU16(file);
	obj->tileleft = ReadU16(file);
	obj->tiletop = ReadU16(file);
	obj->tileright = ReadU16(file);
	obj->tilebottom = ReadU16(file);	// hit rect in tiles
	obj->tilemidx = ReadU16(file);

	obj->hitnorth = ReadI16(file);
	obj->hiteast = ReadI16(file);
	obj->hitsouth = ReadI16(file);
	obj->hitwest = ReadI16(file);	// wall numbers contacted

	obj->temp1 = ReadI16(file);
	obj->temp2 = ReadI16(file);
	obj->temp3 = ReadI16(file);
	obj->temp4 = ReadI16(file);

	ReadU16(file);
	
	obj->next = IntToObj(ReadU16(file));
	obj->prev = IntToObj(ReadU16(file));

}