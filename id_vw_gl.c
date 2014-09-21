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

void VW_GL_Init()
{
	glGenTextures(1, &screentexture);
	glGenTextures(1, &palettetexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, palettetexture);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB8, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, &VW_EGAPalette[0][0]);
	glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 16, GL_RGB, GL_UNSIGNED_BYTE, &VW_EGAPalette[0][0]);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glActiveTexture(GL_TEXTURE0);
	
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
	{
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, linewidth, 200, GL_RED, GL_UNSIGNED_BYTE, &vw_videomem[lastdrawnbuffer]);
	}
}

void VW_GL_Present()
{
	float offX = lastdrawnpan/320.0;
	float endX = (320.0 + lastdrawnpan)/linewidth;
	float vtxCoords[] = {-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f};
	float texCoords[] = {offX, 1.0, endX, 1.0, endX, 0.0, offX, 0.0};

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
	SDL_GL_SwapWindow(window);
}