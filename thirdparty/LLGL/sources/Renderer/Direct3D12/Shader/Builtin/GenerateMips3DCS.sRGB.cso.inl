#if 0
//
// Generated by Microsoft (R) D3D Shader Disassembler
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// no Input
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// no Output
cs_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer CB0[2], immediateIndexed
dcl_sampler s0, mode_default
dcl_resource_texture3d (float,float,float,float) t0
dcl_uav_typed_texture3d (float,float,float,float) u0
dcl_uav_typed_texture3d (float,float,float,float) u1
dcl_uav_typed_texture3d (float,float,float,float) u2
dcl_input vThreadIDInGroupFlattened
dcl_input vThreadID.xyz
dcl_temps 9
dcl_tgsm_structured g0, 4, 64
dcl_tgsm_structured g1, 4, 64
dcl_tgsm_structured g2, 4, 64
dcl_tgsm_structured g3, 4, 64
dcl_thread_group 4, 4, 4
utof r0.xyz, vThreadID.xyzx
add r0.xyz, r0.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
mul r0.xyz, r0.xyzx, cb0[0].xyzx
utof r0.w, cb0[0].w
sample_l_indexable(texture3d)(float,float,float,float) r0.xyzw, r0.xyzx, t0.xyzw, s0, r0.w
lt r1.xyz, r0.xyzx, l(0.003131, 0.003131, 0.003131, 0.000000)
mul r2.xyz, r0.xyzx, l(12.920000, 12.920000, 12.920000, 0.000000)
add r3.xyz, r0.xyzx, l(-0.002280, -0.002280, -0.002280, 0.000000)
sqrt r3.xyz, |r3.xyzx|
mul r4.xyz, r0.xyzx, l(0.134480, 0.134480, 0.134480, 0.000000)
mad r3.xyz, r3.xyzx, l(1.130050, 1.130050, 1.130050, 0.000000), -r4.xyzx
add r3.xyz, r3.xyzx, l(0.005719, 0.005719, 0.005719, 0.000000)
movc r1.xyz, r1.xyzx, r2.xyzx, r3.xyzx
mov r1.w, r0.w
store_uav_typed u0.xyzw, vThreadID.xyzz, r1.xyzw
ieq r1.x, cb0[1].x, l(1)
if_nz r1.x
  ret 
endif 
store_structured g0.x, vThreadIDInGroupFlattened.x, l(0), r0.x
store_structured g1.x, vThreadIDInGroupFlattened.x, l(0), r0.y
store_structured g2.x, vThreadIDInGroupFlattened.x, l(0), r0.z
store_structured g3.x, vThreadIDInGroupFlattened.x, l(0), r1.w
sync_g_t
and r1.x, vThreadIDInGroupFlattened.x, l(21)
if_z r1.x
  iadd r1.xyzw, vThreadIDInGroupFlattened.xxxx, l(1, 4, 5, 16)
  ld_structured r2.x, r1.x, l(0), g0.xxxx
  ld_structured r2.y, r1.x, l(0), g1.xxxx
  ld_structured r2.z, r1.x, l(0), g2.xxxx
  ld_structured r2.w, r1.x, l(0), g3.xxxx
  ld_structured r3.x, r1.y, l(0), g0.xxxx
  ld_structured r3.y, r1.y, l(0), g1.xxxx
  ld_structured r3.z, r1.y, l(0), g2.xxxx
  ld_structured r3.w, r1.y, l(0), g3.xxxx
  ld_structured r4.x, r1.z, l(0), g0.xxxx
  ld_structured r4.y, r1.z, l(0), g1.xxxx
  ld_structured r4.z, r1.z, l(0), g2.xxxx
  ld_structured r4.w, r1.z, l(0), g3.xxxx
  ld_structured r5.x, r1.w, l(0), g0.xxxx
  ld_structured r5.y, r1.w, l(0), g1.xxxx
  ld_structured r5.z, r1.w, l(0), g2.xxxx
  ld_structured r5.w, r1.w, l(0), g3.xxxx
  iadd r1.xyz, vThreadIDInGroupFlattened.xxxx, l(17, 20, 21, 0)
  ld_structured r6.x, r1.x, l(0), g0.xxxx
  ld_structured r6.y, r1.x, l(0), g1.xxxx
  ld_structured r6.z, r1.x, l(0), g2.xxxx
  ld_structured r6.w, r1.x, l(0), g3.xxxx
  ld_structured r7.x, r1.y, l(0), g0.xxxx
  ld_structured r7.y, r1.y, l(0), g1.xxxx
  ld_structured r7.z, r1.y, l(0), g2.xxxx
  ld_structured r7.w, r1.y, l(0), g3.xxxx
  ld_structured r8.x, r1.z, l(0), g0.xxxx
  ld_structured r8.y, r1.z, l(0), g1.xxxx
  ld_structured r8.z, r1.z, l(0), g2.xxxx
  ld_structured r8.w, r1.z, l(0), g3.xxxx
  add r1.xyzw, r0.xyzw, r2.xyzw
  add r1.xyzw, r3.xyzw, r1.xyzw
  add r1.xyzw, r4.xyzw, r1.xyzw
  add r1.xyzw, r5.xyzw, r1.xyzw
  add r1.xyzw, r6.xyzw, r1.xyzw
  add r1.xyzw, r7.xyzw, r1.xyzw
  add r1.xyzw, r8.xyzw, r1.xyzw
  mul r0.xyzw, r1.xyzw, l(0.125000, 0.125000, 0.125000, 0.125000)
  ushr r2.xyzw, vThreadID.xyzz, l(1, 1, 1, 1)
  lt r3.xyz, r1.xyzx, l(0.025046, 0.025046, 0.025046, 0.000000)
  mul r4.xyz, r1.xyzx, l(1.615000, 1.615000, 1.615000, 0.000000)
  mad r5.xyz, r1.xyzx, l(0.125000, 0.125000, 0.125000, 0.000000), l(-0.002280, -0.002280, -0.002280, 0.000000)
  sqrt r5.xyz, |r5.xyzx|
  mul r1.xyz, r1.xyzx, l(0.016810, 0.016810, 0.016810, 0.000000)
  mad r1.xyz, r5.xyzx, l(1.130050, 1.130050, 1.130050, 0.000000), -r1.xyzx
  add r1.xyz, r1.xyzx, l(0.005719, 0.005719, 0.005719, 0.000000)
  movc r1.xyz, r3.xyzx, r4.xyzx, r1.xyzx
  mov r1.w, r0.w
  store_uav_typed u1.xyzw, r2.xyzw, r1.xyzw
