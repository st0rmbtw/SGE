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
dcl_resource_texture2darray (float,float,float,float) t0
dcl_uav_typed_texture2darray (float,float,float,float) u0
dcl_uav_typed_texture2darray (float,float,float,float) u1
dcl_uav_typed_texture2darray (float,float,float,float) u2
dcl_uav_typed_texture2darray (float,float,float,float) u3
dcl_input vThreadIDInGroupFlattened
dcl_input vThreadID.xyz
dcl_temps 7
dcl_tgsm_structured g0, 4, 64
dcl_tgsm_structured g1, 4, 64
dcl_tgsm_structured g2, 4, 64
dcl_tgsm_structured g3, 4, 64
dcl_thread_group 8, 8, 1
iadd r0.zw, vThreadID.zzzz, cb0[1].xxxx
utof r1.xy, vThreadID.xyxx
add r1.xy, r1.xyxx, l(0.250000, 0.250000, 0.000000, 0.000000)
mul r1.xy, r1.xyxx, cb0[0].xyxx
utof r1.z, r0.w
utof r1.w, cb0[0].z
sample_l_indexable(texture2darray)(float,float,float,float) r2.xyzw, r1.xyzx, t0.xyzw, s0, r1.w
mul r3.x, cb0[0].x, l(0.500000)
mov r3.yz, l(0,0,0,0)
add r3.xyz, r1.xyzx, r3.xyzx
sample_l_indexable(texture2darray)(float,float,float,float) r3.xyzw, r3.xyzx, t0.xyzw, s0, r1.w
add r2.xyzw, r2.xyzw, r3.xyzw
mov r3.xz, l(0,0,0,0)
mul r3.y, cb0[0].y, l(0.500000)
add r3.xyz, r1.xyzx, r3.xyzx
sample_l_indexable(texture2darray)(float,float,float,float) r3.xyzw, r3.xyzx, t0.xyzw, s0, r1.w
add r2.xyzw, r2.xyzw, r3.xyzw
mul r3.xy, cb0[0].xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
mov r3.z, l(0)
add r1.xyz, r1.xyzx, r3.xyzx
sample_l_indexable(texture2darray)(float,float,float,float) r1.xyzw, r1.xyzx, t0.xyzw, s0, r1.w
add r1.xyzw, r1.xyzw, r2.xyzw
mul r2.xyzw, r1.xyzw, l(0.250000, 0.250000, 0.250000, 0.250000)
lt r3.xyz, r1.xyzx, l(0.012523, 0.012523, 0.012523, 0.000000)
mul r4.xyz, r1.xyzx, l(3.230000, 3.230000, 3.230000, 0.000000)
mad r5.xyz, r1.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), l(-0.002280, -0.002280, -0.002280, 0.000000)
sqrt r5.xyz, |r5.xyzx|
mul r6.xyz, r1.xyzx, l(0.033620, 0.033620, 0.033620, 0.000000)
mad r5.xyz, r5.xyzx, l(1.130050, 1.130050, 1.130050, 0.000000), -r6.xyzx
add r5.xyz, r5.xyzx, l(0.005719, 0.005719, 0.005719, 0.000000)
movc r3.xyz, r3.xyzx, r4.xyzx, r5.xyzx
mov r0.xy, vThreadID.xyxx
mov r3.w, r2.w
store_uav_typed u0.xyzw, r0.xyzw, r3.xyzw
ieq r0.x, cb0[0].w, l(1)
if_nz r0.x
  ret 
endif 
store_structured g0.x, vThreadIDInGroupFlattened.x, l(0), r2.x
store_structured g1.x, vThreadIDInGroupFlattened.x, l(0), r2.y
store_structured g2.x, vThreadIDInGroupFlattened.x, l(0), r2.z
store_structured g3.x, vThreadIDInGroupFlattened.x, l(0), r3.w
sync_g_t
and r0.x, vThreadIDInGroupFlattened.x, l(9)
if_z r0.x
  iadd r0.xyz, vThreadIDInGroupFlattened.xxxx, l(1, 8, 9, 0)
  ld_structured r3.x, r0.x, l(0), g0.xxxx
  ld_structured r3.y, r0.x, l(0), g1.xxxx
  ld_structured r3.z, r0.x, l(0), g2.xxxx
  ld_structured r3.w, r0.x, l(0), g3.xxxx
  ld_structured r4.x, r0.y, l(0), g0.xxxx
  ld_structured r4.y, r0.y, l(0), g1.xxxx
  ld_structured r4.z, r0.y, l(0), g2.xxxx
  ld_structured r4.w, r0.y, l(0), g3.xxxx
  ld_structured r5.x, r0.z, l(0), g0.xxxx
  ld_structured r5.y, r0.z, l(0), g1.xxxx
  ld_structured r5.z, r0.z, l(0), g2.xxxx
  ld_structured r5.w, r0.z, l(0), g3.xxxx
  mad r1.xyzw, r1.xyzw, l(0.250000, 0.250000, 0.250000, 0.250000), r3.xyzw
  add r1.xyzw, r4.xyzw, r1.xyzw
  add r1.xyzw, r5.xyzw, r1.xyzw
  mul r2.xyzw, r1.xyzw, l(0.250000, 0.250000, 0.250000, 0.250000)
  ushr r3.xy, vThreadID.xyxx, l(1, 1, 0, 0)
  lt r0.xyz, r1.xyzx, l(0.012523, 0.012523, 0.012523, 0.000000)
  mul r4.xyz, r1.xyzx, l(3.230000, 3.230000, 3.230000, 0.000000)
  mad r5.xyz, r1.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), l(-0.002280, -0.002280, -0.002280, 0.000000)
  sqrt r5.xyz, |r5.xyzx|
  mul r1.xyz, r1.xyzx, l(0.033620, 0.033620, 0.033620, 0.000000)
  mad r1.xyz, r5.xyzx, l(1.130050, 1.130050, 1.130050, 0.000000), -r1.xyzx
  add r1.xyz, r1.xyzx, l(0.005719, 0.005719, 0.005719, 0.000000)
  movc r1.xyz, r0.xyzx, r4.xyzx, r1.xyzx
  mov r3.zw, r0.wwww
  mov r1.w, r2.w
  store_uav_typed u1.xyzw, r3.xyzw, r1.xyzw
  store_structured g0.x, vThreadIDInGroupFlattened.x, l(0), r2.x
  store_structured g1.x, vThreadIDInGroupFlattened.x, l(0), r2.y
  store_structured g2.x, vThreadIDInGroupFlattened.x, l(0), r2.z
  store_structured g3.x, vThreadIDInGroupFlattened.x, l(0), r1.w
