# Install DFU Util
## Linux
### Ubuntu
`sudo apt install dfu-util`
### RPM based and other distros
Use you package manager or download a package from OBS: https://build.opensuse.org/package/show/hardware/dfu-util
## Mac OS
`brew install dfu-util`
## Windows
Install correct drivers using zadig: https://zadig.akeo.ie/
Download and unpack: http://dfu-util.sourceforge.net/releases/dfu-util-0.11-binaries.tar.xz

# Flash firmware
## Put the STM32 into DFU mode
1. Press and hold both NRST and BOOT0 buttons
2. Release NRST button
3. Wait a second
4. Release BOOT0 button

## Run the flasher

### Linux and Mac OS
`dfu-util -d 0x0483:0xDF11 -a 0 -s 0x08000000:leave -D firmware.bin`

### Linux 
`dfu-util-0.11-binaries/win64/dfu-util-static.exe -d 0x0483:0xDF11 -a 0 -s 0x08000000:leave -D firmware.bin`
