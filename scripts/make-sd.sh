# 09:32 15/03/2023 - change triggering comment
#!/bin/bash
set -e

if [ "$#" -ne 1 ]; then
echo Please specify one argument: "basic" or "discovery"
exit 1
fi

if [[ $OSTYPE == 'darwin'* ]]; then
if ! command -v brew &> /dev/null; then
echo No homebrew detected. It is required to build an image Press Enter to install it, or press Control-C to abort.
read -s -n 1
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
fi

if ! command -v mformat &> /dev/null; then
brew install mtools
fi
else
if ! command -v mformat &> /dev/null; then
echo No mtools detected. Please install it iin the usual way for your operating system.
exit 1
fi
fi

dd if=/dev/zero of=disk.img bs=1M count=33
mformat -F -i disk.img ::
mcopy -i disk.img nextion-$1-lcd.tft ::

echo
echo Created disk.img. Please flash this file to your SD card with a tool such as balenaEtcher.
