cmake_minimum_required(VERSION 3.26)
project (Aerofoil)
SET(PLATFORM "X" CACHE STRING "Defines the target platform")
SET(EXECNAME "AerofoilX" CACHE STRING "Defines the exec name")

message(${CMAKE_BINARY_DIR})

# Use Release build type by default
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
	set(CMAKE_BUILD_TYPE Release)
	message("Build type unspecified, using Release")
endif()

# Enable LTO by default if supported
if("${CMAKE_INTERPROCEDURAL_OPTIMIZATION}" STREQUAL "")
	include(CheckIPOSupported)
	check_ipo_supported(RESULT IPO_SUPPORTED)
	if(IPO_SUPPORTED)
		set(CMAKE_INTERPROCEDURAL_OPTIMIZATION On)
		message("Compiler supports LTO, enabling automatically")
	endif()
endif()

# FIXME: Clang treats this as an error by default; fixing the source rather
# than downgrading the error to a warning would be a better solution.
add_compile_options(
	$<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang>:-Wno-error=c++11-narrowing>
	)

find_package(SDL2 REQUIRED)

if(PLATFORM STREQUAL "MAC")
	SET(EXECNAME "AerofoilMac" CACHE STRING "Defines the exec name" FORCE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	add_definitions(-D__MACOS__)
endif()

message("Building ${EXECNAME} for: ${PLATFORM}")

add_definitions(-DGP_DEBUG_CONFIG=0)
add_definitions(-DNDEBUG=1)


add_library(stb STATIC
	stb/stb_image_write.c
	)

add_library(zlib STATIC
	zlib/adler32.c
	zlib/crc32.c
	zlib/deflate.c
	zlib/inffast.c
	zlib/inflate.c
	zlib/inftrees.c
	zlib/trees.c
	zlib/zutil.c
	)

add_library(MacRomanConversion STATIC
	MacRomanConversion/MacRomanConversion.cpp
	)

add_library(PortabilityLayer STATIC
	PortabilityLayer/AntiAliasTable.cpp
	PortabilityLayer/AppEventHandler.cpp
	PortabilityLayer/BinHex4.cpp
	PortabilityLayer/BitmapImage.cpp
	PortabilityLayer/ByteSwap.cpp
	PortabilityLayer/CFileStream.cpp
	PortabilityLayer/CompositeRenderedFont.cpp
	PortabilityLayer/DeflateCodec.cpp
	PortabilityLayer/DialogManager.cpp
	PortabilityLayer/DisplayDeviceManager.cpp
	PortabilityLayer/EllipsePlotter.cpp
	PortabilityLayer/FileBrowserUI.cpp
	PortabilityLayer/FileManager.cpp
	PortabilityLayer/FileSectionStream.cpp
	PortabilityLayer/FontFamily.cpp
	PortabilityLayer/FontManager.cpp
	PortabilityLayer/FontRenderer.cpp
	PortabilityLayer/GPArchive.cpp
	PortabilityLayer/HostSuspendHook.cpp
	PortabilityLayer/IconLoader.cpp
	PortabilityLayer/InflateStream.cpp
	PortabilityLayer/InputManager.cpp
	PortabilityLayer/LinePlotter.cpp
	PortabilityLayer/MacBinary2.cpp
	PortabilityLayer/MacFileInfo.cpp
	PortabilityLayer/MacFileMem.cpp
	PortabilityLayer/MemoryManager.cpp
	PortabilityLayer/MemReaderStream.cpp
	PortabilityLayer/MenuManager.cpp
	PortabilityLayer/MMHandleBlock.cpp
	PortabilityLayer/PLApplication.cpp
	PortabilityLayer/PLButtonWidget.cpp
	PortabilityLayer/PLControlDefinitions.cpp
	PortabilityLayer/PLCore.cpp
	PortabilityLayer/PLCTabReducer.cpp
	PortabilityLayer/PLDialogs.cpp
	PortabilityLayer/PLDrivers.cpp
	PortabilityLayer/PLEditboxWidget.cpp
	PortabilityLayer/PLEventQueue.cpp
	PortabilityLayer/PLHacks.cpp
	PortabilityLayer/PLHandle.cpp
	PortabilityLayer/PLIconWidget.cpp
	PortabilityLayer/PLImageWidget.cpp
	PortabilityLayer/PLInvisibleWidget.cpp
	PortabilityLayer/PLKeyEncoding.cpp
	PortabilityLayer/PLLabelWidget.cpp
	PortabilityLayer/PLMenus.cpp
	PortabilityLayer/PLMovies.cpp
	PortabilityLayer/PLNumberFormatting.cpp
	PortabilityLayer/PLPopupMenuWidget.cpp
	PortabilityLayer/PLQDOffscreen.cpp
	PortabilityLayer/PLQDraw.cpp
	PortabilityLayer/PLResourceManager.cpp
	PortabilityLayer/PLResources.cpp
	PortabilityLayer/PLScrollBarWidget.cpp
	PortabilityLayer/PLSound.cpp
	PortabilityLayer/PLStandardColors.cpp
	PortabilityLayer/PLStringCompare.cpp
	PortabilityLayer/PLSysCalls.cpp
	PortabilityLayer/PLTimeTaggedVOSEvent.cpp
	PortabilityLayer/PLWidgets.cpp
	PortabilityLayer/QDGraf.cpp
	PortabilityLayer/QDManager.cpp
	PortabilityLayer/QDPictDecoder.cpp
	PortabilityLayer/QDPictEmitContext.cpp
	PortabilityLayer/QDPictHeader.cpp
	PortabilityLayer/QDPixMap.cpp
	PortabilityLayer/QDPort.cpp
	PortabilityLayer/QDStandardPalette.cpp
	PortabilityLayer/RandomNumberGenerator.cpp
	PortabilityLayer/ResolveCachingColor.cpp
	PortabilityLayer/ResourceCompiledRef.cpp
	PortabilityLayer/ResourceFile.cpp
	PortabilityLayer/ScanlineMask.cpp
	PortabilityLayer/ScanlineMaskBuilder.cpp
	PortabilityLayer/ScanlineMaskConverter.cpp
	PortabilityLayer/ScanlineMaskIterator.cpp
	PortabilityLayer/SimpleGraphic.cpp
	PortabilityLayer/TextPlacer.cpp
	PortabilityLayer/UTF8.cpp
	PortabilityLayer/WindowDef.cpp
	PortabilityLayer/WindowManager.cpp
	PortabilityLayer/WorkerThread.cpp
	PortabilityLayer/XModemCRC.cpp
	PortabilityLayer/ZipFileProxy.cpp
	)

target_include_directories(PortabilityLayer PRIVATE
	Common
	GpCommon
	PortabilityLayer
	zlib
	rapidjson/include
	MacRomanConversion
	stb
	)

target_compile_options(PortabilityLayer PRIVATE -Wno-multichar)

target_link_libraries(PortabilityLayer PRIVATE zlib MacRomanConversion stb)


add_library(GpShell STATIC
	GpShell/GpAppEnvironment.cpp
	GpShell/GpAudioDriverFactory.cpp
	GpShell/GpDisplayDriverFactory.cpp
	GpShell/GpFontHandlerFactory.cpp
	GpShell/GpGlobalConfig.cpp
	GpShell/GpInputDriverFactory.cpp
	GpShell/GpMain.cpp
	GpShell/GpVOSEventQueue.cpp
	)

target_include_directories(GpShell PRIVATE
	Common
	GpCommon
	PortabilityLayer
	)

add_library(GpApp STATIC
	GpApp/About.cpp
	GpApp/AnimCursor.cpp
	GpApp/AppleEvents.cpp
	GpApp/Banner.cpp
	GpApp/ColorUtils.cpp
	GpApp/Coordinates.cpp
	GpApp/DialogUtils.cpp
	GpApp/DynamicMaps.cpp
	GpApp/Dynamics.cpp
	GpApp/Dynamics2.cpp
	GpApp/Dynamics3.cpp
	GpApp/Environ.cpp
	GpApp/Events.cpp
	GpApp/FileError.cpp
	GpApp/GameOver.cpp
	GpApp/GpAppInterface.cpp
	GpApp/Grease.cpp
	GpApp/HighScores.cpp
	GpApp/House.cpp
	GpApp/HouseInfo.cpp
	GpApp/HouseIO.cpp
	GpApp/HouseLegal.cpp
	GpApp/Input.cpp
	GpApp/Interactions.cpp
	GpApp/InterfaceInit.cpp
	GpApp/Link.cpp
	GpApp/Main.cpp
	GpApp/MainMenuUI.cpp
	GpApp/MainWindow.cpp
	GpApp/Map.cpp
	GpApp/Marquee.cpp
	GpApp/Menu.cpp
	GpApp/Modes.cpp
	GpApp/Music.cpp
	GpApp/ObjectAdd.cpp
	GpApp/ObjectDraw.cpp
	GpApp/ObjectDraw2.cpp
	GpApp/ObjectDrawAll.cpp
	GpApp/ObjectEdit.cpp
	GpApp/ObjectInfo.cpp
	GpApp/ObjectRects.cpp
	GpApp/Objects.cpp
	GpApp/Play.cpp
	GpApp/Player.cpp
	GpApp/Prefs.cpp
	GpApp/RectUtils.cpp
	GpApp/Render.cpp
	GpApp/Room.cpp
	GpApp/RoomGraphics.cpp
	GpApp/RoomInfo.cpp
	GpApp/RubberBands.cpp
	GpApp/SavedGames.cpp
	GpApp/Scoreboard.cpp
	GpApp/Scrap.cpp
	GpApp/SelectHouse.cpp
	GpApp/Settings.cpp
	GpApp/Sound.cpp
	GpApp/SoundSync_Cpp11.cpp
	GpApp/SourceExport.cpp
	GpApp/StringUtils.cpp
	GpApp/StructuresInit.cpp
	GpApp/StructuresInit2.cpp
	GpApp/Tools.cpp
	GpApp/Transit.cpp
	GpApp/Transitions.cpp
	GpApp/Triggers.cpp
	GpApp/Trip.cpp
	GpApp/Utilities.cpp
	GpApp/WindowUtils.cpp
	)

target_compile_options(GpApp PRIVATE -Wno-multichar)

target_include_directories(GpApp PRIVATE
	Common
	GpCommon
	PortabilityLayer
	)

target_link_libraries(GpApp PRIVATE PortabilityLayer)

if(CMAKE_HOST_UNIX)
	set(EXEC_SOURCES
		AerofoilPortable/GpSystemServices_POSIX.cpp
		AerofoilPortable/GpThreadEvent_Cpp11.cpp
		AerofoilPortable/GpAllocator_C.cpp
		AerofoilSDL/GpAudioDriver_SDL2.cpp
		AerofoilSDL/GpDisplayDriver_SDL_GL2.cpp
		AerofoilSDL/GpInputDriver_SDL_Gamepad.cpp
		AerofoilSDL/ShaderCode/CopyQuadP.cpp
		AerofoilSDL/ShaderCode/DrawQuad32P.cpp
		AerofoilSDL/ShaderCode/DrawQuadPaletteP.cpp
		AerofoilSDL/ShaderCode/DrawQuadV.cpp
		AerofoilSDL/ShaderCode/ScaleQuadP.cpp
		AerofoilX/GpMain_SDL_X.cpp
		AerofoilX/GpLogDriver_X.cpp
		AerofoilX/GpSystemServices_X.cpp
		AerofoilX/GpFileSystem_X.cpp
		)

	set(EXEC_LIBS
		${SDL2_LIBRARIES}
		GpApp
		GpShell
		PortabilityLayer
		)

	set(EXEC_INC_DIRS
		Common
		GpCommon
		GpShell
		AerofoilSDL
		AerofoilPortable
		PortabilityLayer
		${SDL2_INCLUDE_DIRS}
		)
	if(PLATFORM STREQUAL "MAC")
		list(APPEND EXEC_SOURCES
			AerofoilMac/AerofoilMac/AerofoilApplication.mm
			AerofoilMac/AerofoilMac/MacInit.mm
			)
		list(APPEND EXEC_INC_DIRS
			AerofoilMac/AerofoilMac
			)
		list(APPEND EXEC_LIBS
			"-framework Cocoa"
			)
	endif(PLATFORM STREQUAL "MAC")

	add_executable(${EXECNAME} ${EXEC_SOURCES})
	target_include_directories(${EXECNAME} PRIVATE ${EXEC_INC_DIRS})
	target_link_libraries(${EXECNAME} ${EXEC_LIBS})
endif()


add_executable(flattenmov EXCLUDE_FROM_ALL flattenmov/flattenmov.cpp AerofoilPortable/GpAllocator_C.cpp)
target_include_directories(flattenmov PRIVATE
	Common
	GpCommon
	PortabilityLayer
	AerofoilPortable
	)
target_link_libraries(flattenmov PortabilityLayer)

add_executable(bin2gp EXCLUDE_FROM_ALL bin2gp/bin2gp.cpp AerofoilPortable/GpAllocator_C.cpp)
target_include_directories(bin2gp PRIVATE
	Common
	GpCommon
	PortabilityLayer
	AerofoilPortable
	)
target_link_libraries(bin2gp PortabilityLayer)

add_executable(hqx2bin EXCLUDE_FROM_ALL hqx2bin/hqx2bin.cpp AerofoilPortable/GpAllocator_C.cpp)
target_include_directories(hqx2bin PRIVATE
	Common
	GpCommon
	PortabilityLayer
	AerofoilPortable
	)
target_link_libraries(hqx2bin PortabilityLayer)

add_executable(hqx2gp EXCLUDE_FROM_ALL
	hqx2gp/hqx2gp.cpp
	AerofoilPortable/GpAllocator_C.cpp
	WindowsUnicodeToolShim/UnixUnicodeToolShim.cpp
	)
target_include_directories(hqx2gp PRIVATE
	Common
	GpCommon
	PortabilityLayer
	AerofoilPortable
	WindowsUnicodeToolShim
	)
target_link_libraries(hqx2gp PortabilityLayer)

add_executable(gpr2gpa EXCLUDE_FROM_ALL
	gpr2gpa/gpr2gpa.cpp
	gpr2gpa/macedec.cpp
	AerofoilPortable/GpAllocator_C.cpp
	WindowsUnicodeToolShim/UnixUnicodeToolShim.cpp
	)
target_include_directories(gpr2gpa PRIVATE
	Common
	GpCommon
	PortabilityLayer
	AerofoilPortable
	MacRomanConversion
	WindowsUnicodeToolShim
	rapidjson/include
	zlib
	)
target_link_libraries(gpr2gpa PortabilityLayer MacRomanConversion zlib)

add_executable(MakeTimestamp EXCLUDE_FROM_ALL
	MakeTimestamp/MakeTimestamp.cpp
	WindowsUnicodeToolShim/UnixUnicodeToolShim.cpp
	)
target_include_directories(MakeTimestamp PRIVATE PortabilityLayer WindowsUnicodeToolShim)
target_link_libraries(MakeTimestamp PortabilityLayer)

add_executable(FTagData EXCLUDE_FROM_ALL
	FTagData/FTagData.cpp
	WindowsUnicodeToolShim/UnixUnicodeToolShim.cpp
	)
target_include_directories(FTagData PRIVATE
	Common
	GpCommon
	PortabilityLayer
	AerofoilPortable
	WindowsUnicodeToolShim
	)
target_link_libraries(FTagData PortabilityLayer)

add_executable(MergeGPF EXCLUDE_FROM_ALL
	MergeGPF/MergeGPF.cpp
	AerofoilPortable/GpAllocator_C.cpp
	WindowsUnicodeToolShim/UnixUnicodeToolShim.cpp
	)
target_include_directories(MergeGPF PRIVATE
	Common
	GpCommon
	PortabilityLayer
	AerofoilPortable
	WindowsUnicodeToolShim
	)
target_link_libraries(MergeGPF PortabilityLayer)

add_executable(MiniRez EXCLUDE_FROM_ALL MiniRez/MiniRez.cpp WindowsUnicodeToolShim/UnixUnicodeToolShim.cpp)
target_include_directories(MiniRez PRIVATE
	Common
	GpCommon
	PortabilityLayer
	WindowsUnicodeToolShim
	)
target_link_libraries(MiniRez PortabilityLayer)

add_executable(HouseTool EXCLUDE_FROM_ALL
	HouseTool/HouseTool.cpp
	WindowsUnicodeToolShim/UnixUnicodeToolShim.cpp
	)
target_include_directories(HouseTool PRIVATE
	Common
	GpCommon
	PortabilityLayer
	MacRomanConversion
	WindowsUnicodeToolShim
	)
target_link_libraries(HouseTool PortabilityLayer MacRomanConversion)

add_executable(unpacktool EXCLUDE_FROM_ALL
	unpacktool/ArchiveDescription.cpp
	unpacktool/BWT.cpp
	unpacktool/CompactProLZHDecompressor.cpp
	unpacktool/CompactProLZHRLEDecompressor.cpp
	unpacktool/CompactProParser.cpp
	unpacktool/CompactProRLEDecompressor.cpp
	unpacktool/CRC.cpp
	unpacktool/CSInputBuffer.cpp
	unpacktool/DecompressorProxyReader.cpp
	unpacktool/LZSSDecompressor.cpp
	unpacktool/LZW.cpp
	unpacktool/LZWDecompressor.cpp
	unpacktool/NullDecompressor.cpp
	unpacktool/PrefixCode.cpp
	unpacktool/RLE90Decompressor.cpp
	unpacktool/StringCommon.cpp
	unpacktool/StuffIt13Decompressor.cpp
	unpacktool/StuffIt5Parser.cpp
	unpacktool/StuffItArsenicDecompressor.cpp
	unpacktool/StuffItCommon.cpp
	unpacktool/StuffItHuffmanDecompressor.cpp
	unpacktool/StuffItParser.cpp
	unpacktool/unpacktool.cpp
	WindowsUnicodeToolShim/UnixUnicodeToolShim.cpp
	)
target_include_directories(unpacktool PRIVATE
	Common
	GpCommon
	PortabilityLayer
	MacRomanConversion
	WindowsUnicodeToolShim
	)
target_link_libraries(unpacktool PortabilityLayer MacRomanConversion)


find_package(Freetype)
if(FREETYPE_FOUND)
	add_library(GpFontHandler_FreeType2 STATIC
		GpFontHandler_FreeType2/GpFontHandler_FreeType2.cpp
		)

	target_include_directories(GpFontHandler_FreeType2 PRIVATE
		Common
		GpCommon
		"${FREETYPE_INCLUDE_DIR_ft2build}"
		)

	target_link_libraries(GpFontHandler_FreeType2 PRIVATE Freetype::Freetype)


	add_executable(GenerateFonts EXCLUDE_FROM_ALL
		GenerateFonts/GenerateFonts.cpp
		AerofoilPortable/GpAllocator_C.cpp
		WindowsUnicodeToolShim/UnixUnicodeToolShim.cpp
		)
	target_include_directories(GenerateFonts PRIVATE
		Common
		GpCommon
		PortabilityLayer
		AerofoilPortable
		WindowsUnicodeToolShim
		)
	target_link_libraries(GenerateFonts PortabilityLayer GpFontHandler_FreeType2)
endif()

add_executable(ConvertColorCursors EXCLUDE_FROM_ALL
	ConvertColorCursors/ConvertColorCursors.cpp
	AerofoilPortable/GpAllocator_C.cpp
	stb/stb_image_write.c
	WindowsUnicodeToolShim/UnixUnicodeToolShim.cpp
	)
target_include_directories(ConvertColorCursors PRIVATE
	Common
	GpCommon
	PortabilityLayer
	AerofoilPortable
	stb
	WindowsUnicodeToolShim
	)
target_link_libraries(ConvertColorCursors PortabilityLayer)


add_custom_target(BuildDirs
	BYPRODUCTS Packaged tmp
	COMMAND "${CMAKE_COMMAND}" -E make_directory Packaged/Houses tmp
	VERBATIM
	)

set(DATA_FILES)

function(add_data_file NAME)
	list(APPEND DATA_FILES "Packaged/${NAME}")
	set(DATA_FILES "${DATA_FILES}" PARENT_SCOPE)

	cmake_parse_arguments(PARSE_ARGV 1 ARG "" "" "")
	set(TMPDIR "${CMAKE_CURRENT_BINARY_DIR}/tmp/${NAME}")
	list(TRANSFORM ARG_UNPARSED_ARGUMENTS
		REPLACE {TMPDIR} "${TMPDIR}"
		)

	add_custom_command(
		OUTPUT "Packaged/${NAME}"
		DEPENDS BuildDirs
		COMMAND "${CMAKE_COMMAND}" -E make_directory "${TMPDIR}"
		${ARG_UNPARSED_ARGUMENTS}
		COMMAND "${CMAKE_COMMAND}" -E rename
			"${TMPDIR}/${NAME}"
			"${CMAKE_CURRENT_BINARY_DIR}/Packaged/${NAME}"
		COMMAND "${CMAKE_COMMAND}" -E rm -r -- "${TMPDIR}"
		VERBATIM
		)
endfunction()

add_data_file(ApplicationResources.gpf
	DEPENDS
		MiniRez gpr2gpa FTagData MergeGPF "GliderProData/Glider PRO.r"
		ApplicationResourcePatches/manifest.json DefaultTimestamp.timestamp
	COMMAND MiniRez
		"GliderProData/Glider PRO.r"
		"{TMPDIR}/ApplicationResources.gpr"
	COMMAND gpr2gpa
		"{TMPDIR}/ApplicationResources.gpr"
		DefaultTimestamp.timestamp
		"{TMPDIR}/ApplicationResources.gpa"
		-patch ApplicationResourcePatches/manifest.json
	COMMAND FTagData
		DefaultTimestamp.timestamp
		"{TMPDIR}/ApplicationResources.gpf"
		data ozm5 0 0 locked
	COMMAND MergeGPF
		"{TMPDIR}/ApplicationResources.gpf"
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	)

file(GLOB_RECURSE FONT_DEPS RELATIVE "${CMAKE_SOURCE_DIR}" CONFIGURE_DEPENDS Resources/Fonts/*)
add_data_file(Fonts.gpf
	DEPENDS GenerateFonts MiniRez gpr2gpa FTagData MergeGPF ${FONT_DEPS}
	COMMAND GenerateFonts "${CMAKE_SOURCE_DIR}/Resources" {TMPDIR}
	COMMAND MiniRez "${CMAKE_SOURCE_DIR}/Empty.r" {TMPDIR}/Fonts.gpr
	COMMAND gpr2gpa
		{TMPDIR}/Fonts.gpr
		"${CMAKE_SOURCE_DIR}/DefaultTimestamp.timestamp"
		{TMPDIR}/Fonts.gpa
		-patch {TMPDIR}/FontCacheManifest.json
	COMMAND FTagData
		DefaultTimestamp.timestamp
		{TMPDIR}/Fonts.gpf
		data ozm5 0 0 locked
	COMMAND MergeGPF {TMPDIR}/Fonts.gpf
	)

# These files are committed to the repo and aren't currently useful on non-Windows systems anyway.
#file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Aerofoil/ConvertedResources)
#
#set(CONVERTED_ICONS
#	Aerofoil/ConvertedResources/Large128.ico
#	Aerofoil/ConvertedResources/Large129.ico
#	Aerofoil/ConvertedResources/Large130.ico
#	Aerofoil/ConvertedResources/Large131.ico
#	Aerofoil/ConvertedResources/Large132.ico
#	Aerofoil/ConvertedResources/Large133.ico
#	Aerofoil/ConvertedResources/Small128.ico
#	Aerofoil/ConvertedResources/Small129.ico
#	Aerofoil/ConvertedResources/Small130.ico
#	Aerofoil/ConvertedResources/Small133.ico
#	)
#
#add_custom_command(
#	OUTPUT ${CONVERTED_ICONS}
#	DEPENDS ConvertColorCursors Packaged/ApplicationResources.gpr
#	COMMAND ConvertColorCursors
#	)
#add_custom_target(Icons DEPENDS ${CONVERTED_ICONS})

set(HOUSE_FILES)

function(add_house NAME)
	cmake_parse_arguments(PARSE_ARGV 0 ARG
		""
		PATCH
		""
		)

	if(ARG_PATCH)
		set(PATCH_ARGS -patch "${CMAKE_SOURCE_DIR}/HousePatches/${ARG_PATCH}")
	endif()

	set(BASE_PATH "Packaged/Houses/${NAME}")
	list(APPEND HOUSE_FILES "${BASE_PATH}.gpf")

	set(BYPRODUCTS "${BASE_PATH}.gpr" "${BASE_PATH}.gpa" "${BASE_PATH}.gpd")
	set(BINHEX_SRC "${CMAKE_SOURCE_DIR}/GliderProData/Houses/${NAME}.binhex")
	set(TS "${CMAKE_SOURCE_DIR}/DefaultTimestamp.timestamp")

	add_custom_command(
		OUTPUT
			"${BASE_PATH}.gpf"
		BYPRODUCTS
			${BYPRODUCTS}
		DEPENDS hqx2gp gpr2gpa MergeGPF BuildDirs "${BINHEX_SRC}" "${TS}"
		COMMAND hqx2gp
			"${BINHEX_SRC}"
			"${TS}"
			"${BASE_PATH}"
		COMMAND gpr2gpa
			"${BASE_PATH}.gpr"
			"${TS}"
			"${BASE_PATH}.gpa"
			${PATCH_ARGS}
		${HOUSE_EXTRA_COMMANDS}
		COMMAND MergeGPF
			"${BASE_PATH}.gpf"
		COMMAND "${CMAKE_COMMAND}" -E rm
			${BYPRODUCTS}
		VERBATIM
		)

	set(MOV_GPA_SRC "${CMAKE_SOURCE_DIR}/GliderProData/ConvertedMovies/${NAME}.mov.gpa")
	if(EXISTS "${MOV_GPA_SRC}")
		list(APPEND HOUSE_FILES "${BASE_PATH}.mov.gpf")

		add_custom_command(
			OUTPUT
				"${BASE_PATH}.mov.gpf"
			BYPRODUCTS
				"${BASE_PATH}.mov.gpa"
			DEPENDS FTagData MergeGPF BuildDirs "${MOV_GPA_SRC}" "${TS}"
			COMMAND FTagData
				"${TS}"
				"${BASE_PATH}.mov.gpf"
				MooV ozm5 0 0 locked
			COMMAND "${CMAKE_COMMAND}" -E copy
				-t "Packaged/Houses"
				"${MOV_GPA_SRC}"
			COMMAND MergeGPF
				"${BASE_PATH}.mov.gpf"
			COMMAND "${CMAKE_COMMAND}" -E rm
				"${BASE_PATH}.mov.gpa"
			VERBATIM
			)
	endif()
	set(HOUSE_FILES "${HOUSE_FILES}" PARENT_SCOPE)
endfunction()

add_house("Art Museum")
add_house("California or Bust!")

set(HOUSE_EXTRA_COMMANDS
	DEPENDS HouseTool
	COMMAND HouseTool
		patch "Packaged/Houses/Castle o' the Air.gpd" .firstRoom 77
	)
add_house("Castle o' the Air")
unset(HOUSE_EXTRA_COMMANDS)

add_house("CD Demo House")
add_house("Davis Station")
add_house("Demo House")
add_house("Fun House")
add_house("Grand Prix" PATCH "GrandPrix.json")
add_house("ImagineHouse PRO II" PATCH "ImagineHousePROII.json")
add_house("In The Mirror" PATCH "InTheMirror.json")
add_house("Land of Illusion")
add_house("Leviathan" PATCH "Leviathan.json")
add_house("Metropolis")
add_house("Nemo's Market")
add_house("Rainbow's End" PATCH "RainbowsEnd.json")
add_house("Slumberland")
add_house("SpacePods")
add_house("Teddy World")
add_house("The Asylum Pro")
add_house("Titanic")

add_custom_target(Executable DEPENDS "${EXECNAME}")

add_custom_target(Resources ALL
	DEPENDS
		${DATA_FILES}
		${HOUSE_FILES}
	)

set(TOOL_EXES
	flattenmov
	bin2gp
	hqx2bin
	hqx2gp
	MakeTimestamp
	FTagData
	gpr2gpa
	unpacktool
	MergeGPF
	)
add_custom_target(Tools ALL DEPENDS ${TOOL_EXES})


list(TRANSFORM DATA_FILES PREPEND "${CMAKE_CURRENT_BINARY_DIR}/")
list(TRANSFORM HOUSE_FILES PREPEND "${CMAKE_CURRENT_BINARY_DIR}/")

install(TARGETS "${EXECNAME}" COMPONENT Executable)
install(FILES ${DATA_FILES} DESTINATION lib/aerofoil/Packaged COMPONENT Resources)
install(FILES ${HOUSE_FILES} DESTINATION lib/aerofoil/Packaged/Houses COMPONENT Resources)
install(TARGETS ${TOOL_EXES} DESTINATION lib/aerofoil/tools COMPONENT Tools)

if (UNIX AND NOT APPLE)
	install(FILES Resources/Linux/io.github.elasota.aerofoil.metainfo.xml
		DESTINATION share/appdata
		PERMISSIONS WORLD_READ GROUP_READ GROUP_EXECUTE OWNER_READ OWNER_WRITE)
	install(FILES Resources/Linux/io.github.elasota.aerofoil.desktop
		DESTINATION share/applications
		PERMISSIONS WORLD_EXECUTE WORLD_READ GROUP_READ GROUP_EXECUTE OWNER_READ OWNER_WRITE OWNER_EXECUTE)
	install(FILES Resources/Linux/io.github.elasota.aerofoil.svg
		DESTINATION share/icons/hicolor/scalable/apps
		PERMISSIONS WORLD_READ GROUP_READ GROUP_EXECUTE OWNER_READ OWNER_WRITE)
endif()
