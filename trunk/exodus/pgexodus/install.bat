call config.bat
net stop "%POSTGRES_SERVICE%"
net start "%POSTGRES_SERVICE%"
copy %RELEASE_OR_DEBUG%\pgexodus.dll "%POSTGRES_PATH%\lib"