endif 
ieq r0.x, cb0[0].w, l(2)
if_nz r0.x
  ret 
endif 
sync_g_t
and r0.x, vThreadIDInGroupFlattened.x, l(27)
if_z r0.x
  iadd r0.xyz, vThreadIDInGroupFlattened.xxxx, l(2, 16, 18, 0)
  ld_structured r1.x, r0.x, l(0), g0.xxxx
  ld_structured r1.y, r0.x, l(0), g1.xxxx
  ld_structured r1.z, r0.x, l(0), g2.xxxx
  ld_structured r1.w, r0.x, l(0), g3.xxxx
  ld_structured r3.x, r0.y, l(0), g0.xxxx
  ld_structured r3.y, r0.y, l(0), g1.xxxx
  ld_structured r3.z, r0.y, l(0), g2.xxxx
  ld_structured r3.w, r0.y, l(0), g3.xxxx
  ld_structured r4.x, r0.z, l(0), g0.xxxx
  ld_structured r4.y, r0.z, l(0), g1.xxxx
  ld_structured r4.z, r0.z, l(0), g2.xxxx
  ld_structured r4.w, r0.z, l(0), g3.xxxx
  add r1.xyzw, r1.xyzw, r2.xyzw
  add r1.xyzw, r3.xyzw, r1.xyzw
  add r1.xyzw, r4.xyzw, r1.xyzw
  mul r2.xyzw, r1.xyzw, l(0.250000, 0.250000, 0.250000, 0.250000)
  ushr r3.xy, vThreadID.xyxx, l(2, 2, 0, 0)
  lt r0.xyz, r1.xyzx, l(0.012523, 0.012523, 0.012523, 0.000000)
  mul r4.xyz, r1.xyzx, l(3.230000, 3.230000, 3.230000, 0.000000)
  mad r5.xyz, r1.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), l(-0.002280, -0.002280, -0.002280, 0.000000)
  sqrt r5.xyz, |r5.xyzx|
  mul r1.xyz, r1.xyzx, l(0.033620, 0.033620, 0.033620, 0.000000)
  mad r1.xyz, r5.xyzx, l(1.130050, 1.130050, 1.130050, 0.000000), -r1.xyzx
  add r1.xyz, r1.xyzx, l(0.005719, 0.005719, 0.005719, 0.000000)
  movc r1.xyz, r0.xyzx, r4.xyzx, r1.xyzx
  mov r3.zw, r0.wwww
  mov r1.w, r2.w
  store_uav_typed u2.xyzw, r3.xyzw, r1.xyzw
  store_structured g0.x, vThreadIDInGroupFlattened.x, l(0), r2.x
  store_structured g1.x, vThreadIDInGroupFlattened.x, l(0), r2.y
  store_structured g2.x, vThreadIDInGroupFlattened.x, l(0), r2.z
  store_structured g3.x, vThreadIDInGroupFlattened.x, l(0), r1.w
endif 
ieq r0.x, cb0[0].w, l(3)
if_nz r0.x
  ret 
endif 
sync_g_t
if_z vThreadIDInGroupFlattened.x
  ld_structured r1.x, l(4), l(0), g0.xxxx
  ld_structured r1.y, l(4), l(0), g1.xxxx
  ld_structured r1.z, l(4), l(0), g2.xxxx
  ld_structured r1.w, l(4), l(0), g3.xxxx
  ld_structured r3.x, l(32), l(0), g0.xxxx
  ld_structured r3.y, l(32), l(0), g1.xxxx
  ld_structured r3.z, l(32), l(0), g2.xxxx
  ld_structured r3.w, l(32), l(0), g3.xxxx
  ld_structured r4.x, l(36), l(0), g0.xxxx
  ld_structured r4.y, l(36), l(0), g1.xxxx
  ld_structured r4.z, l(36), l(0), g2.xxxx
  ld_structured r4.w, l(36), l(0), g3.xxxx
  add r1.xyzw, r1.xyzw, r2.xyzw
  add r1.xyzw, r3.xyzw, r1.xyzw
  add r1.xyzw, r4.xyzw, r1.xyzw
  mul r2.w, r1.w, l(0.250000)
  ushr r3.xy, vThreadID.xyxx, l(3, 3, 0, 0)
  lt r0.xyz, r1.xyzx, l(0.012523, 0.012523, 0.012523, 0.000000)
  mul r4.xyz, r1.xyzx, l(3.230000, 3.230000, 3.230000, 0.000000)
  mad r5.xyz, r1.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), l(-0.002280, -0.002280, -0.002280, 0.000000)
  sqrt r5.xyz, |r5.xyzx|
  mul r1.xyz, r1.xyzx, l(0.033620, 0.033620, 0.033620, 0.000000)
  mad r1.xyz, r5.xyzx, l(1.130050, 1.130050, 1.130050, 0.000000), -r1.xyzx
  add r1.xyz, r1.xyzx, l(0.005719, 0.005719, 0.005719, 0.000000)
  movc r2.xyz, r0.xyzx, r4.xyzx, r1.xyzx
  mov r3.zw, r0.wwww
  store_uav_typed u3.xyzw, r3.xyzw, r2.xyzw
