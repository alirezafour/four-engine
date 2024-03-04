if (-not (Test-Path -Path "shaders" -PathType Container)) {
  New-Item -Path "shaders" -ItemType "directory"
}
& "${env:VULKAN_SDK}\Bin\glslc.exe" four-engine\src\renderer\shaders\simpleShader.vert -o shaders\simpleShader.vert.spv
& "${env:VULKAN_SDK}\Bin\glslc.exe" four-engine\src\renderer\shaders\simpleShader.frag -o shaders\simpleShader.frag.spv
