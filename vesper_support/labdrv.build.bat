@echo off

rem ============================================================================
rem windows xp 32 비트는 win xp x86 build environment 로 빌드
rem		ddkbuild -verbose -WIN7XP [checked | free] . -cZ
rem
rem windows xp 64 비트는 win 2003 x64 build environment 로 빌드
rem		ddkbuild -verbose -WIN7NETA64 [checked | free] . -cZ
rem ============================================================================




set DDKBUILD=.\ddkbuild.bat
set _proj_dir=.
set _target_name=vesper

rem SOURCES 파일에서 사용 할 TARGETNAME 변수를 설정
rem 
set TARGETNAME=%_target_name%

cls
echo ===========================================================================
echo               build script by somma ( fixbrain@gmail.com )
echo ===========================================================================

rem  labdrv.bat clean
rem 	or
rem  labdrv.bat [x86 | x64] [checked | free]

rem -----------------------------------------------------------------------------
rem 	parameter check
rem -----------------------------------------------------------------------------
rem  #1 set environment variables
if /i "%1"=="clean" (
	echo ----------------------------------
	echo          clean x86 checked
	echo ----------------------------------
    set _target_platform=x86
	set _output_directory=..\bin_x86
	set _target_config=-WIN7XP checked
	set _target_path=%_proj_dir%\objchk_wxp_x86\i386\%_target_name%.sys
	set _target_dir=%_proj_dir%\objchk_wxp_x86
    set _log_file=%_proj_dir%\buildchk_wxp_x86
    call :_dump_environment_variables
    call :_delete_files

    echo ----------------------------------
	echo          clean x86 free
	echo ----------------------------------
	set _target_platform=x86
	set _output_directory=..\bin_x86
	set _target_config=-WIN7XP free
	set _target_path=%_proj_dir%\objfre_wxp_x86\i386\%_target_name%.sys
	set _target_dir=%_proj_dir%\objfre_wxp_x86
    set _log_file=%_proj_dir%\buildfre_wxp_x86
    call :_dump_environment_variables
    call :_delete_files

	echo ----------------------------------
	echo          clean x64 checked
	echo ----------------------------------
	set _target_platform=x64
	set _output_directory=..\bin_x64
	set _target_config=-WIN7NETA64 checked
	set _target_path=%_proj_dir%\objchk_wnet_amd64\amd64\%_target_name%.sys
	set _target_dir=%_proj_dir%\objchk_wnet_amd64
    set _log_file=%_proj_dir%\buildchk_wnet_amd64
	call :_dump_environment_variables
    call :_delete_files

	echo ----------------------------------
	echo          clean x64 free
	echo ----------------------------------
	set _target_platform=x64
	set _output_directory=..\bin_x64
	set _target_config=-WIN7NETA64 checked
	set _target_path=%_proj_dir%\objfre_wnet_amd64\amd64\%_target_name%.sys
	set _target_dir=%_proj_dir%\objfre_wnet_amd64
    set _log_file=%_proj_dir%\buildfre_wnet_amd64
	call :_dump_environment_variables
    call :_delete_files

	goto :eof
) else if /i "%1" == "x86" (
	set _target_platform=x86
	set _output_directory=..\bin_x86

	if /i "%2" == "checked"		(
        echo ----------------------------------
        echo          build x86 checked
        echo ----------------------------------
		set _target_config=-WIN7XP checked
		set _target_path=%_proj_dir%\objchk_wxp_x86\i386\%_target_name%.sys
		set _target_dir=%_proj_dir%\objchk_wxp_x86
	) else if /i "%2" == "free" (
        echo ----------------------------------
        echo          build x86 free
        echo ----------------------------------
		set _target_config=-WIN7XP free
		set _target_path=%_proj_dir%\objfre_wxp_x86\i386\%_target_name%.sys
		set _target_dir=%_proj_dir%\objfre_wxp_x86
	) else (
		echo invalid parameter. 2nd param = %2
		goto :EOF
	)
) else if /i "%1%" == "x64" (
	set _target_platform=x64
	set _output_directory=..\bin_x64

	if /i "%2" == "checked"		(
        echo ----------------------------------
        echo          build x64 checked
        echo ----------------------------------
		set _target_config=-WIN7NETA64 checked
		set _target_path=%_proj_dir%\objchk_wnet_amd64\amd64\%_target_name%.sys
		set _target_dir=%_proj_dir%\objchk_wnet_amd64
	) else if /i "%2" == "free" (
        echo ----------------------------------
        echo          build x64 free
        echo ----------------------------------
		set _target_config=-WIN7NETA64 free
		set _target_path=%_proj_dir%\objfre_wnet_amd64\amd64\%_target_name%.sys
		set _target_dir=%_proj_dir%\objfre_wnet_amd64
	) else (
		echo  invalid parameter. 2nd param = %2
		goto :EOF
	)
) else (
    echo ----------------------------------
    echo invalid parameters...
    echo(
    echo usage
	echo        labdrv.build.bat  clean
	echo        labdrv.build.bat  x86/x64    checked/free
	goto :eof
)


rem -----------------------------------------------------------------------------
rem 	BUILD
rem -----------------------------------------------------------------------------
:_build_and_copy_bin
call :_dump_environment_variables
echo %DDKBUILD% -verbose %_target_config% %_proj_dir% -cZ
call %DDKBUILD% -verbose %_target_config% %_proj_dir% -cZ

echo [*] copy driver files [ %_target_path% ] to [ %_output_directory% ]
copy "%_target_path%" "%_output_directory%"

echo [*] copy driver files to debug target
copy /Y "%_target_path%" v:
goto :EOF


rem -----------------------------------------------------------------------------
rem 	DUMP BUILD VARIABLES (sub routine)
rem -----------------------------------------------------------------------------
:_dump_environment_variables
echo    _proj_dir           = %_proj_dir%
echo    _target_name        = %_target_name%
echo    _target_platform    = %_target_platform%
echo    _output_directory   = %_output_directory%
echo    _target_config      = %_target_config%
echo    _target_path        = %_target_path%
echo    _target_dir         = %_target_dir%
echo    _log_file           = %_log_file%.log
if not exist %_output_directory% mkdir %_output_directory%
goto :eof

rem -----------------------------------------------------------------------------
rem 	Delete files (sub routine)
rem -----------------------------------------------------------------------------
:_delete_files
rmdir /S /Q %_target_dir%
del /Q %_log_file%.log
goto :eof