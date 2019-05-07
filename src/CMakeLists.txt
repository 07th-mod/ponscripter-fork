add_executable(embed EXCLUDE_FROM_ALL
	embed.cpp
)

add_custom_command(
	OUTPUT resources.cpp
	COMMAND embed > ${CMAKE_CURRENT_BINARY_DIR}/resources.cpp
	DEPENDS embed
)

add_executable(ponscr
	AnimationInfo.cpp
	AnimationInfo.h
	BaseReader.h
	bstrlib.c
	bstrlib.h
	bstrwrap.cpp
	bstrwrap.h
	cp932_encoding.cpp
	cp932_tables.h
	defs.h
	DirectReader.cpp
	DirectReader.h
	DirPaths.cpp
	DirPaths.h
	DirtyRect.cpp
	DirtyRect.h
	encoding.cpp
	encoding.h
	expression.cpp
	expression.h
	font.cpp
	font.h
	Fontinfo.cpp
	Fontinfo.h
	graphics_altivec.cpp
	graphics_altivec.h
	graphics_common.h
	graphics_mmx.cpp
	graphics_mmx.h
	graphics_sse2.cpp
	graphics_sse2.h
	NsaReader.cpp
	NsaReader.h
	Ponscripter.cpp
	PonscripterLabel.cpp
	PonscripterLabel.h
	PonscripterLabel_animation.cpp
	PonscripterLabel_command.cpp
	PonscripterLabel_effect.cpp
	PonscripterLabel_effect_breakup.cpp
	PonscripterLabel_effect_cascade.cpp
	PonscripterLabel_effect_trig.cpp
	PonscripterLabel_event.cpp
	PonscripterLabel_ext.cpp
	PonscripterLabel_file.cpp
	PonscripterLabel_file2.cpp
	PonscripterLabel_image.cpp
	PonscripterLabel_rmenu.cpp
	PonscripterLabel_sound.cpp
	PonscripterLabel_text.cpp
	PonscripterMessage.cpp
	PonscripterMessage.h
	PonscripterUserEvents.h
	prng.cpp
	pstring.cpp
	pstring.h
	resize_image.cpp
	resize_image.h
	${CMAKE_CURRENT_BINARY_DIR}/resources.cpp
	resources.h
	SarReader.cpp
	SarReader.h
	ScriptHandler.cpp
	ScriptHandler.h
	ScriptParser.cpp
	ScriptParser.h
	ScriptParser_command.cpp
	version.h
	winres.h)

target_include_directories(ponscr
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(ponscr
	PRIVATE
		BZip2::BZip2
		Freetype::Freetype
		JPEG::JPEG
		PNG::PNG
		Vorbis::Vorbis
		Vorbis::VorbisFile
		SDL2::Main
		SDL2::Image
		SDL2::Mixer
		SDL2::SMPEG
		ZLIB::ZLIB)

if (ENABLE_GAME_CONTROLLERS)
	target_compile_definitions(ponscr
		PRIVATE
			ENABLE_JOYSTICK)
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_compile_definitions(ponscr
		PRIVATE
			WIN32
			USE_OGG_VORBIS)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	target_compile_definitions(ponscr
		PRIVATE
			MACOSX
			UTF8_CAPTION
			UTF8_FILESYSTEM
			USE_OGG_VORBIS)
elseif (UNIX AND NOT APPLE)
	if (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
		target_compile_definitions(ponscr
			PRIVATE
				CONST_ICONV)
	endif ()
	target_compile_definitions(ponscr
		PRIVATE
			LINUX
			USE_OGG_VORBIS)
endif ()