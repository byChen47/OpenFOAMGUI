#!/bin/sh
PATH=/D/3.Wpsandother/Qt/setting/6.10.2/mingw_64/bin:$PATH
export PATH
QT_PLUGIN_PATH=/D/3.Wpsandother/Qt/setting/6.10.2/mingw_64/plugins${QT_PLUGIN_PATH:+:$QT_PLUGIN_PATH}
export QT_PLUGIN_PATH
exec "$@"
