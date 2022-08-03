#include "cpuid.h"
#include <cpuid.h>
#include "stdio.h"

void cpuid::get_vendorstring(char* vendorstring)
{
    uint32_t* vendorstr = (uint32_t*)vendorstring;
    int unused;
    __cpuid(0, unused, vendorstr[0], vendorstr[2], vendorstr[1]);
}

bool cpuid::has_feature_EDX(cpufeaturesEDX feature) {
    int unused;
    uint32_t features;
    __cpuid(1, unused, unused, unused, features);
    return features & static_cast<uint32_t>(feature);
}

bool cpuid::has_feature_ECX(cpufeaturesECX feature) {
    int unused;
    uint32_t features;
    __cpuid(1, unused, unused, features, unused);
    return features & static_cast<uint32_t>(feature);
}

 
void cpuid::printFeatures() {
    char vendor[13];
    vendor[12] = '\0';
    get_vendorstring(vendor);
    printf("vendor: %s\n", vendor);


    printf("checking features(EDX)\n");
    char EDXfeatures[32][8] = {"FPU", "VME", "DE", "PSE", "TSC", "MSR", "PAE", "MCE", "CX8", "APIC", "???", "SEP", "MTRR", "PGE", "MCA", "CMOV", "PAT", "PSE36", "PSN", "CLFLUSH", "???", "DS", "ACPI", "MMX", "FXSR", "SSE", "SSE2", "SS", "HTT", "TM", "IA64", "PBE"};
    int unused, featuresEDX;
    __cpuid(1, unused, unused, unused, featuresEDX);
    for(int i = 0; i < 32; i++) {
        if(featuresEDX & (1 << i)) {
            printf("CPU has %s\n", EDXfeatures[i]);
        }
    }
}