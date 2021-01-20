set -e

if [ -z "$STEAMLESS" ] && [ -z "$SSH_KEY" ]; then
	echo "Can't get steam API without SSH key, skipping"
	exit 0
fi

if [ "$TRAVIS_OS_NAME" == "osx" ]; then
	./configure --unsupported-compiler --with-internal-libs $STEAM
	make
	if [[ "$(gcc --version)" =~ "Apple clang version "([0-9]+) ]] && [[ "${BASH_REMATCH[1]}" -lt 10 ]]; then
		mv src/ponscr ponscr64
		make distclean
		CC="clang -arch i386" CXX="clang++ -arch i386" ./configure --unsupported-compiler --with-internal-libs $STEAM
		CFLAGS="-arch i386" LDFLAGS="-arch i386" make MACOSX_DEPLOYMENT_TARGET=10.5
		mv src/ponscr ponscr32
		lipo -create -output src/ponscr ponscr32 ponscr64
	fi
elif [ "$TRAVIS_OS_NAME" == "linux" ]; then
	LIBFOLDER="lib64"
	if [ -n "$STEAM" ]; then
		# Freetype header search is slightly broken, "fix" it by soft linking it to the expected place
		sudo ln -s /usr/include/freetype2 /usr/include/freetype2/freetype
		run.sh ./configure --with-external-sdl-mixer $STEAM
		LDFLAGS="-Wl,-rpath,XORIGIN/$LIBFOLDER:." run.sh make
	else
		mkdir src/extlib/include src/extlib/lib
		if [ "$TRAVIS_BRANCH" == "wh-mod" ]; then
			# Ciconia comes with a lib64 directory full of broken stuff
			# We'll try to build a binary with as few dependencies as possible but internal sdl is broken on linux
			# Because of that, reference a new lib64 directory, leaving us the option of shipping a working libsdl later
			./configure --with-internal-sdl-image --with-internal-sdl-mixer --with-internal-smpeg --with-internal-freetype --with-internal-bzip
			LIBFOLDER="lib64real"
		else
			# Program seems to break (testing with Ubuntu 18.04) with internal SDL, so using external SDL
			# On the other hand, GOG copy is missing libpng12 so on systems with a newer libpng the game doesn't work
			# Fix that by using internal sdlimage
			./configure --with-internal-sdl-image
		fi
		LDFLAGS="-Wl,-rpath,XORIGIN/$LIBFOLDER:." make
	fi
	chrpath -r "\$ORIGIN/$LIBFOLDER:." src/ponscr
else
	# Windows build
	$mingw32 ./configure $STEAM
	$mingw32 make
fi

cd src
if [ "$TRAVIS_OS_NAME" == "windows" ]; then
	zip -9 ../ponscr.zip ponscr.exe
else
	zip -9 ../ponscr.zip ponscr
fi
cd ..

echo -n "$TRAVIS_OS_NAME$STEAM" > buildinfo.txt