endif 
ieq r1.x, cb0[1].x, l(2)
if_nz r1.x
  ret 
endif 
store_structured g0.x, vThreadIDInGroupFlattened.x, l(0), r0.x
store_structured g1.x, vThreadIDInGroupFlattened.x, l(0), r0.y
store_structured g2.x, vThreadIDInGroupFlattened.x, l(0), r0.z
store_structured g3.x, vThreadIDInGroupFlattened.x, l(0), r0.w
sync_g_t
if_z vThreadIDInGroupFlattened.x
  ld_structured r1.x, l(2), l(0), g0.xxxx
  ld_structured r1.y, l(2), l(0), g1.xxxx
  ld_structured r1.z, l(2), l(0), g2.xxxx
  ld_structured r1.w, l(2), l(0), g3.xxxx
  ld_structured r2.x, l(8), l(0), g0.xxxx
  ld_structured r2.y, l(8), l(0), g1.xxxx
  ld_structured r2.z, l(8), l(0), g2.xxxx
  ld_structured r2.w, l(8), l(0), g3.xxxx
  ld_structured r3.x, l(10), l(0), g0.xxxx
  ld_structured r3.y, l(10), l(0), g1.xxxx
  ld_structured r3.z, l(10), l(0), g2.xxxx
  ld_structured r3.w, l(10), l(0), g3.xxxx
  ld_structured r4.x, l(32), l(0), g0.xxxx
  ld_structured r4.y, l(32), l(0), g1.xxxx
  ld_structured r4.z, l(32), l(0), g2.xxxx
  ld_structured r4.w, l(32), l(0), g3.xxxx
  ld_structured r5.x, l(34), l(0), g0.xxxx
  ld_structured r5.y, l(34), l(0), g1.xxxx
  ld_structured r5.z, l(34), l(0), g2.xxxx
  ld_structured r5.w, l(34), l(0), g3.xxxx
  ld_structured r6.x, l(40), l(0), g0.xxxx
  ld_structured r6.y, l(40), l(0), g1.xxxx
  ld_structured r6.z, l(40), l(0), g2.xxxx
  ld_structured r6.w, l(40), l(0), g3.xxxx
  ld_structured r7.x, l(42), l(0), g0.xxxx
  ld_structured r7.y, l(42), l(0), g1.xxxx
  ld_structured r7.z, l(42), l(0), g2.xxxx
  ld_structured r7.w, l(42), l(0), g3.xxxx
  add r0.xyzw, r0.xyzw, r1.xyzw
  add r0.xyzw, r2.xyzw, r0.xyzw
  add r0.xyzw, r3.xyzw, r0.xyzw
  add r0.xyzw, r4.xyzw, r0.xyzw
  add r0.xyzw, r5.xyzw, r0.xyzw
  add r0.xyzw, r6.xyzw, r0.xyzw
  add r0.xyzw, r7.xyzw, r0.xyzw
  mul r1.w, r0.w, l(0.125000)
  ushr r2.xyzw, vThreadID.xyzz, l(2, 2, 2, 2)
  lt r3.xyz, r0.xyzx, l(0.025046, 0.025046, 0.025046, 0.000000)
  mul r4.xyz, r0.xyzx, l(1.615000, 1.615000, 1.615000, 0.000000)
  mad r5.xyz, r0.xyzx, l(0.125000, 0.125000, 0.125000, 0.000000), l(-0.002280, -0.002280, -0.002280, 0.000000)
  sqrt r5.xyz, |r5.xyzx|
  mul r0.xyz, r0.xyzx, l(0.016810, 0.016810, 0.016810, 0.000000)
  mad r0.xyz, r5.xyzx, l(1.130050, 1.130050, 1.130050, 0.000000), -r0.xyzx
  add r0.xyz, r0.xyzx, l(0.005719, 0.005719, 0.005719, 0.000000)
  movc r1.xyz, r3.xyzx, r4.xyzx, r0.xyzx
  store_uav_typed u2.xyzw, r2.xyzw, r1.xyzw
