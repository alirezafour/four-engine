cmake -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang
copy .\build\compile_commands.json .\compile_commands.json
cmake --build build
.\build\four-engine\test\window-test.exe
.\build\four-engine\test\application-test.exe

