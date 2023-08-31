cmake -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_CXX_COMPILER=Clang
copy .\build\compile_commands.json .\compile_commands.json
cmake --build build
