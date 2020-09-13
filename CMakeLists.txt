cmake_minimum_required(VERSION 2.6)

project(KDreams)

if (CMAKE_SIZEOF_VOID_P MATCHES "8")
    set(BIN_LIBROOT "x86_64")
    set(BIN_SUFFIX ".x86_64")
    set(BIN_RPATH "\$ORIGIN/x86_64")
else()
    set(BIN_LIBROOT "x86")
    set(BIN_SUFFIX ".x86")
    set(BIN_RPATH "\$ORIGIN/x86")
endif()

set(CMAKE_SKIP_BUILD_RPATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH ${BIN_RPATH})
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)


#should be set, this cmake file does not support mac/win32
if (UNIX)
	add_definitions(-DTARGET_OS_LINUX)
	add_definitions(-Wno-write-strings -Wno-attributes -Wno-deprecated)
endif (UNIX)

set(CMAKE_C_FLAGS "-std=gnu99 -funsigned-char")

set(KDREAMS_ID_SRCS
id_heads.h
id_ca.h
id_ca.c
id_in.h
id_in.c
id_mm.h
id_mm.c
id_rf.h
id_rf.c
id_rf_a.c
id_sd.h
id_sd.c
id_us.h
id_us.c
id_us_a.c
id_vw.h
id_vw.c
id_vw_a.c
id_vw_ae.c
id_vw_conv.c
id_vw_gl.c
)

set(KDREAMS_KD_SRCS
kd_stat.c
#kd_stat_steam.cpp
kd_stat_null.c
kd_def.h
kd_act1.c
kd_act2.c
kd_demo.c
kd_keen.c
kd_main.c
kd_play.c
kd_compat.c
icon.c
)

set(KDREAMS_SOFTDISK_SRCS
lzhuff.h
lzhuf.c
lzw.h
sl_file.h
soft.h
soft.c
gelib.h
gelib.c
jam_io.h
jam_io.c

)

add_executable(KDreams
${KDREAMS_ID_SRCS}
${KDREAMS_KD_SRCS}
${KDREAMS_SOFTDISK_SRCS}
)

target_link_libraries(KDreams
#steam_api
GLEW
GL
SDL2
m
)
