Aerofoil is a port of John Calhoun's Glider PRO

It currently supports Windows and Android, and the code has been significantly
rewritten to keep the OS-specific code to a minimum and isolated to a few
specific projects.

If you would like to help port Aerofoil to another system, please open an
issue in the issue tracker with the platform that you're interested in
porting to!



Project structure:

Core application binaries:
- Aerofoil: Base application, loads drivers and the application.
- AerofoilSDL: SDL version.  This is not normally included with releases, its
  main purpose is to ease porting.
- AerofoilAndroid: Android project.
- GpApp: Glider PRO application DLL.
- GpAudioDriver_XAudio2: XAudio2 audio driver.
- GpDisplayDriver_D3D11: Direct3D 11 display driver.
- GpInputDriver_XInput: XInput gamepad driver.
- ReleasePackageInstaller: WiX project that builds the Windows Installer package.

Building ReleasePackageInstaller will also automatically run all of the data
import and packaging scripts.  Doing a clean Rebuild on ReleasePackageInstaller
should give you a complete game installation in the "ReleasePkg" directory.

NOTE: ReleasePackageInstaller is currently non-functional because the Visual Studio
WiX tools were discontinued, it needs to be ported to use the CLI tools instead.

To build the game data, run ConvertResources.bat instead.

To package a release, run MakeRelease.bat
  

Libraries:
- Common: Common core definitions used by all projects
- GpCommon: Common types usable by any project.
- GpShell: Static library containing platform-independent portion of the shell app.
- MacRomanConversion: Small library that handles conversion between the Mac
  Roman character set and Unicode.
- PortabilityLayer: A static library that supports interaction with various
  vintage Mac-specific formats and API calls, and provides a lot of
  intermediate functionality like drawing, UI, and resource management.  Some
  parts are capable of running by themselves, others require the support of drivers.
- ShaderSrc: HLSL shader source code.
- WindowsUnicodeToolShim: A wrapper for some tools that provides a "main"-like
  entry point and some file functions in UTF-8 instead of UTF-16 for easier porting.

Data:
- ApplicationResourcePatches: Additions and modifications to the application
  resources from the base game data.
- Documentation: Shippable documentation files.
- GliderProData: Glider PRO assets.
- Resources: Shippable resource files.

Tools:
- CompileShadersD3D11: Compiles D3D11 shaders into C++ files containing the
  compiled shader data.
- EmitWiXVersion: Emits a WiX include file containing the build number.  (Used
  to build the installer.)
- FTagData: Copies a data-only file to a .gpd and creates a .gpf for it
- MakeTimestamp: Tool that dumps the current time to a combined timestamp file.
- MiniRez: Tool that converts a subset of text rez files into a resource file.
- PictChecker: Test utility for validating the PICT loader.
- bin2gp: Converts MacBinary to Aerofoil "triplet" (.gpr, .gpd, and .gpf files)
- flattenmov: Merges a vintage format QuickTime movie with metadata in the
  resource fork and image data in the data fork into a combined .mov file.
- gpr2gpa: Imports resources from a .gpr file into a .gpa resource archive.
- hqx2bin: Converts BinHex to MacBinary.
- hqx2gp: Converts BinHex to Aerofoil "triplet" format.
- unpacktool: Tool that decompresses StuffIt and Compact Pro archives.

Third-party:
- FreeType: FreeType third-party library.
- rapidjson: rapidjson third-party library.
- stb: stb_image third-party library.
- zlib: zlib third-party library.