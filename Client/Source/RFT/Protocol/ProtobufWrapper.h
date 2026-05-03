// ProtobufWrapper.h
#pragma once

// ── 1. 무조건 push (정의 여부 무관) ──────────────────────
#pragma push_macro("check")
#pragma push_macro("verify")
#pragma push_macro("ensure")
#pragma push_macro("TEXT")
#pragma push_macro("PI")
#pragma push_macro("DWORD")
#pragma push_macro("MIN")
#pragma push_macro("MAX")

// ── 2. 무조건 undef ──────────────────────────────────────
#undef check
#undef verify
#undef ensure
#undef TEXT
#undef PI
#undef DWORD
#undef MIN
#undef MAX

#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN

THIRD_PARTY_INCLUDES_START
#pragma warning(push)
#pragma warning(disable: 4005 4125 4127 4200 4201 4251 4800 4996)

#include "Protocol.pb.h"

#pragma warning(pop)
THIRD_PARTY_INCLUDES_END

// ── 3. 무조건 pop (반드시 복원됨) ────────────────────────
#pragma pop_macro("MAX")
#pragma pop_macro("MIN")
#pragma pop_macro("DWORD")
#pragma pop_macro("PI")
#pragma pop_macro("TEXT")
#pragma pop_macro("ensure")
#pragma pop_macro("verify")
#pragma pop_macro("check")