using UnrealBuildTool;
using System.IO;
using System; // Exception 처리를 위해 추가

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

        if (Directory.Exists(Path.Combine(VcpkgPath, "include")))
        {
            PublicIncludePaths.Add(Path.Combine(VcpkgPath, "include"));
        }

        // ── [안전한 경로 탐색] Target.ProjectFile 대신 ModuleDirectory 활용 ──
        string TargetBinariesDir = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../Binaries/Win64"));

        if (!Directory.Exists(TargetBinariesDir))
        {
            Directory.CreateDirectory(TargetBinariesDir);
        }

        // ── Protobuf ──────────────────────────────────────────
        string ProtobufLib = Path.Combine(VcpkgLibPath, "libprotobuf.lib");
        if (File.Exists(ProtobufLib)) PublicAdditionalLibraries.Add(ProtobufLib);

        string ProtobufSrcDll = Path.Combine(VcpkgBinPath, "libprotobuf.dll");
        string ProtobufDstDll = Path.Combine(TargetBinariesDir, "libprotobuf.dll");
        if (File.Exists(ProtobufSrcDll))
        {
            try { File.Copy(ProtobufSrcDll, ProtobufDstDll, true); }
            catch (Exception) { /* 에디터가 켜져 있어서 덮어쓰기 실패해도 무시 (이미 파일이 있으므로 괜찮음) */ }

            RuntimeDependencies.Add(Path.Combine("$(ProjectDir)", "Binaries", "Win64", "libprotobuf.dll"));
        }

        // ── Abseil: import lib 연결 + DLL 복사 ───────────────
        if (Directory.Exists(VcpkgLibPath))
        {
            foreach (string AbslLib in Directory.GetFiles(VcpkgLibPath, "absl_*.lib"))
            {
                PublicAdditionalLibraries.Add(AbslLib);
            }
            foreach (string AbseilLib in Directory.GetFiles(VcpkgLibPath, "abseil_*.lib"))
            {
                PublicAdditionalLibraries.Add(AbseilLib);
            }
        }

        if (Directory.Exists(VcpkgBinPath))
        {
            string[] AbseilDlls = Directory.GetFiles(VcpkgBinPath, "abseil_*.dll");
            foreach (string SrcDll in AbseilDlls)
            {
                string FileName = Path.GetFileName(SrcDll);
                string DstDll = Path.Combine(TargetBinariesDir, FileName);

                try { File.Copy(SrcDll, DstDll, true); }
                catch (Exception) { /* 에디터 Lock 무시 */ }

                RuntimeDependencies.Add(Path.Combine("$(ProjectDir)", "Binaries", "Win64", FileName));
            }
        }

        // ── utf8_range: protobuf 4.x 이상 의존성 ─────────────
        string Utf8RangeLib = Path.Combine(VcpkgLibPath, "utf8_range.lib");
        string Utf8ValidityLib = Path.Combine(VcpkgLibPath, "utf8_validity.lib");
        if (File.Exists(Utf8RangeLib)) PublicAdditionalLibraries.Add(Utf8RangeLib);
        if (File.Exists(Utf8ValidityLib)) PublicAdditionalLibraries.Add(Utf8ValidityLib);

        // ── 전처리기 ──────────────────────────────────────────
        PublicDefinitions.Add("NOMINMAX");
        PublicDefinitions.Add("WIN32_LEAN_AND_MEAN");
        PublicDefinitions.Add("PROTOBUF_USE_DLLS");
        PublicDefinitions.Add("ABSL_CONSUME_DLL");
    }
}