#include <string>
namespace slg { namespace ocl {
std::string KernelSource_materialdefs_funcs_mirror = 
"#line 2 \"materialdefs_funcs_mirror.cl\"\n"
"\n"
"/***************************************************************************\n"
" * Copyright 1998-2018 by authors (see AUTHORS.txt)                        *\n"
" *                                                                         *\n"
" *   This file is part of LuxCoreRender.                                   *\n"
" *                                                                         *\n"
" * Licensed under the Apache License, Version 2.0 (the \"License\");         *\n"
" * you may not use this file except in compliance with the License.        *\n"
" * You may obtain a copy of the License at                                 *\n"
" *                                                                         *\n"
" *     http://www.apache.org/licenses/LICENSE-2.0                          *\n"
" *                                                                         *\n"
" * Unless required by applicable law or agreed to in writing, software     *\n"
" * distributed under the License is distributed on an \"AS IS\" BASIS,       *\n"
" * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*\n"
" * See the License for the specific language governing permissions and     *\n"
" * limitations under the License.                                          *\n"
" ***************************************************************************/\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Mirror material\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined (PARAM_ENABLE_MAT_MIRROR)\n"
"\n"
"OPENCL_FORCE_INLINE BSDFEvent MirrorMaterial_GetEventTypes() {\n"
"	return SPECULAR | REFLECT;\n"
"}\n"
"\n"
"OPENCL_FORCE_INLINE bool MirrorMaterial_IsDelta() {\n"
"	return true;\n"
"}\n"
"\n"
"OPENCL_FORCE_INLINE float3 MirrorMaterial_Evaluate(\n"
"		__global HitPoint *hitPoint, const float3 lightDir, const float3 eyeDir,\n"
"		BSDFEvent *event, float *directPdfW,\n"
"		const float3 krVal) {\n"
"	return BLACK;\n"
"}\n"
"\n"
"OPENCL_FORCE_INLINE float3 MirrorMaterial_Sample(\n"
"		__global HitPoint *hitPoint, const float3 fixedDir, float3 *sampledDir,\n"
"		const float u0, const float u1,\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		const float passThroughEvent,\n"
"#endif\n"
"		float *pdfW, float *cosSampledDir, BSDFEvent *event,\n"
"		const float3 krVal) {\n"
"	*event = SPECULAR | REFLECT;\n"
"\n"
"	*sampledDir = (float3)(-fixedDir.x, -fixedDir.y, fixedDir.z);\n"
"	*pdfW = 1.f;\n"
"\n"
"	*cosSampledDir = fabs((*sampledDir).z);\n"
"\n"
"	return Spectrum_Clamp(krVal);\n"
"}\n"
"\n"
"#endif\n"
; } }
