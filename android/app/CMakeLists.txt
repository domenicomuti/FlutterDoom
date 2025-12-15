cmake_minimum_required(VERSION 3.22.1)

project(doom)

set(SOURCES
	../../ios/linuxdoom-1.10/dart/dart_api_dl.c
	../../ios/linuxdoom-1.10/dart_interface.c
    ../../ios/linuxdoom-1.10/doomdef.c
	../../ios/linuxdoom-1.10/doomstat.c
	../../ios/linuxdoom-1.10/dstrings.c
	../../ios/linuxdoom-1.10/i_system.c
	../../ios/linuxdoom-1.10/i_sound.c
	../../ios/linuxdoom-1.10/i_video.c
	../../ios/linuxdoom-1.10/i_net.c	
	../../ios/linuxdoom-1.10/tables.c	
	../../ios/linuxdoom-1.10/f_finale.c
	../../ios/linuxdoom-1.10/f_wipe.c
	../../ios/linuxdoom-1.10/d_main.c
	../../ios/linuxdoom-1.10/d_net.c
	../../ios/linuxdoom-1.10/d_items.c
	../../ios/linuxdoom-1.10/g_game.c
	../../ios/linuxdoom-1.10/m_menu.c
	../../ios/linuxdoom-1.10/m_misc.c
	../../ios/linuxdoom-1.10/m_argv.c
	../../ios/linuxdoom-1.10/m_bbox.c
	../../ios/linuxdoom-1.10/m_fixed.c
	../../ios/linuxdoom-1.10/m_swap.c
	../../ios/linuxdoom-1.10/m_cheat.c
	../../ios/linuxdoom-1.10/m_random.c
	../../ios/linuxdoom-1.10/am_map.c
	../../ios/linuxdoom-1.10/p_ceilng.c
	../../ios/linuxdoom-1.10/p_doors.c
	../../ios/linuxdoom-1.10/p_enemy.c
	../../ios/linuxdoom-1.10/p_floor.c
	../../ios/linuxdoom-1.10/p_inter.c
	../../ios/linuxdoom-1.10/p_lights.c
	../../ios/linuxdoom-1.10/p_map.c
	../../ios/linuxdoom-1.10/p_maputl.c
	../../ios/linuxdoom-1.10/p_plats.c
	../../ios/linuxdoom-1.10/p_pspr.c
	../../ios/linuxdoom-1.10/p_setup.c
	../../ios/linuxdoom-1.10/p_sight.c
	../../ios/linuxdoom-1.10/p_spec.c
	../../ios/linuxdoom-1.10/p_switch.c
	../../ios/linuxdoom-1.10/p_mobj.c
	../../ios/linuxdoom-1.10/p_telept.c
	../../ios/linuxdoom-1.10/p_tick.c
	../../ios/linuxdoom-1.10/p_saveg.c
	../../ios/linuxdoom-1.10/p_user.c
	../../ios/linuxdoom-1.10/r_bsp.c
	../../ios/linuxdoom-1.10/r_data.c
	../../ios/linuxdoom-1.10/r_draw.c
	../../ios/linuxdoom-1.10/r_main.c
	../../ios/linuxdoom-1.10/r_plane.c
	../../ios/linuxdoom-1.10/r_segs.c
	../../ios/linuxdoom-1.10/r_sky.c
	../../ios/linuxdoom-1.10/r_things.c
	../../ios/linuxdoom-1.10/w_wad.c
	../../ios/linuxdoom-1.10/wi_stuff.c
	../../ios/linuxdoom-1.10/v_video.c
	../../ios/linuxdoom-1.10/st_lib.c
	../../ios/linuxdoom-1.10/st_stuff.c
	../../ios/linuxdoom-1.10/hu_stuff.c
	../../ios/linuxdoom-1.10/hu_lib.c
	../../ios/linuxdoom-1.10/s_sound.c
	../../ios/linuxdoom-1.10/z_zone.c
	../../ios/linuxdoom-1.10/info.c
	../../ios/linuxdoom-1.10/sounds.c
)

add_library(doom SHARED ${SOURCES})

target_compile_options(doom PRIVATE
    -g -Wall -DNORMALUNIX -DLINUX
	-Wno-unused-const-variable
)

target_link_libraries(doom PRIVATE
	m
)

if(ANDROID)
	target_link_libraries(doom PRIVATE
		log
	)
endif()