#include <arch/cpuid.h>
#include <arch/simd.h>
#include <stdio.h>

extern "C" void simd_enable_sse();

bool simd::enableSSE() {
    if (cpuid::has_feature_EDX(cpuid::cpufeaturesEDX::SSE)) {
        simd_enable_sse();
        return true;
    }
    return false;
}
