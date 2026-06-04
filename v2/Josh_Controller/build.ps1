$vcvars = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
$wolframInclude = "C:\Program Files\Wolfram Research\Wolfram\14.2\SystemFiles\IncludeFiles\C"
$localInclude = "C:\Projects\MachineControl\v2\include"
$clearviewInclude = "C:\Program Files (x86)\Teknic\ClearView\sdk\inc"
$labjackInclude = "C:\Program Files (x86)\LabJack\Drivers"
$outDll = "C:\Projects\MachineControl\v2\Josh_Controller\build\bin\LabController.dll"

cmd /c " `"$vcvars`" x64 && cl.exe /std:c++17 /Zi /EHsc /nologo /LD /I`"$wolframInclude`" /I`"$localInclude`" /I`"$clearviewInclude`" /I`"$labjackInclude`" /Fe`"$outDll`" C:\Projects\MachineControl\v2\src\API.cpp "