endif 
ret 
// Approximately 0 instruction slots used
#endif

const BYTE g_GenerateMips2DCS_sRGB_OddXY[] =
{
     68,  88,  66,  67, 164, 243, 
    150, 243, 131, 134,  47,  13, 
    154,  41, 230, 156, 206, 239, 
    245,  13,   1,   0,   0,   0, 
    252,  20,   0,   0,   4,   0, 
      0,   0,  48,   0,   0,   0, 
     64,   0,   0,   0,  80,   0, 
      0,   0,  56,  20,   0,   0, 
     73,  83,  71,  78,   8,   0, 
      0,   0,   0,   0,   0,   0, 
      8,   0,   0,   0,  79,  83, 
     71,  78,   8,   0,   0,   0, 
      0,   0,   0,   0,   8,   0, 
      0,   0,  83,  72,  69,  88, 
    224,  19,   0,   0,  80,   0, 
      5,   0, 248,   4,   0,   0, 
    106,   8,   0,   1,  89,   0, 
      0,   4,  70, 142,  32,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   0,  90,   0,   0,   3, 
      0,  96,  16,   0,   0,   0, 
      0,   0,  88,  64,   0,   4, 
      0, 112,  16,   0,   0,   0, 
      0,   0,  85,  85,   0,   0, 
    156,  64,   0,   4,   0, 224, 
     17,   0,   0,   0,   0,   0, 
     85,  85,   0,   0, 156,  64, 
      0,   4,   0, 224,  17,   0, 
      1,   0,   0,   0,  85,  85, 
      0,   0, 156,  64,   0,   4, 
      0, 224,  17,   0,   2,   0, 
      0,   0,  85,  85,   0,   0, 
    156,  64,   0,   4,   0, 224, 
     17,   0,   3,   0,   0,   0, 
     85,  85,   0,   0,  95,   0, 
      0,   2,   0,  64,   2,   0, 
     95,   0,   0,   2, 114,   0, 
      2,   0, 104,   0,   0,   2, 
      7,   0,   0,   0, 160,   0, 
      0,   5,   0, 240,  17,   0, 
      0,   0,   0,   0,   4,   0, 
      0,   0,  64,   0,   0,   0, 
    160,   0,   0,   5,   0, 240, 
     17,   0,   1,   0,   0,   0, 
      4,   0,   0,   0,  64,   0, 
      0,   0, 160,   0,   0,   5, 
      0, 240,  17,   0,   2,   0, 
      0,   0,   4,   0,   0,   0, 
     64,   0,   0,   0, 160,   0, 
      0,   5,   0, 240,  17,   0, 
      3,   0,   0,   0,   4,   0, 
      0,   0,  64,   0,   0,   0, 
    155,   0,   0,   4,   8,   0, 
      0,   0,   8,   0,   0,   0, 
      1,   0,   0,   0,  30,   0, 
      0,   7, 194,   0,  16,   0, 
      0,   0,   0,   0, 166,  10, 
      2,   0,   6, 128,  32,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,  86,   0,   0,   4, 
     50,   0,  16,   0,   1,   0, 
      0,   0,  70,   0,   2,   0, 
      0,   0,   0,  10,  50,   0, 
     16,   0,   1,   0,   0,   0, 
     70,   0,  16,   0,   1,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0, 128,  62,   0,   0, 
    128,  62,   0,   0,   0,   0, 
      0,   0,   0,   0,  56,   0, 
      0,   8,  50,   0,  16,   0, 
      1,   0,   0,   0,  70,   0, 
     16,   0,   1,   0,   0,   0, 
     70, 128,  32,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     86,   0,   0,   5,  66,   0, 
     16,   0,   1,   0,   0,   0, 
     58,   0,  16,   0,   0,   0, 
      0,   0,  86,   0,   0,   6, 
    130,   0,  16,   0,   1,   0, 
      0,   0,  42, 128,  32,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  72,   0,   0, 141, 
      2,   2,   0, 128,  67,  85, 
     21,   0, 242,   0,  16,   0, 
      2,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
     70, 126,  16,   0,   0,   0, 
      0,   0,   0,  96,  16,   0, 
      0,   0,   0,   0,  58,   0, 
     16,   0,   1,   0,   0,   0, 
     56,   0,   0,   8,  18,   0, 
     16,   0,   3,   0,   0,   0, 
     10, 128,  32,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,  63,  54,   0,   0,   8, 
     98,   0,  16,   0,   3,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   7, 114,   0,  16,   0, 
      3,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
     70,   2,  16,   0,   3,   0, 
      0,   0,  72,   0,   0, 141, 
      2,   2,   0, 128,  67,  85, 
     21,   0, 242,   0,  16,   0, 
      3,   0,   0,   0,  70,   2, 
     16,   0,   3,   0,   0,   0, 
     70, 126,  16,   0,   0,   0, 
      0,   0,   0,  96,  16,   0, 
      0,   0,   0,   0,  58,   0, 
     16,   0,   1,   0,   0,   0, 
      0,   0,   0,   7, 242,   0, 
     16,   0,   2,   0,   0,   0, 
     70,  14,  16,   0,   2,   0, 
      0,   0,  70,  14,  16,   0, 
      3,   0,   0,   0,  54,   0, 
      0,   8,  82,   0,  16,   0, 
      3,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     56,   0,   0,   8,  34,   0, 
     16,   0,   3,   0,   0,   0, 
     26, 128,  32,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,  63,   0,   0,   0,   7, 
    114,   0,  16,   0,   3,   0, 
      0,   0,  70,   2,  16,   0, 
      1,   0,   0,   0,  70,   2, 
     16,   0,   3,   0,   0,   0, 
     72,   0,   0, 141,   2,   2, 
      0, 128,  67,  85,  21,   0, 
    242,   0,  16,   0,   3,   0, 
      0,   0,  70,   2,  16,   0, 
      3,   0,   0,   0,  70, 126, 
     16,   0,   0,   0,   0,   0, 
      0,  96,  16,   0,   0,   0, 
      0,   0,  58,   0,  16,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   7, 242,   0,  16,   0, 
      2,   0,   0,   0,  70,  14, 
     16,   0,   2,   0,   0,   0, 
     70,  14,  16,   0,   3,   0, 
      0,   0,  56,   0,   0,  11, 
     50,   0,  16,   0,   3,   0, 
      0,   0,  70, 128,  32,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,  63,   0,   0, 
      0,  63,   0,   0,   0,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   5,  66,   0,  16,   0, 
      3,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   7, 114,   0, 
     16,   0,   1,   0,   0,   0, 
     70,   2,  16,   0,   1,   0, 
      0,   0,  70,   2,  16,   0, 
      3,   0,   0,   0,  72,   0, 
      0, 141,   2,   2,   0, 128, 
     67,  85,  21,   0, 242,   0, 
     16,   0,   1,   0,   0,   0, 
     70,   2,  16,   0,   1,   0, 
      0,   0,  70, 126,  16,   0, 
      0,   0,   0,   0,   0,  96, 
     16,   0,   0,   0,   0,   0, 
     58,   0,  16,   0,   1,   0, 
      0,   0,   0,   0,   0,   7, 
    242,   0,  16,   0,   1,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,  70,  14, 
     16,   0,   2,   0,   0,   0, 
     56,   0,   0,  10, 242,   0, 
     16,   0,   2,   0,   0,   0, 
     70,  14,  16,   0,   1,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0, 128,  62,   0,   0, 
    128,  62,   0,   0, 128,  62, 
      0,   0, 128,  62,  49,   0, 
      0,  10, 114,   0,  16,   0, 
      3,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
      2,  64,   0,   0,  28,  46, 
     77,  60,  28,  46,  77,  60, 
     28,  46,  77,  60,   0,   0, 
      0,   0,  56,   0,   0,  10, 
    114,   0,  16,   0,   4,   0, 
      0,   0,  70,   2,  16,   0, 
      1,   0,   0,   0,   2,  64, 
      0,   0,  82, 184,  78,  64, 
     82, 184,  78,  64,  82, 184, 
     78,  64,   0,   0,   0,   0, 
     50,   0,   0,  15, 114,   0, 
     16,   0,   5,   0,   0,   0, 
     70,   2,  16,   0,   1,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0, 128,  62,   0,   0, 
    128,  62,   0,   0, 128,  62, 
      0,   0,   0,   0,   2,  64, 
      0,   0,  13, 108,  21, 187, 
     13, 108,  21, 187,  13, 108, 
     21, 187,   0,   0,   0,   0, 
     75,   0,   0,   6, 114,   0, 
     16,   0,   5,   0,   0,   0, 
     70,   2,  16, 128, 129,   0, 
      0,   0,   5,   0,   0,   0, 
     56,   0,   0,  10, 114,   0, 
     16,   0,   6,   0,   0,   0, 
     70,   2,  16,   0,   1,   0, 
      0,   0,   2,  64,   0,   0, 
     32, 181,   9,  61,  32, 181, 
      9,  61,  32, 181,   9,  61, 
      0,   0,   0,   0,  50,   0, 
      0,  13, 114,   0,  16,   0, 
      5,   0,   0,   0,  70,   2, 
     16,   0,   5,   0,   0,   0, 
      2,  64,   0,   0, 122, 165, 
    144,  63, 122, 165, 144,  63, 
    122, 165, 144,  63,   0,   0, 
      0,   0,  70,   2,  16, 128, 
     65,   0,   0,   0,   6,   0, 
      0,   0,   0,   0,   0,  10, 
    114,   0,  16,   0,   5,   0, 
      0,   0,  70,   2,  16,   0, 
      5,   0,   0,   0,   2,  64, 
      0,   0, 115, 102, 187,  59, 
    115, 102, 187,  59, 115, 102, 
    187,  59,   0,   0,   0,   0, 
     55,   0,   0,   9, 114,   0, 
     16,   0,   3,   0,   0,   0, 
     70,   2,  16,   0,   3,   0, 
      0,   0,  70,   2,  16,   0, 
      4,   0,   0,   0,  70,   2, 
     16,   0,   5,   0,   0,   0, 
     54,   0,   0,   4,  50,   0, 
     16,   0,   0,   0,   0,   0, 
     70,   0,   2,   0,  54,   0, 
      0,   5, 130,   0,  16,   0, 
      3,   0,   0,   0,  58,   0, 
     16,   0,   2,   0,   0,   0, 
    164,   0,   0,   7, 242, 224, 
     17,   0,   0,   0,   0,   0, 
     70,  14,  16,   0,   0,   0, 
      0,   0,  70,  14,  16,   0, 
      3,   0,   0,   0,  32,   0, 
      0,   8,  18,   0,  16,   0, 
      0,   0,   0,   0,  58, 128, 
     32,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   1,   0,   0,   0, 
     31,   0,   4,   3,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     62,   0,   0,   1,  21,   0, 
      0,   1, 168,   0,   0,   8, 
     18, 240,  17,   0,   0,   0, 
      0,   0,  10,  64,   2,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      2,   0,   0,   0, 168,   0, 
      0,   8,  18, 240,  17,   0, 
      1,   0,   0,   0,  10,  64, 
      2,   0,   1,  64,   0,   0, 
      0,   0,   0,   0,  26,   0, 
     16,   0,   2,   0,   0,   0, 
    168,   0,   0,   8,  18, 240, 
     17,   0,   2,   0,   0,   0, 
     10,  64,   2,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
     42,   0,  16,   0,   2,   0, 
      0,   0, 168,   0,   0,   8, 
     18, 240,  17,   0,   3,   0, 
      0,   0,  10,  64,   2,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,  58,   0,  16,   0, 
      3,   0,   0,   0, 190,  24, 
      0,   1,   1,   0,   0,   6, 
     18,   0,  16,   0,   0,   0, 
      0,   0,  10,  64,   2,   0, 
      1,  64,   0,   0,   9,   0, 
      0,   0,  31,   0,   0,   3, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  30,   0,   0,   9, 
    114,   0,  16,   0,   0,   0, 
      0,   0,   6,  64,   2,   0, 
      2,  64,   0,   0,   1,   0, 
      0,   0,   8,   0,   0,   0, 
      9,   0,   0,   0,   0,   0, 
      0,   0, 167,   0,   0,   9, 
     18,   0,  16,   0,   3,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   0,   0, 
      0,   0, 167,   0,   0,   9, 
     34,   0,  16,   0,   3,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   1,   0, 
      0,   0, 167,   0,   0,   9, 
     66,   0,  16,   0,   3,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   2,   0, 
      0,   0, 167,   0,   0,   9, 
    130,   0,  16,   0,   3,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   3,   0, 
      0,   0, 167,   0,   0,   9, 
     18,   0,  16,   0,   4,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   0,   0, 
      0,   0, 167,   0,   0,   9, 
     34,   0,  16,   0,   4,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   1,   0, 
      0,   0, 167,   0,   0,   9, 
     66,   0,  16,   0,   4,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   2,   0, 
      0,   0, 167,   0,   0,   9, 
    130,   0,  16,   0,   4,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   3,   0, 
      0,   0, 167,   0,   0,   9, 
     18,   0,  16,   0,   5,   0, 
      0,   0,  42,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   0,   0, 
      0,   0, 167,   0,   0,   9, 
     34,   0,  16,   0,   5,   0, 
      0,   0,  42,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   1,   0, 
      0,   0, 167,   0,   0,   9, 
     66,   0,  16,   0,   5,   0, 
      0,   0,  42,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   2,   0, 
      0,   0, 167,   0,   0,   9, 
    130,   0,  16,   0,   5,   0, 
      0,   0,  42,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   3,   0, 
      0,   0,  50,   0,   0,  12, 
    242,   0,  16,   0,   1,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,   2,  64, 
      0,   0,   0,   0, 128,  62, 
      0,   0, 128,  62,   0,   0, 
    128,  62,   0,   0, 128,  62, 
     70,  14,  16,   0,   3,   0, 
      0,   0,   0,   0,   0,   7, 
    242,   0,  16,   0,   1,   0, 
      0,   0,  70,  14,  16,   0, 
      4,   0,   0,   0,  70,  14, 
     16,   0,   1,   0,   0,   0, 
      0,   0,   0,   7, 242,   0, 
     16,   0,   1,   0,   0,   0, 
     70,  14,  16,   0,   5,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,  56,   0, 
      0,  10, 242,   0,  16,   0, 
      2,   0,   0,   0,  70,  14, 
     16,   0,   1,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
    128,  62,   0,   0, 128,  62, 
      0,   0, 128,  62,   0,   0, 
    128,  62,  85,   0,   0,   9, 
     50,   0,  16,   0,   3,   0, 
      0,   0,  70,   0,   2,   0, 
      2,  64,   0,   0,   1,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  49,   0,   0,  10, 
    114,   0,  16,   0,   0,   0, 
      0,   0,  70,   2,  16,   0, 
      1,   0,   0,   0,   2,  64, 
      0,   0,  28,  46,  77,  60, 
     28,  46,  77,  60,  28,  46, 
     77,  60,   0,   0,   0,   0, 
     56,   0,   0,  10, 114,   0, 
     16,   0,   4,   0,   0,   0, 
     70,   2,  16,   0,   1,   0, 
      0,   0,   2,  64,   0,   0, 
     82, 184,  78,  64,  82, 184, 
     78,  64,  82, 184,  78,  64, 
      0,   0,   0,   0,  50,   0, 
      0,  15, 114,   0,  16,   0, 
      5,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
    128,  62,   0,   0, 128,  62, 
      0,   0, 128,  62,   0,   0, 
      0,   0,   2,  64,   0,   0, 
     13, 108,  21, 187,  13, 108, 
     21, 187,  13, 108,  21, 187, 
      0,   0,   0,   0,  75,   0, 
      0,   6, 114,   0,  16,   0, 
      5,   0,   0,   0,  70,   2, 
     16, 128, 129,   0,   0,   0, 
      5,   0,   0,   0,  56,   0, 
      0,  10, 114,   0,  16,   0, 
      1,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
      2,  64,   0,   0,  32, 181, 
      9,  61,  32, 181,   9,  61, 
     32, 181,   9,  61,   0,   0, 
      0,   0,  50,   0,   0,  13, 
    114,   0,  16,   0,   1,   0, 
      0,   0,  70,   2,  16,   0, 
      5,   0,   0,   0,   2,  64, 
      0,   0, 122, 165, 144,  63, 
    122, 165, 144,  63, 122, 165, 
    144,  63,   0,   0,   0,   0, 
     70,   2,  16, 128,  65,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,  10, 114,   0, 
     16,   0,   1,   0,   0,   0, 
     70,   2,  16,   0,   1,   0, 
      0,   0,   2,  64,   0,   0, 
    115, 102, 187,  59, 115, 102, 
    187,  59, 115, 102, 187,  59, 
      0,   0,   0,   0,  55,   0, 
      0,   9, 114,   0,  16,   0, 
      1,   0,   0,   0,  70,   2, 
     16,   0,   0,   0,   0,   0, 
     70,   2,  16,   0,   4,   0, 
      0,   0,  70,   2,  16,   0, 
      1,   0,   0,   0,  54,   0, 
      0,   5, 194,   0,  16,   0, 
      3,   0,   0,   0, 246,  15, 
     16,   0,   0,   0,   0,   0, 
     54,   0,   0,   5, 130,   0, 
     16,   0,   1,   0,   0,   0, 
     58,   0,  16,   0,   2,   0, 
      0,   0, 164,   0,   0,   7, 
    242, 224,  17,   0,   1,   0, 
      0,   0,  70,  14,  16,   0, 
      3,   0,   0,   0,  70,  14, 
     16,   0,   1,   0,   0,   0, 
    168,   0,   0,   8,  18, 240, 
     17,   0,   0,   0,   0,   0, 
     10,  64,   2,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   2,   0, 
      0,   0, 168,   0,   0,   8, 
     18, 240,  17,   0,   1,   0, 
      0,   0,  10,  64,   2,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,  26,   0,  16,   0, 
      2,   0,   0,   0, 168,   0, 
      0,   8,  18, 240,  17,   0, 
      2,   0,   0,   0,  10,  64, 
      2,   0,   1,  64,   0,   0, 
      0,   0,   0,   0,  42,   0, 
     16,   0,   2,   0,   0,   0, 
    168,   0,   0,   8,  18, 240, 
     17,   0,   3,   0,   0,   0, 
     10,  64,   2,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
     58,   0,  16,   0,   1,   0, 
      0,   0,  21,   0,   0,   1, 
     32,   0,   0,   8,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     58, 128,  32,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      1,  64,   0,   0,   2,   0, 
      0,   0,  31,   0,   4,   3, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  62,   0,   0,   1, 
     21,   0,   0,   1, 190,  24, 
      0,   1,   1,   0,   0,   6, 
     18,   0,  16,   0,   0,   0, 
      0,   0,  10,  64,   2,   0, 
      1,  64,   0,   0,  27,   0, 
      0,   0,  31,   0,   0,   3, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  30,   0,   0,   9, 
    114,   0,  16,   0,   0,   0, 
      0,   0,   6,  64,   2,   0, 
      2,  64,   0,   0,   2,   0, 
      0,   0,  16,   0,   0,   0, 
     18,   0,   0,   0,   0,   0, 
      0,   0, 167,   0,   0,   9, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   0,   0, 
      0,   0, 167,   0,   0,   9, 
     34,   0,  16,   0,   1,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   1,   0, 
      0,   0, 167,   0,   0,   9, 
     66,   0,  16,   0,   1,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   2,   0, 
      0,   0, 167,   0,   0,   9, 
    130,   0,  16,   0,   1,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   3,   0, 
      0,   0, 167,   0,   0,   9, 
     18,   0,  16,   0,   3,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   0,   0, 
      0,   0, 167,   0,   0,   9, 
     34,   0,  16,   0,   3,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   1,   0, 
      0,   0, 167,   0,   0,   9, 
     66,   0,  16,   0,   3,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   2,   0, 
      0,   0, 167,   0,   0,   9, 
    130,   0,  16,   0,   3,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   3,   0, 
      0,   0, 167,   0,   0,   9, 
     18,   0,  16,   0,   4,   0, 
      0,   0,  42,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   0,   0, 
      0,   0, 167,   0,   0,   9, 
     34,   0,  16,   0,   4,   0, 
      0,   0,  42,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   1,   0, 
      0,   0, 167,   0,   0,   9, 
     66,   0,  16,   0,   4,   0, 
      0,   0,  42,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   2,   0, 
      0,   0, 167,   0,   0,   9, 
    130,   0,  16,   0,   4,   0, 
      0,   0,  42,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   3,   0, 
      0,   0,   0,   0,   0,   7, 
    242,   0,  16,   0,   1,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,  70,  14, 
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
      0,   0,  56,   0,   0,  10, 
    242,   0,  16,   0,   2,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,   2,  64, 
      0,   0,   0,   0, 128,  62, 
      0,   0, 128,  62,   0,   0, 
    128,  62,   0,   0, 128,  62, 
     85,   0,   0,   9,  50,   0, 
     16,   0,   3,   0,   0,   0, 
     70,   0,   2,   0,   2,  64, 
      0,   0,   2,   0,   0,   0, 
      2,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     49,   0,   0,  10, 114,   0, 
     16,   0,   0,   0,   0,   0, 
     70,   2,  16,   0,   1,   0, 
      0,   0,   2,  64,   0,   0, 
     28,  46,  77,  60,  28,  46, 
     77,  60,  28,  46,  77,  60, 
      0,   0,   0,   0,  56,   0, 
      0,  10, 114,   0,  16,   0, 
      4,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
      2,  64,   0,   0,  82, 184, 
     78,  64,  82, 184,  78,  64, 
     82, 184,  78,  64,   0,   0, 
      0,   0,  50,   0,   0,  15, 
    114,   0,  16,   0,   5,   0, 
      0,   0,  70,   2,  16,   0, 
      1,   0,   0,   0,   2,  64, 
      0,   0,   0,   0, 128,  62, 
      0,   0, 128,  62,   0,   0, 
    128,  62,   0,   0,   0,   0, 
      2,  64,   0,   0,  13, 108, 
     21, 187,  13, 108,  21, 187, 
     13, 108,  21, 187,   0,   0, 
      0,   0,  75,   0,   0,   6, 
    114,   0,  16,   0,   5,   0, 
      0,   0,  70,   2,  16, 128, 
    129,   0,   0,   0,   5,   0, 
      0,   0,  56,   0,   0,  10, 
    114,   0,  16,   0,   1,   0, 
      0,   0,  70,   2,  16,   0, 
      1,   0,   0,   0,   2,  64, 
      0,   0,  32, 181,   9,  61, 
     32, 181,   9,  61,  32, 181, 
      9,  61,   0,   0,   0,   0, 
     50,   0,   0,  13, 114,   0, 
     16,   0,   1,   0,   0,   0, 
     70,   2,  16,   0,   5,   0, 
      0,   0,   2,  64,   0,   0, 
    122, 165, 144,  63, 122, 165, 
    144,  63, 122, 165, 144,  63, 
      0,   0,   0,   0,  70,   2, 
     16, 128,  65,   0,   0,   0, 
      1,   0,   0,   0,   0,   0, 
      0,  10, 114,   0,  16,   0, 
      1,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
      2,  64,   0,   0, 115, 102, 
    187,  59, 115, 102, 187,  59, 
    115, 102, 187,  59,   0,   0, 
      0,   0,  55,   0,   0,   9, 
    114,   0,  16,   0,   1,   0, 
      0,   0,  70,   2,  16,   0, 
      0,   0,   0,   0,  70,   2, 
     16,   0,   4,   0,   0,   0, 
     70,   2,  16,   0,   1,   0, 
      0,   0,  54,   0,   0,   5, 
    194,   0,  16,   0,   3,   0, 
      0,   0, 246,  15,  16,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   5, 130,   0,  16,   0, 
      1,   0,   0,   0,  58,   0, 
     16,   0,   2,   0,   0,   0, 
    164,   0,   0,   7, 242, 224, 
     17,   0,   2,   0,   0,   0, 
     70,  14,  16,   0,   3,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0, 168,   0, 
      0,   8,  18, 240,  17,   0, 
      0,   0,   0,   0,  10,  64, 
      2,   0,   1,  64,   0,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   2,   0,   0,   0, 
    168,   0,   0,   8,  18, 240, 
     17,   0,   1,   0,   0,   0, 
     10,  64,   2,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
     26,   0,  16,   0,   2,   0, 
      0,   0, 168,   0,   0,   8, 
     18, 240,  17,   0,   2,   0, 
      0,   0,  10,  64,   2,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,  42,   0,  16,   0, 
      2,   0,   0,   0, 168,   0, 
      0,   8,  18, 240,  17,   0, 
      3,   0,   0,   0,  10,  64, 
      2,   0,   1,  64,   0,   0, 
      0,   0,   0,   0,  58,   0, 
     16,   0,   1,   0,   0,   0, 
     21,   0,   0,   1,  32,   0, 
      0,   8,  18,   0,  16,   0, 
      0,   0,   0,   0,  58, 128, 
     32,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   3,   0,   0,   0, 
     31,   0,   4,   3,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     62,   0,   0,   1,  21,   0, 
      0,   1, 190,  24,   0,   1, 
     31,   0,   0,   2,  10,  64, 
      2,   0, 167,   0,   0,   9, 
     18,   0,  16,   0,   1,   0, 
      0,   0,   1,  64,   0,   0, 
      4,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   0,   0, 
      0,   0, 167,   0,   0,   9, 
     34,   0,  16,   0,   1,   0, 
      0,   0,   1,  64,   0,   0, 
      4,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   1,   0, 
      0,   0, 167,   0,   0,   9, 
     66,   0,  16,   0,   1,   0, 
      0,   0,   1,  64,   0,   0, 
      4,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   2,   0, 
      0,   0, 167,   0,   0,   9, 
    130,   0,  16,   0,   1,   0, 
      0,   0,   1,  64,   0,   0, 
      4,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   3,   0, 
      0,   0, 167,   0,   0,   9, 
     18,   0,  16,   0,   3,   0, 
      0,   0,   1,  64,   0,   0, 
     32,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   0,   0, 
      0,   0, 167,   0,   0,   9, 
     34,   0,  16,   0,   3,   0, 
      0,   0,   1,  64,   0,   0, 
     32,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   1,   0, 
      0,   0, 167,   0,   0,   9, 
     66,   0,  16,   0,   3,   0, 
      0,   0,   1,  64,   0,   0, 
     32,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   2,   0, 
      0,   0, 167,   0,   0,   9, 
    130,   0,  16,   0,   3,   0, 
      0,   0,   1,  64,   0,   0, 
     32,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   3,   0, 
      0,   0, 167,   0,   0,   9, 
     18,   0,  16,   0,   4,   0, 
      0,   0,   1,  64,   0,   0, 
     36,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   0,   0, 
      0,   0, 167,   0,   0,   9, 
     34,   0,  16,   0,   4,   0, 
      0,   0,   1,  64,   0,   0, 
     36,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   1,   0, 
      0,   0, 167,   0,   0,   9, 
     66,   0,  16,   0,   4,   0, 
      0,   0,   1,  64,   0,   0, 
     36,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   2,   0, 
      0,   0, 167,   0,   0,   9, 
    130,   0,  16,   0,   4,   0, 
      0,   0,   1,  64,   0,   0, 
     36,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 240,  17,   0,   3,   0, 
      0,   0,   0,   0,   0,   7, 
    242,   0,  16,   0,   1,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,  70,  14, 
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
      0,   0,  56,   0,   0,   7, 
    130,   0,  16,   0,   2,   0, 
      0,   0,  58,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0, 128,  62, 
     85,   0,   0,   9,  50,   0, 
     16,   0,   3,   0,   0,   0, 
     70,   0,   2,   0,   2,  64, 
      0,   0,   3,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     49,   0,   0,  10, 114,   0, 
     16,   0,   0,   0,   0,   0, 
     70,   2,  16,   0,   1,   0, 
      0,   0,   2,  64,   0,   0, 
     28,  46,  77,  60,  28,  46, 
     77,  60,  28,  46,  77,  60, 
      0,   0,   0,   0,  56,   0, 
      0,  10, 114,   0,  16,   0, 
      4,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
      2,  64,   0,   0,  82, 184, 
     78,  64,  82, 184,  78,  64, 
     82, 184,  78,  64,   0,   0, 
      0,   0,  50,   0,   0,  15, 
    114,   0,  16,   0,   5,   0, 
      0,   0,  70,   2,  16,   0, 
      1,   0,   0,   0,   2,  64, 
      0,   0,   0,   0, 128,  62, 
      0,   0, 128,  62,   0,   0, 
    128,  62,   0,   0,   0,   0, 
      2,  64,   0,   0,  13, 108, 
     21, 187,  13, 108,  21, 187, 
     13, 108,  21, 187,   0,   0, 
      0,   0,  75,   0,   0,   6, 
    114,   0,  16,   0,   5,   0, 
      0,   0,  70,   2,  16, 128, 
    129,   0,   0,   0,   5,   0, 
      0,   0,  56,   0,   0,  10, 
    114,   0,  16,   0,   1,   0, 
      0,   0,  70,   2,  16,   0, 
      1,   0,   0,   0,   2,  64, 
      0,   0,  32, 181,   9,  61, 
     32, 181,   9,  61,  32, 181, 
      9,  61,   0,   0,   0,   0, 
     50,   0,   0,  13, 114,   0, 
     16,   0,   1,   0,   0,   0, 
     70,   2,  16,   0,   5,   0, 
      0,   0,   2,  64,   0,   0, 
    122, 165, 144,  63, 122, 165, 
    144,  63, 122, 165, 144,  63, 
      0,   0,   0,   0,  70,   2, 
     16, 128,  65,   0,   0,   0, 
      1,   0,   0,   0,   0,   0, 
      0,  10, 114,   0,  16,   0, 
      1,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
      2,  64,   0,   0, 115, 102, 
    187,  59, 115, 102, 187,  59, 
    115, 102, 187,  59,   0,   0, 
      0,   0,  55,   0,   0,   9, 
    114,   0,  16,   0,   2,   0, 
      0,   0,  70,   2,  16,   0, 
      0,   0,   0,   0,  70,   2, 
     16,   0,   4,   0,   0,   0, 
     70,   2,  16,   0,   1,   0, 
      0,   0,  54,   0,   0,   5, 
    194,   0,  16,   0,   3,   0, 
      0,   0, 246,  15,  16,   0, 
      0,   0,   0,   0, 164,   0, 
      0,   7, 242, 224,  17,   0, 
      3,   0,   0,   0,  70,  14, 
     16,   0,   3,   0,   0,   0, 
     70,  14,  16,   0,   2,   0, 
      0,   0,  21,   0,   0,   1, 
     62,   0,   0,   1,  82,  84, 
     83,  48, 188,   0,   0,   0, 
      2,   0,   0,   0,   3,   0, 
      0,   0,  24,   0,   0,   0, 
      1,   0,   0,   0, 136,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,  60,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  72,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 104,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   5,   0,   0,   0, 
      1,   0,   0,   0,  80,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   1,   0,   0,   0, 
    112,   0,   0,   0,   1,   0, 
      0,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,  20,   0, 
      0,   0,   3,   0,   0,   0, 
      3,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
     16,   0,   0,   0,   4,   0, 
      0,   0,   2,   0,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    127, 127,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0
};
