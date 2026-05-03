using UnrealBuildTool;
using System.IO;

public class RFT : ModuleRules
{
    public RFT(ReadOnlyTargetRules Target) : base(Target)
    {
        CppStandard = CppStandardVersion.Cpp17;
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject", "Engine", "InputCore", "UMG",
            "HTTP", "Json", "JsonUtilities", "Networking", "Sockets"
        });
        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        bUseRTTI = true;
        bEnableExceptions = true;

        string ProtocolPath = Path.Combine(ModuleDirectory, "Protocol");
        PublicIncludePaths.Add(ProtocolPath);

        string VcpkgPath = @"C:\vcpkg\installed\x64-windows";
        string VcpkgLibPath = Path.Combine(VcpkgPath, "lib");
        string VcpkgBinPath = Path.Combine(VcpkgPath, "bin");

        PublicIncludePaths.Add(Path.Combine(VcpkgPath, "include")); //include파일 안에 absl,google파일이 존재

        // ── Protobuf ──────────────────────────────────────────
        PublicAdditionalLibraries.Add(Path.Combine(VcpkgLibPath, "libprotobuf.lib")); //있음
        RuntimeDependencies.Add(Path.Combine(VcpkgBinPath, "libprotobuf.dll")); //있음

        // ── Abseil: import lib 연결 + DLL 복사 ───────────────
        foreach (string LIB in Directory.GetFiles(VcpkgLibPath, "absl_*.lib")) //23개 있음
        {
            PublicAdditionalLibraries.Add(LIB);
        }
        foreach (string DLL in Directory.GetFiles(VcpkgBinPath, "abseil_*.dll")) //absl_*.dll은 존재x
        {
            RuntimeDependencies.Add(DLL);
        }

        // ── utf8_range: protobuf 4.x 이상 의존성 ─────────────
        // absl 말고도 utf8_range, utf8_validity가 필요한 경우 있음
        string Utf8RangeLib = Path.Combine(VcpkgLibPath, "utf8_range.lib");
        string Utf8ValidityLib = Path.Combine(VcpkgLibPath, "utf8_validity.lib");
        if (File.Exists(Utf8RangeLib)) PublicAdditionalLibraries.Add(Utf8RangeLib);
        if (File.Exists(Utf8ValidityLib)) PublicAdditionalLibraries.Add(Utf8ValidityLib);

        // ── 전처리기 ──────────────────────────────────────────
        PublicDefinitions.Add("NOMINMAX");
        PublicDefinitions.Add("WIN32_LEAN_AND_MEAN");
        PublicDefinitions.Add("PROTOBUF_USE_DLLS");
        
    }
}