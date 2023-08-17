if exist dist.* goto lrdyex
nuke _dist_.tmp
mkdir _dist_.tmp
makefile "copy $@ _dist_.tmp$_" < mkdist.txt > _mkdist0.bat
call _mkdist0.bat
del  _mkdist0.bat
del tldist.zip
del tldist.exe
pkzip tldist _dist_.tmp\*.*
zip2exe tldist
del tldist.zip
nuke _dist_.tmp
goto exit
:lrdyex
echo Dist already exists.  Nuke it first.
:exit
