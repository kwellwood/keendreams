/*
Omnispeak: A Commander Keen Reimplementation
Copyright (C) 2012 David Gow <david@ingeniumdigital.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "id_heads.h"
#include <SDL2/SDL.h>
#include <GL/glew.h>

GLuint screentexture;
GLuint palettetexture;
GLuint vw_shaderprogram;
unsigned lastdrawnbuffer;
unsigned lastdrawnpan;
int vw_viewportx, vw_viewporty, vw_viewportw, vw_viewporth;
int vw_integerscalex, vw_integerscaley;
GLuint fbotexture;
GLuint framebufferobj;
extern SDL_Window *window;
extern const uint8_t VW_EGAPalette[16][3];
const char *pxprog = 	"#version 110\n"\
			"\n"\
			"uniform sampler2D screenBuf;\n"\
			"uniform sampler1D palette;\n"\
			"\n"\
			"void main() {\n"\
			"\tgl_FragColor = texture1D(palette,texture2D(screenBuf, gl_TexCoord[0].xy).r);\n"\
			"}\n";

void VW_GL_SetVGAPalette(const uint8_t vgapal[16][3])
{
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, palettetexture);
	glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 16, GL_RGB, GL_UNSIGNED_BYTE, &vgapal[0][0]);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glActiveTexture(GL_TEXTURE0);
}

void VW_GL_SetEGAPalette(char egapal[17])
{
	uint8_t vgapal[16][3];
	for(int i = 0; i < 16; ++i)
	{
		int index = ((egapal[i] >> 1) & 8) | (egapal[i] & 7);
		vgapal[i][0] = VW_EGAPalette[index][0];
		vgapal[i][1] = VW_EGAPalette[index][1];
		vgapal[i][2] = VW_EGAPalette[index][2];
	}
	VW_GL_SetVGAPalette(vgapal);
}

void VW_GL_Init()
{
	glGenTextures(1, &screentexture);
	glGenTextures(1, &palettetexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, palettetexture);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB8, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glActiveTexture(GL_TEXTURE0);
	VW_GL_SetVGAPalette(&VW_EGAPalette[0][0]);
	
	int compileStatus = 0;
	GLuint ps = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(ps, 1, &pxprog, 0);
	glCompileShader(ps);
	glGetShaderiv(ps, GL_COMPILE_STATUS, &compileStatus);
	if (!compileStatus)
	{
		glDeleteShader(ps);
		Quit("Could not compile palette conversion fragment shader!");
	}

	vw_shaderprogram = glCreateProgram();
	glAttachShader(vw_shaderprogram, ps);
	glLinkProgram(vw_shaderprogram);
	glDeleteShader(ps);
	compileStatus = 0;
	glGetProgramiv(vw_shaderprogram, GL_LINK_STATUS, &compileStatus);
	if (!compileStatus)
	{
		glDeleteProgram(vw_shaderprogram);
		Quit("Could not link palette conversion program!");
	}
	glUseProgram(vw_shaderprogram);
	glUniform1i(glGetUniformLocation(vw_shaderprogram, "screenBuf"),0);
	glUniform1i(glGetUniformLocation(vw_shaderprogram, "palette"),1);
}

void VW_GL_SetupFramebufferObject()
{
	static int old_integerscalex = -1;
	static int old_integerscaley = -1;
	if (!GLEW_EXT_framebuffer_object)
	{
		return;
	}
	if (!fbotexture)
		glGenTextures(1, &fbotexture);
	if (!framebufferobj)
		glGenFramebuffersEXT(1, &framebufferobj);
	if (old_integerscalex != vw_integerscalex || old_integerscaley != vw_integerscaley)
	{
		glBindTexture(GL_TEXTURE_2D, fbotexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, vw_integerscalex, vw_integerscaley, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebufferobj);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fbotexture, 0);
		
		GLenum framebufferstatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if (framebufferstatus != GL_FRAMEBUFFER_COMPLETE_EXT)
		{
			Quit("Framebuffer Object was not complete!");
		}
		old_integerscalex = vw_integerscalex;
		old_integerscaley = vw_integerscaley;
	}
	else
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebufferobj);
	glViewport(0, 0, vw_integerscalex, vw_integerscaley);
}

void VW_GL_BlitFramebufferObject(int vw, int vh)
{
	if (!GLEW_EXT_framebuffer_object) return;
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	if (!GLEW_EXT_framebuffer_blit)
	{
		float vtxCoords[] = {-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f};
		float fboCoords[] = {0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};
		glVertexPointer(2, GL_FLOAT, 0, vtxCoords);
		glTexCoordPointer(2, GL_FLOAT, 0, fboCoords);
		glUseProgram(0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, fbotexture);
		glDrawArrays(GL_QUADS, 0, 4);
	}
	else
	{
		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);
		glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, framebufferobj);
		glBlitFramebufferEXT(0, 0, vw_integerscalex, vw_integerscaley, 0, 0, vw, vh, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}
}

void VW_GL_GetViewport(int *x, int *y, int *w, int *h)
{
	int winw, winh;
	SDL_GetWindowSize(window,&winw, &winh);
	// TODO: Aspect ratio correction.
	if (x) *x = 0;
	if (y) *y = 0;
	if (w) *w = winw;
	if (h) *h = winh;
}

void VW_GL_SetFullscreen(boolean fullscreen)
{
	SDL_SetWindowFullscreen(window, fullscreen?SDL_WINDOW_FULLSCREEN_DESKTOP:0);
}

void VW_GL_UpdateLineWidth()
{
	glBindTexture(GL_TEXTURE_2D, screentexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, linewidth, 200, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void VW_GL_UpdateGLBuffer()
{
	glBindTexture(GL_TEXTURE_2D, screentexture);
#ifndef VW_MMAP_RINGBUFFER
	if (lastdrawnbuffer + (linewidth * 200) >= VW_VIDEOMEM_SIZE)
	{
		unsigned lineshalf = (VW_VIDEOMEM_SIZE - lastdrawnbuffer) / linewidth;
		unsigned pixhalf = (VW_VIDEOMEM_SIZE - lastdrawnbuffer) % linewidth;
		printf("lineshalf = %d (ldb = %d, vms = %d)\n", lineshalf, lastdrawnbuffer, VW_VIDEOMEM_SIZE);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, linewidth, lineshalf, GL_RED, GL_UNSIGNED_BYTE, &vw_videomem[lastdrawnbuffer]);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, lineshalf, pixhalf, 1, GL_RED, GL_UNSIGNED_BYTE, &vw_videomem[lineshalf * linewidth]);
		glTexSubImage2D(GL_TEXTURE_2D, 0, pixhalf, lineshalf, linewidth - pixhalf, 1, GL_RED, GL_UNSIGNED_BYTE, &vw_videomem[0]);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, lineshalf - 1, linewidth, 200 - lineshalf - 1, GL_RED, GL_UNSIGNED_BYTE, &vw_videomem[pixhalf]);
	}
	else
#endif
	{
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, linewidth, 200, GL_RED, GL_UNSIGNED_BYTE, &vw_videomem[lastdrawnbuffer]);
	}
}

void VW_GL_Present()
{
	float offX = (float)lastdrawnpan/(float)linewidth;
	float endX = offX + (320.0/linewidth);
	float vtxCoords[] = {-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f};
	float texCoords[] = {offX, 1.0, endX, 1.0, endX, 0.0, offX, 0.0};

	int vx, vy, vw, vh;
	VW_GL_GetViewport(&vx,&vy,&vw,&vh);
	vw_integerscalex = (vw/320)*320;
	vw_integerscaley = (vh/200)*200;
	VW_GL_SetupFramebufferObject();
	
	glClearColor(0,1,0,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glUseProgram(vw_shaderprogram);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, screentexture);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, vtxCoords);
	glTexCoordPointer(2, GL_FLOAT, 0, texCoords);

	glDrawArrays(GL_QUADS, 0, 4);
	glViewport(vx,vy,vw,vh);
	VW_GL_BlitFramebufferObject(vw, vh);
	SDL_GL_SwapWindow(window);
}