(* ::Package:: *)

BeginPackage["WolframPackageTest`"]

(* 1. Declare the public functions *)
WAdd::usage = "WAdd[a, b] adds two integers.";
WSub::usage = "WSub[a, b] subtracts two integers.";
WMul::usage = "WMul[a, b] multiplies two integers.";
WDiv::usage = "WDiv[a, b] divides two integers.";

Begin["`Private`"]

(* 2. Find and track the library path automatically *)
dllPath = FindLibrary["wolfram_package_test"];
(*dllPath = FileNameJoin[{
    PacletObject["WolframPackageTest"]["Location"],
    "LibraryResources",
    $SystemID,
    "wolfram_package_test"   (* no extension — LibraryFunctionLoad appends .dll/.so/.dylib *)
}];*)

(* 3. Map the C++ functions directly to the public names *)
WAdd = LibraryFunctionLoad[dllPath, "Wolfram_add", {Integer, Integer}, Integer];
WSub = LibraryFunctionLoad[dllPath, "Wolfram_sub", {Integer, Integer}, Integer];
WMul = LibraryFunctionLoad[dllPath, "Wolfram_mul", {Integer, Integer}, Integer];
WDiv = LibraryFunctionLoad[dllPath, "Wolfram_div", {Integer, Integer}, Integer];

End[]
EndPackage[]