rm -rf ./i386
mkdir ./i386
rm -rf ./x86_64
mkdir ./x86_64
rm -rf ./universal
mkdir ./universal
make ENABLE64BIT=No USE_ASM=Yes BUILDTYPE=Release
mv *.a ./i386/
make clean
make ENABLE64BIT=Yes USE_ASM=Yes BUILDTYPE=Release
mv *.a ./x86_64/
make clean
echo "Build library - libssl.a"
lipo -create ./i386/libprocessing.a ./x86_64/libprocessing.a -output ./universal/libprocessing.a
lipo -create ./i386/libcommon.a ./x86_64/libcommon.a -output ./universal/libcommon.a
lipo -create ./i386/libencoder.a ./x86_64/libencoder.a -output ./universal/libencoder.a
lipo -create ./i386/libdecoder.a ./x86_64/libdecoder.a -output ./universal/libdecoder.a
lipo -info ./universal/libprocessing.a
lipo -info ./universal/libcommon.a
lipo -info ./universal/libdecoder.a
lipo -info ./universal/libencoder.a
cp codec/api/svc/* ../ShowKit/openH264/osx/
mv ./universal/*.a ../ShowKit/openH264/
