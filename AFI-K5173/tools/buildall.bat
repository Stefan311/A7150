@path=%path%;c:\tp7\bin\;c:\as80\;
@cd bin2inc
tpc bin2inc.pas
@if not exist bin2inc.exe goto error
@cd ..
@cd crctool
tpc crctool.pas
@if not exist crctool.exe goto error
@cd ..
@path=%path%;c:\bin2inc\;c:\crctool\;
cd kesdump
@call build.bat
@if not exist kesdump.exe goto error
@cd ..
cd afitest
@call build.bat
@if not exist afitest.exe goto error
@cd ..
cd afiflash
call build.bat
@if not exist afiflash.exe goto error
@cd ..
@echo "Fertig"
@pause
@exit 0

:error
@echo "Fehler!"
@pause
@exit 1
