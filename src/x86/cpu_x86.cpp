/* cpu_x86.cpp
 * 
 * Author           : Alexander J. Yee
 * Date Created     : 04/12/2014
 * Last Modified    : 04/12/2014
 * 
 */

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Dependencies
#include <cstring>
#include <iostream>
#include "cpu_x86.h"
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
#   if _WIN32
#       include "cpu_x86_Windows.ipp"
#   elif defined(__GNUC__) || defined(__clang__)
#       include "cpu_x86_Linux.ipp"
#   else
#       error "No cpuid intrinsic defined for compiler."
#   endif
#else
#   error "No cpuid intrinsic defined for processor architecture."
#endif

namespace FeatureDetector{
    using std::cout;
    using std::endl;
    using std::memcpy;
    using std::memset;
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
cpu_x86::cpu_x86(){
    memset(this, 0, sizeof(*this));
}
bool cpu_x86::detect_OS_AVX(){
    //  Copied from: http://stackoverflow.com/a/22521619/922184

    bool avxSupported = false;

    int cpuInfo[4];
    cpuid(cpuInfo, 1);

    bool osUsesXSAVE_XRSTORE = (cpuInfo[2] & (1 << 27)) != 0;
    bool cpuAVXSuport = (cpuInfo[2] & (1 << 28)) != 0;

    if (osUsesXSAVE_XRSTORE && cpuAVXSuport)
    {
        uint64_t xcrFeatureMask = xgetbv(_XCR_XFEATURE_ENABLED_MASK);
        avxSupported = (xcrFeatureMask & 0x6) == 0x6;
    }

    return avxSupported;
}
bool cpu_x86::detect_OS_AVX512(){
    if (!detect_OS_AVX())
        return false;

    uint64_t xcrFeatureMask = xgetbv(_XCR_XFEATURE_ENABLED_MASK);
    return (xcrFeatureMask & 0xe6) == 0xe6;
}
std::string cpu_x86::get_vendor_string(){
    int32_t CPUInfo[4];
    char name[13];

    cpuid(CPUInfo, 0);
    memcpy(name + 0, &CPUInfo[1], 4);
    memcpy(name + 4, &CPUInfo[3], 4);
    memcpy(name + 8, &CPUInfo[2], 4);
    name[12] = '\0';

    return name;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void cpu_x86::detect_host(){
    //  OS Features
    OS_x64 = detect_OS_x64();
    OS_AVX = detect_OS_AVX();
    OS_AVX512 = detect_OS_AVX512();

    //  Vendor
    std::string vendor(get_vendor_string());
    if (vendor == "GenuineIntel"){
        Vendor_Intel = true;
    }else if (vendor == "AuthenticAMD"){
        Vendor_AMD = true;
    }

    int info[4];
    cpuid(info, 0);
    int nIds = info[0];

    cpuid(info, 0x80000000);
    uint32_t nExIds = info[0];

    //  Detect Features
    if (nIds >= 0x00000001){
        cpuid(info, 0x00000001);
        HW_MMX    = (info[3] & ((int)1 << 23)) != 0;
        HW_SSE    = (info[3] & ((int)1 << 25)) != 0;
        HW_SSE2   = (info[3] & ((int)1 << 26)) != 0;
        HW_SSE3   = (info[2] & ((int)1 <<  0)) != 0;

        HW_SSSE3  = (info[2] & ((int)1 <<  9)) != 0;
        HW_SSE41  = (info[2] & ((int)1 << 19)) != 0;
        HW_SSE42  = (info[2] & ((int)1 << 20)) != 0;
        HW_AES    = (info[2] & ((int)1 << 25)) != 0;

        HW_AVX    = (info[2] & ((int)1 << 28)) != 0;
        HW_FMA3   = (info[2] & ((int)1 << 12)) != 0;

        HW_RDRAND = (info[2] & ((int)1 << 30)) != 0;
    }
    if (nIds >= 0x00000007){
        cpuid(info, 0x00000007);
        HW_AVX2         = (info[1] & ((int)1 <<  5)) != 0;

        HW_BMI1         = (info[1] & ((int)1 <<  3)) != 0;
        HW_BMI2         = (info[1] & ((int)1 <<  8)) != 0;
        HW_ADX          = (info[1] & ((int)1 << 19)) != 0;
        HW_MPX          = (info[1] & ((int)1 << 14)) != 0;
        HW_SHA          = (info[1] & ((int)1 << 29)) != 0;
        HW_PREFETCHWT1  = (info[2] & ((int)1 <<  0)) != 0;

        HW_AVX512_F     = (info[1] & ((int)1 << 16)) != 0;
        HW_AVX512_CD    = (info[1] & ((int)1 << 28)) != 0;
        HW_AVX512_PF    = (info[1] & ((int)1 << 26)) != 0;
        HW_AVX512_ER    = (info[1] & ((int)1 << 27)) != 0;
        HW_AVX512_VL    = (info[1] & ((int)1 << 31)) != 0;
        HW_AVX512_BW    = (info[1] & ((int)1 << 30)) != 0;
        HW_AVX512_DQ    = (info[1] & ((int)1 << 17)) != 0;
        HW_AVX512_IFMA  = (info[1] & ((int)1 << 21)) != 0;
        HW_AVX512_VBMI  = (info[2] & ((int)1 <<  1)) != 0;
    }
    if (nExIds >= 0x80000001){
        cpuid(info, 0x80000001);
        HW_x64   = (info[3] & ((int)1 << 29)) != 0;
        HW_ABM   = (info[2] & ((int)1 <<  5)) != 0;
        HW_SSE4a = (info[2] & ((int)1 <<  6)) != 0;
        HW_FMA4  = (info[2] & ((int)1 << 16)) != 0;
        HW_XOP   = (info[2] & ((int)1 << 11)) != 0;
    }
}
void cpu_x86::print() const{
	if (HW_MMX) { cout << "MMX" <<endl; }
	if (HW_x64 && OS_x64) { cout << "x64" <<endl; }
	if (HW_ABM) { cout << "ABM" <<endl; }
	if (HW_RDRAND) { cout << "RDRAND" <<endl; }
	if (HW_BMI1) { cout << "BMI1" <<endl; }
	if (HW_BMI2) { cout << "BMI2" <<endl; }
	if (HW_ADX) { cout << "ADX" <<endl; }
	if (HW_MPX) { cout << "MPX" <<endl; }
	if (HW_PREFETCHWT1) { cout << "PREFETCHWT1" <<endl; }
	if (HW_SSE) { cout << "SSE" <<endl; }
	if (HW_SSE2) { cout << "SSE2" <<endl; }
	if (HW_SSE3) { cout << "SSE3" <<endl; }
	if (HW_SSSE3) { cout << "SSSE3" <<endl; }
	if (HW_SSE4a) { cout << "SSE4a" <<endl; }
	if (HW_SSE41) { cout << "SSE4.1" <<endl; }
	if (HW_SSE42) { cout << "SSE4.2" <<endl; }
	if (HW_AES) { cout << "AES-NI" <<endl; }
	if (HW_SHA) { cout << "SHA" <<endl; }

	
	if (OS_AVX && HW_AVX) { cout << "AVX" <<endl; }
	if (HW_XOP) { cout << "XOP" <<endl; }
	if (HW_FMA3) { cout << "FMA3" <<endl; }
	if (HW_FMA4) { cout << "FMA4" <<endl; }
	if (HW_AVX2) { cout << "AVX2" <<endl; }
	if (OS_AVX512 && HW_AVX512_F) { cout << "AVX512-F" <<endl; }
	if (OS_AVX512 && HW_AVX512_CD) { cout << "AVX512-CD" <<endl; }
	if (OS_AVX512 && HW_AVX512_PF) { cout << "AVX512-PF" <<endl; }
	if (OS_AVX512 && HW_AVX512_ER) { cout << "AVX512-ER" <<endl; }
	if (OS_AVX512 && HW_AVX512_VL) { cout << "AVX512-VL" <<endl; }
	if (OS_AVX512 && HW_AVX512_BW) { cout << "AVX512-BW" <<endl; }
	if (OS_AVX512 && HW_AVX512_DQ) { cout << "AVX512-DQ" <<endl; }
	if (OS_AVX512 && HW_AVX512_IFMA) { cout << "AVX512-IFMA" <<endl; }
	if (OS_AVX512 && HW_AVX512_VBMI) { cout << "AVX512-VBMI" <<endl; }
}
void cpu_x86::print_host(){
    cpu_x86 features;
    features.detect_host();
    features.print();
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
}
