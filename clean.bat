@echo off
del freewill.sdf
rmdir /S /Q Debug
rmdir /S /Q Release
cd freewill
rmdir /S /Q Debug
rmdir /S /Q Release
del *_i.c
del *_p.c
del dlldata.c
del bodyplus.h
del boundplus.h
del fileplus.h
del kineplus.h
del matplus.h
del meshplus.h
del rndrplus.h
del sceneplus.h
del transplus.h
del common.h
del freewill.h
del freewill.vcproj.*.user
cd ..
cd fwrender
rmdir /S /Q Debug
rmdir /S /Q Release
del *_i.c
del *_p.c
del dlldata.c
del fwrender.h
del fwrender.vcproj.*.user
cd ..
cd fwaction
rmdir /S /Q Debug
rmdir /S /Q Release
del *_i.c
del *_p.c
del dlldata.c
del actionplus.h
del fwaction.h
del fwaction.vcproj.*.user
cd ..
cd fwview
rmdir /S /Q Debug
rmdir /S /Q Release
del fwview.vcproj.*.user
cd ..
cd fwlib
rmdir /S /Q Debug
rmdir /S /Q Release
del fwlib.vcproj.*.user
cd ..
cd FWStarter
rmdir /S /Q Debug
rmdir /S /Q Release
del FWStarter.vcproj.*.user
cd ..
echo Done...
