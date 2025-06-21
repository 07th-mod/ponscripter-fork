set -e

echo -n "$TRAVIS_OS_NAME$STEAM$NODEP" > buildinfo.txt

if [ -z "$STEAMLESS" ] && [ -z "$SSH_KEY" ]; then
	echo "Can't get steam API without SSH key, skipping"
	echo "::warning file=.travis_run.sh::Can't download Steam SDK without SSH key so build skipped"
	exit 0
fi

if [ "$TRAVIS_OS_NAME" == "osx" ]; then
	./configure --unsupported-compiler --with-internal-libs --no-docs $STEAM
	LDFLAGS="-dead_strip" make
	if [[ "$(gcc --version)" =~ "Apple clang version "([0-9]+) ]] && [[ "${BASH_REMATCH[1]}" -lt 10 ]]; then
		mv src/ponscr ponscr64
		make distclean
		# Disable making docs on MacOS to prevent error - we don't output docs anyway on Github Actions
		CC="clang -arch i386" CXX="clang++ -arch i386" ./configure --unsupported-compiler --with-internal-libs --no-docs $STEAM
		CFLAGS="-arch i386" LDFLAGS="-dead_strip -arch i386" make MACOSX_DEPLOYMENT_TARGET=10.5
		mv src/ponscr ponscr32
		lipo -create -output src/ponscr ponscr32 ponscr64
	fi
elif [ "$TRAVIS_OS_NAME" == "linux" ]; then
	LIBFOLDER="lib64"
	if [ -n "$STEAM" ]; then
		# Freetype header search is slightly broken, "fix" it by soft linking it to the expected place
		sudo ln -s /usr/include/freetype2 /usr/include/freetype2/freetype
		# We ship steam build to people with GOG copies, so build with internal sdlimage (see below for details)
		run.sh ./configure --with-external-sdl-mixer $STEAM --internal-all-images
		LDFLAGS="-Wl,-rpath,XORIGIN/$LIBFOLDER:." run.sh make
	else
		mkdir src/extlib/include src/extlib/lib
		if [ "$TRAVIS_BRANCH" == "wh-mod" ] || [ -n "$NODEP" ]; then
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
	$mingw32 make -j2
fi

cd src
if [ "$TRAVIS_OS_NAME" == "windows" ]; then
	7z a ../ponscr.zip ponscr.exe
else
	zip -9 ../ponscr.zip ponscr
fi
cd ..
