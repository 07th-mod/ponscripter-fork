set -e

if [ -z "$STEAMLESS" ] && [ -z "$DOWNLOAD_SERVER" ]; then
	echo "Can't get steam API without download server key, skipping"
	echo "::warning file=.travis_before_install.sh::Can't download Steam SDK without download server so build skipped"
	exit 0
fi

case "$TRAVIS_OS_NAME" in
linux)
	sudo apt-get update -qq
	sudo apt-get install -y libasound2-dev ;;
windows)
	choco install -y unzip ;;
esac

if [ -z "$STEAMLESS" ] && [ -n "$DOWNLOAD_SERVER" ]; then
	export STEAM="-steam"
	if [ "$TRAVIS_OS_NAME" == "linux" ]; then
		export STEAM_RUNTIME_HOST_ARCH=$(dpkg --print-architecture)
		export STEAM_RUNTIME_ROOT="$(pwd)/steam/runtime/amd64"
		export STEAM_RUNTIME_TARGET_ARCH=amd64
		export PATH="$(pwd)/steam/bin:$PATH"
	fi

	# Steam SDK
	# Umineko is great and ships a different version of the steam api dll for each os
	case "$TRAVIS_OS_NAME" in
	osx)     STEAMVER=129a;;
	windows) STEAMVER=134;;
	linux)   STEAMVER=141;;
	esac
	curl -O "${DOWNLOAD_SERVER}/steamworks_sdk_${STEAMVER}.zip" 2>/dev/null
	unzip -d src/extlib/src/steam-sdk/ "steamworks_sdk_${STEAMVER}.zip"
	mv src/extlib/src/steam-sdk/sdk/* src/extlib/src/steam-sdk/
fi

if [ "$TRAVIS_OS_NAME" == "linux" ]; then
	sudo apt-get install -y chrpath libbz2-dev libvorbis-dev libfreetype6-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-dev libgles2-mesa-dev libglu1-mesa-dev
	if [ -z "$STEAMLESS" ] && [ -n "$DOWNLOAD_SERVER" ]; then
		# For i386 steam builds, we build x86_64
		# sudo dpkg --add-architecture i386
		# sudo apt-get update -qq
		# sudo apt-get install -y libbz2-dev:i386 libfreetype6-dev:i386 libsdl2-mixer-dev:i386 libsdl2-image-dev:i386 libsdl2-dev:i386 libpulse-dev:i386 libavahi-client-dev:i386 libgles2-mesa-dev:i386 libglu1-mesa-dev:i386 libgl1-mesa-dev:i386 libpng-dev:i386

		# Steam build environment
		mkdir steam
		for file in "http://media.steampowered.com/client/runtime/steam-runtime-sdk_latest.tar.xz" "http://media.steampowered.com/client/runtime/x-tools-amd64-i386_latest.tar.xz" "http://media.steampowered.com/client/runtime/x-tools-amd64-amd64_latest.tar.xz"; do
			echo "Downloading $file"
			curl "$file" | tar -xJf - --strip-components=1 -C steam
		done
		mkdir steam/runtime
		echo "Downloading http://media.steampowered.com/client/runtime/steam-runtime-dev-release_latest.tar.xz"
		curl "http://media.steampowered.com/client/runtime/steam-runtime-dev-release_latest.tar.xz" | tar -xJf - --strip-components=1 -C steam/runtime
	fi
elif [ "$TRAVIS_OS_NAME" == "windows" ]; then
	[[ ! -f C:/tools/msys64/msys2_shell.cmd ]] && rm -rf C:/tools/msys64
	choco upgrade --no-progress -y msys2
	export msys2='cmd //C RefreshEnv.cmd '
	export msys2+='& set MSYS=winsymlinks:nativestrict '
	export msys2+='& C:\\tools\\msys64\\msys2_shell.cmd -defterm -no-start'
	export mingw64="$msys2 -mingw64 -full-path -here -c "\"\$@"\" --"
	export mingw32="$msys2 -mingw32 -full-path -here -c "\"\$@"\" --"
	export msys2+=" -msys2 -c "\"\$@"\" --"
	$msys2 pacman --sync --noconfirm --needed mingw-w64-i686-gcc make autoconf automake-wrapper
	## Install more MSYS2 packages from https://packages.msys2.org/base here
	taskkill //IM gpg-agent.exe //F || true # https://travis-ci.community/t/4967
fi
