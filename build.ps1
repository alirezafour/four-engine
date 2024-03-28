cmake -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_CXX_COMPILER=Clang -DCMAKE_C_COMPILER=Clang -DBUILD_TESTS=ON -DFOUR_USE_GLFW=ON
copy .\build\compile_commands.json .\compile_commands.json
cmake --build build --config Debug --parallel

$testPath = ".\build\four-engine\test\window-test.exe"
if(Test-Path $testPath) {
  .\build\four-engine\test\window-test.exe
}
else
{
  Write-Host "window-test.exe not found"
}

$testPath = ".\build\four-engine\test\application-test.exe"
if(Test-Path $testPath) {
  .\build\four-engine\test\application-test.exe
}
else
{
  Write-Host "application-test.exe not found"
}
