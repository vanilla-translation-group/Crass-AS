#!/usr/bin/env bash

if [ -n "$1" ]; then
	glspath="$1"
else
	glsver="4.07.03"
	wget https://www.entis.jp/gls/bin/EntisGLS$i.zip -o EntisGLS.zip
	unzip EntisGLS.zip "EntisGLS$i\\\\GLS3\\\\*" "EntisGLS$i\\\\Cotopha\\\\Include\\\\*"
	glspath="EntisGLS$i"
fi
glspath="`realpath "$glspath"`"

cd "`dirname "$0"`"
mkdir -p include

cd include
cp "$glspath/GLS3/erisalib/Include/"* .
cp "$glspath/GLS3/ESL/Include/"* .
cp -r "$glspath/Cotopha/Include/common/esl" .
cp -r "$glspath/Cotopha/Include/common/sakura" .
cp "$glspath/GLS3/EGL/Include/"* .
cp "$glspath/GLS3/EGL/common/"* .
cp -r "$glspath/Cotopha/Include/win32/esl" .
cp -r "$glspath/Cotopha/Include/common/sakuragl" .
cp -r "$glspath/Cotopha/Include/common/sakuracl" .
cp "$glspath/GLS3/GLS3/Include/"* .
cd ..

for i in `find include/ -type f`; do
	iconv -f CP932 -t UTF-8 $i > $i.tmp && mv $i.tmp $i
done

sed -i.bak "s/LoadLibrary/LoadLibraryA/g" `grep -rl LoadLibrary include/`
