# Emby PVR
Emby PVR client addon for [Kodi] (http://kodi.tv)

## Build instructions

### Linux

1. `git clone https://github.com/xbmc/xbmc.git kodi`
2. `git clone https://github.com/escabe/pvr.emby.git`
3. `cd pvr.emby && mkdir build && cd build`
4. `cmake -DADDONS_TO_BUILD=pvr.emby -DADDON_SRC_PREFIX=../.. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=../../kodi/kodi-build/addons -DPACKAGE_ZIP=1 ../../kodi/cmake/addons`
5. `make`

##### Useful links

* [Kodi's PVR user support] (http://forum.kodi.tv/forumdisplay.php?fid=167)
* [Kodi's PVR development support] (http://forum.kodi.tv/forumdisplay.php?fid=136)