endif 
ret 
// Approximately 0 instruction slots used
#endif

const BYTE g_GenerateMips3DCS_sRGB[] =
{
     68,  88,  66,  67, 240, 199, 
     75,  69, 222, 165, 177, 212, 
      5, 139, 249, 166,  95,  68, 
    111,  53,   1,   0,   0,   0, 
    204,  18,   0,   0,   4,   0, 
      0,   0,  48,   0,   0,   0, 
     64,   0,   0,   0,  80,   0, 
      0,   0,   8,  18,   0,   0, 
     73,  83,  71,  78,   8,   0, 
      0,   0,   0,   0,   0,   0, 
      8,   0,   0,   0,  79,  83, 
     71,  78,   8,   0,   0,   0, 
      0,   0,   0,   0,   8,   0, 
      0,   0,  83,  72,  69,  88, 
    176,  17,   0,   0,  80,   0, 
      5,   0, 108,   4,   0,   0, 
    106,   8,   0,   1,  89,   0, 
      0,   4,  70, 142,  32,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   0,  90,   0,   0,   3, 
      0,  96,  16,   0,   0,   0, 
      0,   0,  88,  40,   0,   4, 
      0, 112,  16,   0,   0,   0, 
      0,   0,  85,  85,   0,   0, 
    156,  40,   0,   4,   0, 224, 
     17,   0,   0,   0,   0,   0, 
     85,  85,   0,   0, 156,  40, 
      0,   4,   0, 224,  17,   0, 
      1,   0,   0,   0,  85,  85, 
      0,   0, 156,  40,   0,   4, 
      0, 224,  17,   0,   2,   0, 
      0,   0,  85,  85,   0,   0, 
     95,   0,   0,   2,   0,  64, 
      2,   0,  95,   0,   0,   2, 
    114,   0,   2,   0, 104,   0, 
      0,   2,   9,   0,   0,   0, 
    160,   0,   0,   5,   0, 240, 
     17,   0,   0,   0,   0,   0, 
      4,   0,   0,   0,  64,   0, 
      0,   0, 160,   0,   0,   5, 
      0, 240,  17,   0,   1,   0, 
      0,   0,   4,   0,   0,   0, 
     64,   0,   0,   0, 160,   0, 
      0,   5,   0, 240,  17,   0, 
      2,   0,   0,   0,   4,   0, 
      0,   0,  64,   0,   0,   0, 
    160,   0,   0,   5,   0, 240, 
     17,   0,   3,   0,   0,   0, 
      4,   0,   0,   0,  64,   0, 
      0,   0, 155,   0,   0,   4, 
      4,   0,   0,   0,   4,   0, 
      0,   0,   4,   0,   0,   0, 
     86,   0,   0,   4, 114,   0, 
     16,   0,   0,   0,   0,   0, 
     70,   2,   2,   0,   0,   0, 
      0,  10, 114,   0,  16,   0, 
      0,   0,   0,   0,  70,   2, 
     16,   0,   0,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0,  63,   0,   0,   0,  63, 
      0,   0,   0,  63,   0,   0, 
      0,   0,  56,   0,   0,   8, 
    114,   0,  16,   0,   0,   0, 
      0,   0,  70,   2,  16,   0, 
      0,   0,   0,   0,  70, 130, 
     32,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  86,   0, 
      0,   6, 130,   0,  16,   0, 
      0,   0,   0,   0,  58, 128, 
     32,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  72,   0, 
      0, 141,  66,   1,   0, 128, 
     67,  85,  21,   0, 242,   0, 
     16,   0,   0,   0,   0,   0, 
     70,   2,  16,   0,   0,   0, 
      0,   0,  70, 126,  16,   0, 
      0,   0,   0,   0,   0,  96, 
     16,   0,   0,   0,   0,   0, 
     58,   0,  16,   0,   0,   0, 
      0,   0,  49,   0,   0,  10, 
    114,   0,  16,   0,   1,   0, 
      0,   0,  70,   2,  16,   0, 
      0,   0,   0,   0,   2,  64, 
      0,   0,  28,  46,  77,  59, 
     28,  46,  77,  59,  28,  46, 
     77,  59,   0,   0,   0,   0, 
     56,   0,   0,  10, 114,   0, 
     16,   0,   2,   0,   0,   0, 
     70,   2,  16,   0,   0,   0, 
      0,   0,   2,  64,   0,   0, 
     82, 184,  78,  65,  82, 184, 
     78,  65,  82, 184,  78,  65, 
      0,   0,   0,   0,   0,   0, 
      0,  10, 114,   0,  16,   0, 
      3,   0,   0,   0,  70,   2, 
     16,   0,   0,   0,   0,   0, 
      2,  64,   0,   0,  13, 108, 
     21, 187,  13, 108,  21, 187, 
     13, 108,  21, 187,   0,   0, 
      0,   0,  75,   0,   0,   6, 
    114,   0,  16,   0,   3,   0, 
      0,   0,  70,   2,  16, 128, 
    129,   0,   0,   0,   3,   0, 
      0,   0,  56,   0,   0,  10, 
    114,   0,  16,   0,   4,   0, 
      0,   0,  70,   2,  16,   0, 
      0,   0,   0,   0,   2,  64, 
      0,   0,  32, 181,   9,  62, 
     32, 181,   9,  62,  32, 181, 
      9,  62,   0,   0,   0,   0, 
     50,   0,   0,  13, 114,   0, 
     16,   0,   3,   0,   0,   0, 
     70,   2,  16,   0,   3,   0, 
      0,   0,   2,  64,   0,   0, 
    122, 165, 144,  63, 122, 165, 
    144,  63, 122, 165, 144,  63, 
      0,   0,   0,   0,  70,   2, 
     16, 128,  65,   0,   0,   0, 
      4,   0,   0,   0,   0,   0, 
      0,  10, 114,   0,  16,   0, 
      3,   0,   0,   0,  70,   2, 
     16,   0,   3,   0,   0,   0, 
      2,  64,   0,   0, 115, 102, 
    187,  59, 115, 102, 187,  59, 
    115, 102, 187,  59,   0,   0, 
      0,   0,  55,   0,   0,   9, 
    114,   0,  16,   0,   1,   0, 
      0,   0,  70,   2,  16,   0, 
      1,   0,   0,   0,  70,   2, 
     16,   0,   2,   0,   0,   0, 
     70,   2,  16,   0,   3,   0, 
      0,   0,  54,   0,   0,   5, 
    130,   0,  16,   0,   1,   0, 
      0,   0,  58,   0,  16,   0, 
      0,   0,   0,   0, 164,   0, 
      0,   6, 242, 224,  17,   0, 
      0,   0,   0,   0,  70,  10, 
      2,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,  32,   0, 
      0,   8,  18,   0,  16,   0, 
      1,   0,   0,   0,  10, 128, 
     32,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   1,   0,   0,   0, 
     31,   0,   4,   3,  10,   0, 
     16,   0,   1,   0,   0,   0, 
     62,   0,   0,   1,  21,   0, 
      0,   1, 168,   0,   0,   8, 
     18, 240,  17,   0,   0,   0, 
      0,   0,  10,  64,   2,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0, 168,   0, 
      0,   8,  18, 240,  17,   0, 
      1,   0,   0,   0,  10,  64, 
      2,   0,   1,  64,   0,   0, 
      0,   0,   0,   0,  26,   0, 
     16,   0,   0,   0,   0,   0, 
    168,   0,   0,   8,  18, 240, 
     17,   0,   2,   0,   0,   0, 
     10,  64,   2,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
     42,   0,  16,   0,   0,   0, 
      0,   0, 168,   0,   0,   8, 
     18, 240,  17,   0,   3,   0, 
      0,   0,  10,  64,   2,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,  58,   0,  16,   0, 
      1,   0,   0,   0, 190,  24, 
      0,   1,   1,   0,   0,   6, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  10,  64,   2,   0, 
      1,  64,   0,   0,  21,   0, 
      0,   0,  31,   0,   0,   3, 
     10,   0,  16,   0,   1,   0, 
      0,   0,  30,   0,   0,   9, 
    242,   0,  16,   0,   1,   0, 
      0,   0,   6,  64,   2,   0, 
      2,  64,   0,   0,   1,   0, 
      0,   0,   4,   0,   0,   0, 
      5,   0,   0,   0,  16,   0, 
      0,   0, 167,   0,   0,   9, 
     18,   0,  16,   0,   2,   0, 
      0,   0,  10,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   0,   0, 
      0,   0, 167,   0,   0,   9, 
     34,   0,  16,   0,   2,   0, 
      0,   0,  10,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   1,   0, 
      0,   0, 167,   0,   0,   9, 
     66,   0,  16,   0,   2,   0, 
      0,   0,  10,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   2,   0, 
      0,   0, 167,   0,   0,   9, 
    130,   0,  16,   0,   2,   0, 
      0,   0,  10,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   3,   0, 
      0,   0, 167,   0,   0,   9, 
     18,   0,  16,   0,   3,   0, 
      0,   0,  26,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   0,   0, 
      0,   0, 167,   0,   0,   9, 
     34,   0,  16,   0,   3,   0, 
      0,   0,  26,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   1,   0, 
      0,   0, 167,   0,   0,   9, 
     66,   0,  16,   0,   3,   0, 
      0,   0,  26,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   2,   0, 
      0,   0, 167,   0,   0,   9, 
    130,   0,  16,   0,   3,   0, 
      0,   0,  26,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   3,   0, 
      0,   0, 167,   0,   0,   9, 
     18,   0,  16,   0,   4,   0, 
      0,   0,  42,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   0,   0, 
      0,   0, 167,   0,   0,   9, 
     34,   0,  16,   0,   4,   0, 
      0,   0,  42,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   1,   0, 
      0,   0, 167,   0,   0,   9, 
     66,   0,  16,   0,   4,   0, 
      0,   0,  42,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   2,   0, 
      0,   0, 167,   0,   0,   9, 
    130,   0,  16,   0,   4,   0, 
      0,   0,  42,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   3,   0, 
      0,   0, 167,   0,   0,   9, 
     18,   0,  16,   0,   5,   0, 
      0,   0,  58,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   0,   0, 
      0,   0, 167,   0,   0,   9, 
     34,   0,  16,   0,   5,   0, 
      0,   0,  58,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   1,   0, 
      0,   0, 167,   0,   0,   9, 
     66,   0,  16,   0,   5,   0, 
      0,   0,  58,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   2,   0, 
      0,   0, 167,   0,   0,   9, 
    130,   0,  16,   0,   5,   0, 
      0,   0,  58,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   3,   0, 
      0,   0,  30,   0,   0,   9, 
    114,   0,  16,   0,   1,   0, 
      0,   0,   6,  64,   2,   0, 
      2,  64,   0,   0,  17,   0, 
      0,   0,  20,   0,   0,   0, 
     21,   0,   0,   0,   0,   0, 
      0,   0, 167,   0,   0,   9, 
     18,   0,  16,   0,   6,   0, 
      0,   0,  10,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   0,   0, 
      0,   0, 167,   0,   0,   9, 
     34,   0,  16,   0,   6,   0, 
      0,   0,  10,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   1,   0, 
      0,   0, 167,   0,   0,   9, 
     66,   0,  16,   0,   6,   0, 
      0,   0,  10,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   2,   0, 
      0,   0, 167,   0,   0,   9, 
    130,   0,  16,   0,   6,   0, 
      0,   0,  10,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   3,   0, 
      0,   0, 167,   0,   0,   9, 
     18,   0,  16,   0,   7,   0, 
      0,   0,  26,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   0,   0, 
      0,   0, 167,   0,   0,   9, 
     34,   0,  16,   0,   7,   0, 
      0,   0,  26,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   1,   0, 
      0,   0, 167,   0,   0,   9, 
     66,   0,  16,   0,   7,   0, 
      0,   0,  26,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   2,   0, 
      0,   0, 167,   0,   0,   9, 
    130,   0,  16,   0,   7,   0, 
      0,   0,  26,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   3,   0, 
      0,   0, 167,   0,   0,   9, 
     18,   0,  16,   0,   8,   0, 
      0,   0,  42,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   0,   0, 
      0,   0, 167,   0,   0,   9, 
     34,   0,  16,   0,   8,   0, 
      0,   0,  42,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   1,   0, 
      0,   0, 167,   0,   0,   9, 
     66,   0,  16,   0,   8,   0, 
      0,   0,  42,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   2,   0, 
      0,   0, 167,   0,   0,   9, 
    130,   0,  16,   0,   8,   0, 
      0,   0,  42,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   3,   0, 
      0,   0,   0,   0,   0,   7, 
    242,   0,  16,   0,   1,   0, 
      0,   0,  70,  14,  16,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   2,   0,   0,   0, 
      0,   0,   0,   7, 242,   0, 
     16,   0,   1,   0,   0,   0, 
     70,  14,  16,   0,   3,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   7, 242,   0,  16,   0, 
      1,   0,   0,   0,  70,  14, 
     16,   0,   4,   0,   0,   0, 
     70,  14,  16,   0,   1,   0, 
      0,   0,   0,   0,   0,   7, 
    242,   0,  16,   0,   1,   0, 
      0,   0,  70,  14,  16,   0, 
      5,   0,   0,   0,  70,  14, 
     16,   0,   1,   0,   0,   0, 
      0,   0,   0,   7, 242,   0, 
     16,   0,   1,   0,   0,   0, 
     70,  14,  16,   0,   6,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   7, 242,   0,  16,   0, 
      1,   0,   0,   0,  70,  14, 
     16,   0,   7,   0,   0,   0, 
     70,  14,  16,   0,   1,   0, 
      0,   0,   0,   0,   0,   7, 
    242,   0,  16,   0,   1,   0, 
      0,   0,  70,  14,  16,   0, 
      8,   0,   0,   0,  70,  14, 
     16,   0,   1,   0,   0,   0, 
     56,   0,   0,  10, 242,   0, 
     16,   0,   0,   0,   0,   0, 
     70,  14,  16,   0,   1,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,  62,   0,   0, 
      0,  62,   0,   0,   0,  62, 
      0,   0,   0,  62,  85,   0, 
      0,   9, 242,   0,  16,   0, 
      2,   0,   0,   0,  70,  10, 
      2,   0,   2,  64,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0,  49,   0, 
      0,  10, 114,   0,  16,   0, 
      3,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
      2,  64,   0,   0,  28,  46, 
    205,  60,  28,  46, 205,  60, 
     28,  46, 205,  60,   0,   0, 
      0,   0,  56,   0,   0,  10, 
    114,   0,  16,   0,   4,   0, 
      0,   0,  70,   2,  16,   0, 
      1,   0,   0,   0,   2,  64, 
      0,   0,  82, 184, 206,  63, 
     82, 184, 206,  63,  82, 184, 
    206,  63,   0,   0,   0,   0, 
     50,   0,   0,  15, 114,   0, 
     16,   0,   5,   0,   0,   0, 
     70,   2,  16,   0,   1,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,  62,   0,   0, 
      0,  62,   0,   0,   0,  62, 
      0,   0,   0,   0,   2,  64, 
      0,   0,  13, 108,  21, 187, 
     13, 108,  21, 187,  13, 108, 
     21, 187,   0,   0,   0,   0, 
     75,   0,   0,   6, 114,   0, 
     16,   0,   5,   0,   0,   0, 
     70,   2,  16, 128, 129,   0, 
      0,   0,   5,   0,   0,   0, 
     56,   0,   0,  10, 114,   0, 
     16,   0,   1,   0,   0,   0, 
     70,   2,  16,   0,   1,   0, 
      0,   0,   2,  64,   0,   0, 
     32, 181, 137,  60,  32, 181, 
    137,  60,  32, 181, 137,  60, 
      0,   0,   0,   0,  50,   0, 
      0,  13, 114,   0,  16,   0, 
      1,   0,   0,   0,  70,   2, 
     16,   0,   5,   0,   0,   0, 
      2,  64,   0,   0, 122, 165, 
    144,  63, 122, 165, 144,  63, 
    122, 165, 144,  63,   0,   0, 
      0,   0,  70,   2,  16, 128, 
     65,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,  10, 
    114,   0,  16,   0,   1,   0, 
      0,   0,  70,   2,  16,   0, 
      1,   0,   0,   0,   2,  64, 
      0,   0, 115, 102, 187,  59, 
    115, 102, 187,  59, 115, 102, 
    187,  59,   0,   0,   0,   0, 
     55,   0,   0,   9, 114,   0, 
     16,   0,   1,   0,   0,   0, 
     70,   2,  16,   0,   3,   0, 
      0,   0,  70,   2,  16,   0, 
      4,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
     54,   0,   0,   5, 130,   0, 
     16,   0,   1,   0,   0,   0, 
     58,   0,  16,   0,   0,   0, 
      0,   0, 164,   0,   0,   7, 
    242, 224,  17,   0,   1,   0, 
      0,   0,  70,  14,  16,   0, 
      2,   0,   0,   0,  70,  14, 
     16,   0,   1,   0,   0,   0, 
     21,   0,   0,   1,  32,   0, 
      0,   8,  18,   0,  16,   0, 
      1,   0,   0,   0,  10, 128, 
     32,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   2,   0,   0,   0, 
     31,   0,   4,   3,  10,   0, 
     16,   0,   1,   0,   0,   0, 
     62,   0,   0,   1,  21,   0, 
      0,   1, 168,   0,   0,   8, 
     18, 240,  17,   0,   0,   0, 
      0,   0,  10,  64,   2,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0, 168,   0, 
      0,   8,  18, 240,  17,   0, 
      1,   0,   0,   0,  10,  64, 
      2,   0,   1,  64,   0,   0, 
      0,   0,   0,   0,  26,   0, 
     16,   0,   0,   0,   0,   0, 
    168,   0,   0,   8,  18, 240, 
     17,   0,   2,   0,   0,   0, 
     10,  64,   2,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
     42,   0,  16,   0,   0,   0, 
      0,   0, 168,   0,   0,   8, 
     18, 240,  17,   0,   3,   0, 
      0,   0,  10,  64,   2,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,  58,   0,  16,   0, 
      0,   0,   0,   0, 190,  24, 
      0,   1,  31,   0,   0,   2, 
     10,  64,   2,   0, 167,   0, 
      0,   9,  18,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   2,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      0,   0,   0,   0, 167,   0, 
      0,   9,  34,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   2,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      1,   0,   0,   0, 167,   0, 
      0,   9,  66,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   2,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      2,   0,   0,   0, 167,   0, 
      0,   9, 130,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   2,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      3,   0,   0,   0, 167,   0, 
      0,   9,  18,   0,  16,   0, 
      2,   0,   0,   0,   1,  64, 
      0,   0,   8,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      0,   0,   0,   0, 167,   0, 
      0,   9,  34,   0,  16,   0, 
      2,   0,   0,   0,   1,  64, 
      0,   0,   8,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      1,   0,   0,   0, 167,   0, 
      0,   9,  66,   0,  16,   0, 
      2,   0,   0,   0,   1,  64, 
      0,   0,   8,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      2,   0,   0,   0, 167,   0, 
      0,   9, 130,   0,  16,   0, 
      2,   0,   0,   0,   1,  64, 
      0,   0,   8,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      3,   0,   0,   0, 167,   0, 
      0,   9,  18,   0,  16,   0, 
      3,   0,   0,   0,   1,  64, 
      0,   0,  10,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      0,   0,   0,   0, 167,   0, 
      0,   9,  34,   0,  16,   0, 
      3,   0,   0,   0,   1,  64, 
      0,   0,  10,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      1,   0,   0,   0, 167,   0, 
      0,   9,  66,   0,  16,   0, 
      3,   0,   0,   0,   1,  64, 
      0,   0,  10,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      2,   0,   0,   0, 167,   0, 
      0,   9, 130,   0,  16,   0, 
      3,   0,   0,   0,   1,  64, 
      0,   0,  10,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      3,   0,   0,   0, 167,   0, 
      0,   9,  18,   0,  16,   0, 
      4,   0,   0,   0,   1,  64, 
      0,   0,  32,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      0,   0,   0,   0, 167,   0, 
      0,   9,  34,   0,  16,   0, 
      4,   0,   0,   0,   1,  64, 
      0,   0,  32,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      1,   0,   0,   0, 167,   0, 
      0,   9,  66,   0,  16,   0, 
      4,   0,   0,   0,   1,  64, 
      0,   0,  32,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      2,   0,   0,   0, 167,   0, 
      0,   9, 130,   0,  16,   0, 
      4,   0,   0,   0,   1,  64, 
      0,   0,  32,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      3,   0,   0,   0, 167,   0, 
      0,   9,  18,   0,  16,   0, 
      5,   0,   0,   0,   1,  64, 
      0,   0,  34,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      0,   0,   0,   0, 167,   0, 
      0,   9,  34,   0,  16,   0, 
      5,   0,   0,   0,   1,  64, 
      0,   0,  34,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      1,   0,   0,   0, 167,   0, 
      0,   9,  66,   0,  16,   0, 
      5,   0,   0,   0,   1,  64, 
      0,   0,  34,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      2,   0,   0,   0, 167,   0, 
      0,   9, 130,   0,  16,   0, 
      5,   0,   0,   0,   1,  64, 
      0,   0,  34,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      3,   0,   0,   0, 167,   0, 
      0,   9,  18,   0,  16,   0, 
      6,   0,   0,   0,   1,  64, 
      0,   0,  40,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      0,   0,   0,   0, 167,   0, 
      0,   9,  34,   0,  16,   0, 
      6,   0,   0,   0,   1,  64, 
      0,   0,  40,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      1,   0,   0,   0, 167,   0, 
      0,   9,  66,   0,  16,   0, 
      6,   0,   0,   0,   1,  64, 
      0,   0,  40,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      2,   0,   0,   0, 167,   0, 
      0,   9, 130,   0,  16,   0, 
      6,   0,   0,   0,   1,  64, 
      0,   0,  40,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      3,   0,   0,   0, 167,   0, 
      0,   9,  18,   0,  16,   0, 
      7,   0,   0,   0,   1,  64, 
      0,   0,  42,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      0,   0,   0,   0, 167,   0, 
      0,   9,  34,   0,  16,   0, 
      7,   0,   0,   0,   1,  64, 
      0,   0,  42,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      1,   0,   0,   0, 167,   0, 
      0,   9,  66,   0,  16,   0, 
      7,   0,   0,   0,   1,  64, 
      0,   0,  42,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      2,   0,   0,   0, 167,   0, 
      0,   9, 130,   0,  16,   0, 
      7,   0,   0,   0,   1,  64, 
      0,   0,  42,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 240,  17,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   7, 242,   0,  16,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   0,   0,   0,   0, 
     70,  14,  16,   0,   1,   0, 
      0,   0,   0,   0,   0,   7, 
    242,   0,  16,   0,   0,   0, 
      0,   0,  70,  14,  16,   0, 
      2,   0,   0,   0,  70,  14, 
     16,   0,   0,   0,   0,   0, 
      0,   0,   0,   7, 242,   0, 
     16,   0,   0,   0,   0,   0, 
     70,  14,  16,   0,   3,   0, 
      0,   0,  70,  14,  16,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   7, 242,   0,  16,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   4,   0,   0,   0, 
     70,  14,  16,   0,   0,   0, 
      0,   0,   0,   0,   0,   7, 
    242,   0,  16,   0,   0,   0, 
      0,   0,  70,  14,  16,   0, 
      5,   0,   0,   0,  70,  14, 
     16,   0,   0,   0,   0,   0, 
      0,   0,   0,   7, 242,   0, 
     16,   0,   0,   0,   0,   0, 
     70,  14,  16,   0,   6,   0, 
      0,   0,  70,  14,  16,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   7, 242,   0,  16,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   7,   0,   0,   0, 
     70,  14,  16,   0,   0,   0, 
      0,   0,  56,   0,   0,   7, 
    130,   0,  16,   0,   1,   0, 
      0,   0,  58,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,  62, 
     85,   0,   0,   9, 242,   0, 
     16,   0,   2,   0,   0,   0, 
     70,  10,   2,   0,   2,  64, 
      0,   0,   2,   0,   0,   0, 
      2,   0,   0,   0,   2,   0, 
      0,   0,   2,   0,   0,   0, 
     49,   0,   0,  10, 114,   0, 
     16,   0,   3,   0,   0,   0, 
     70,   2,  16,   0,   0,   0, 
      0,   0,   2,  64,   0,   0, 
     28,  46, 205,  60,  28,  46, 
    205,  60,  28,  46, 205,  60, 
      0,   0,   0,   0,  56,   0, 
      0,  10, 114,   0,  16,   0, 
      4,   0,   0,   0,  70,   2, 
     16,   0,   0,   0,   0,   0, 
      2,  64,   0,   0,  82, 184, 
    206,  63,  82, 184, 206,  63, 
     82, 184, 206,  63,   0,   0, 
      0,   0,  50,   0,   0,  15, 
    114,   0,  16,   0,   5,   0, 
      0,   0,  70,   2,  16,   0, 
      0,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,  62, 
      0,   0,   0,  62,   0,   0, 
      0,  62,   0,   0,   0,   0, 
      2,  64,   0,   0,  13, 108, 
     21, 187,  13, 108,  21, 187, 
     13, 108,  21, 187,   0,   0, 
      0,   0,  75,   0,   0,   6, 
    114,   0,  16,   0,   5,   0, 
      0,   0,  70,   2,  16, 128, 
    129,   0,   0,   0,   5,   0, 
      0,   0,  56,   0,   0,  10, 
    114,   0,  16,   0,   0,   0, 
      0,   0,  70,   2,  16,   0, 
      0,   0,   0,   0,   2,  64, 
      0,   0,  32, 181, 137,  60, 
     32, 181, 137,  60,  32, 181, 
    137,  60,   0,   0,   0,   0, 
     50,   0,   0,  13, 114,   0, 
     16,   0,   0,   0,   0,   0, 
     70,   2,  16,   0,   5,   0, 
      0,   0,   2,  64,   0,   0, 
    122, 165, 144,  63, 122, 165, 
    144,  63, 122, 165, 144,  63, 
      0,   0,   0,   0,  70,   2, 
     16, 128,  65,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,  10, 114,   0,  16,   0, 
      0,   0,   0,   0,  70,   2, 
     16,   0,   0,   0,   0,   0, 
      2,  64,   0,   0, 115, 102, 
    187,  59, 115, 102, 187,  59, 
    115, 102, 187,  59,   0,   0, 
      0,   0,  55,   0,   0,   9, 
    114,   0,  16,   0,   1,   0, 
      0,   0,  70,   2,  16,   0, 
      3,   0,   0,   0,  70,   2, 
     16,   0,   4,   0,   0,   0, 
     70,   2,  16,   0,   0,   0, 
      0,   0, 164,   0,   0,   7, 
    242, 224,  17,   0,   2,   0, 
      0,   0,  70,  14,  16,   0, 
      2,   0,   0,   0,  70,  14, 
     16,   0,   1,   0,   0,   0, 
     21,   0,   0,   1,  62,   0, 
      0,   1,  82,  84,  83,  48, 
    188,   0,   0,   0,   2,   0, 
      0,   0,   3,   0,   0,   0, 
     24,   0,   0,   0,   1,   0, 
      0,   0, 136,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
     60,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     72,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    104,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      5,   0,   0,   0,   1,   0, 
      0,   0,  80,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      1,   0,   0,   0, 112,   0, 
      0,   0,   1,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,  20,   0,   0,   0, 
      3,   0,   0,   0,   3,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,  16,   0, 
      0,   0,   4,   0,   0,   0, 
      2,   0,   0,   0,   0,   0, 
      0,   0, 255, 255, 127, 127, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0
};
