cmake -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_CXX_COMPILER=Clang -DCMAKE_C_COMPILER=Clang -DBUILD_TESTS=ON -DFOUR_USE_GLFW=ON
copy .\build\compile_commands.json .\compile_commands.json
cmake --build build --config Debug
.\build\four-engine\test\window-test.exe
.\build\four-engine\test\application-test.exe

