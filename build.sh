#mos build --platform esp32 --local --no-libs-update && mos flash
mos build --local --verbose --deps-dir=../mongoose_quatsch/deps --binary-libs-dir ../mongoose_quatsch/bins --no-libs-update --platform esp32
