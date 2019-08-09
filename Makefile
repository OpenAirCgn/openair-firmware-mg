PORT=--port /dev/cu.usbserial-DM01NFWV

build/fw.zip: src/*
	mos build --platform esp32 --local --no-libs-update --verbose

flash: build/fw.zip
	mos flash $(PORT)

console:
	mos console $(PORT)
