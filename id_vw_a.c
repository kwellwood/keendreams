// Stubs for id_vw_a.asm

#include "id_heads.h"


void VW_WaitVBL(int vblanks)
{
	while (vblanks--)
		VW_GL_Present();
}

cardtype VW_VideoID()
{
	return VGAcard;
}

boolean FindPS2()
{
	return false;
}

boolean FindEGA()
{
	return true;
}

boolean FindCGA()
{
	return true;
}

boolean FindMono()
{
	return true;
}

boolean Find6845()
{
	return true;
}

void FindActive()
{
}

void FoundDevice()
{
}
