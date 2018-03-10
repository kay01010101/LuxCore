#include <string>
namespace slg { namespace ocl {
std::string KernelSource_materialdefs_template_glossycoating = 
"#line 2 \"materialdefs_funcs_glossycoating.cl\"\n"
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
"// Glossycoating material\n"
"//\n"
"// One instance of this file for each Glossycoating material is used in Compiled scene\n"
"// class after having expanded the following parameters.\n"
"//\n"
"// Preprocessing parameters:\n"
"//  <<CS_GLOSSYCOATING_MATERIAL_INDEX>>\n"
"//  <<CS_MAT_BASE_MATERIAL_INDEX>>\n"
"//  <<CS_MAT_BASE_PREFIX>>_\n"
"//  <<CS_MAT_BASE_POSTFIX>>\n"
"//  <<CS_KS_TEXTURE>>\n"
"//  <<CS_NU_TEXTURE>>\n"
"//  <<CS_NV_TEXTURE>>\n"
"//  <<CS_KA_TEXTURE>>\n"
"//  <<CS_DEPTH_TEXTURE>>\n"
"//  <<CS_INDEX_TEXTURE>>\n"
"//  <<CS_MB_FLAG>>\n"
"//------------------------------------------------------------------------------\n"
"\n"
"OPENCL_FORCE_NOT_INLINE BSDFEvent Material_Index<<CS_GLOSSYCOATING_MATERIAL_INDEX>>_GetEventTypes(__global const Material* restrict material\n"
"		MATERIALS_PARAM_DECL) {\n"
"	return\n"
"			<<CS_MAT_BASE_PREFIX>>_GetEventTypes<<CS_MAT_BASE_POSTFIX>>(&mats[<<CS_MAT_BASE_MATERIAL_INDEX>>]\n"
"				MATERIALS_PARAM) |\n"
"			GLOSSY | REFLECT;\n"
"}\n"
"\n"
"OPENCL_FORCE_NOT_INLINE bool Material_Index<<CS_GLOSSYCOATING_MATERIAL_INDEX>>_IsDelta(__global const Material* restrict material\n"
"		MATERIALS_PARAM_DECL) {\n"
"	return false;\n"
"}\n"
"\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"OPENCL_FORCE_NOT_INLINE float3 Material_Index<<CS_GLOSSYCOATING_MATERIAL_INDEX>>_GetPassThroughTransparency(__global const Material* restrict material,\n"
"		__global HitPoint *hitPoint, const float3 localFixedDir, const float passThroughEvent\n"
"		MATERIALS_PARAM_DECL) {\n"
"	return <<CS_MAT_BASE_PREFIX>>_GetPassThroughTransparency<<CS_MAT_BASE_POSTFIX>>(&mats[<<CS_MAT_BASE_MATERIAL_INDEX>>],\n"
"			hitPoint, localFixedDir, passThroughEvent MATERIALS_PARAM);\n"
"}\n"
"#endif\n"
"\n"
"OPENCL_FORCE_NOT_INLINE float3 Material_Index<<CS_GLOSSYCOATING_MATERIAL_INDEX>>_Evaluate(__global const Material* restrict material,\n"
"		__global HitPoint *hitPoint, const float3 lightDir, const float3 eyeDir,\n"
"		BSDFEvent *event, float *directPdfW\n"
"		MATERIALS_PARAM_DECL) {\n"
"	const float3 fixedDir = eyeDir;\n"
"	const float3 sampledDir = lightDir;\n"
"\n"
"	// Note: this is the same side test used by matte translucent material and\n"
"	// it is different from the CPU test because HitPoint::dpdu and HitPoint::dpdv\n"
"	// are not available here without bump mapping.\n"
"	const float sideTest = CosTheta(lightDir) * CosTheta(eyeDir);\n"
"\n"
"	if (sideTest > DEFAULT_COS_EPSILON_STATIC) {\n"
"		// Reflection\n"
"\n"
"		const float3 lightDirBase = lightDir;\n"
"		const float3 eyeDirBase = eyeDir;\n"
"\n"
"		const float3 baseF = <<CS_MAT_BASE_PREFIX>>_Evaluate<<CS_MAT_BASE_POSTFIX>>(&mats[<<CS_MAT_BASE_MATERIAL_INDEX>>],\n"
"				hitPoint, lightDirBase, eyeDirBase, event, directPdfW MATERIALS_PARAM);\n"
"\n"
"		// Back face: no coating\n"
"		if (eyeDir.z <= 0.f)\n"
"			return baseF;\n"
"\n"
"		// Front face: coating+base\n"
"		*event |= GLOSSY | REFLECT;\n"
"\n"
"		float3 ks = <<CS_KS_TEXTURE>>;\n"
"#if defined(PARAM_ENABLE_MAT_GLOSSYCOATING_INDEX)\n"
"		const float i = <<CS_INDEX_TEXTURE>>;\n"
"		if (i > 0.f) {\n"
"			const float ti = (i - 1.f) / (i + 1.f);\n"
"			ks *= ti * ti;\n"
"		}\n"
"#endif\n"
"		ks = Spectrum_Clamp(ks);\n"
"\n"
"		const float u = clamp(<<CS_NU_TEXTURE>>, 1e-9f, 1.f);\n"
"#if defined(PARAM_ENABLE_MAT_GLOSSYCOATING_ANISOTROPIC)\n"
"		const float v = clamp(<<CS_NV_TEXTURE>>, 1e-9f, 1.f);\n"
"		const float u2 = u * u;\n"
"		const float v2 = v * v;\n"
"		const float anisotropy = (u2 < v2) ? (1.f - u2 / v2) : u2 > 0.f ? (v2 / u2 - 1.f) : 0.f;\n"
"		const float roughness = u * v;\n"
"#else\n"
"		const float anisotropy = 0.f;\n"
"		const float roughness = u * u;\n"
"#endif\n"
"\n"
"		if (directPdfW) {\n"
"			const float wCoating = SchlickBSDF_CoatingWeight(ks, fixedDir);\n"
"			const float wBase = 1.f - wCoating;\n"
"\n"
"			*directPdfW = wBase * *directPdfW +\n"
"				wCoating * SchlickBSDF_CoatingPdf(roughness, anisotropy, fixedDir, sampledDir);\n"
"		}\n"
"\n"
"#if defined(PARAM_ENABLE_MAT_GLOSSYCOATING_ABSORPTION)\n"
"		// Absorption\n"
"		const float cosi = fabs(sampledDir.z);\n"
"		const float coso = fabs(fixedDir.z);\n"
"\n"
"		const float3 alpha = Spectrum_Clamp(<<CS_KA_TEXTURE>>);\n"
"		const float3 absorption = CoatingAbsorption(cosi, coso, alpha, <<CS_DEPTH_TEXTURE>>);\n"
"#else\n"
"		const float3 absorption = WHITE;\n"
"#endif\n"
"\n"
"		// Coating fresnel factor\n"
"		const float3 H = normalize(fixedDir + sampledDir);\n"
"		const float3 S = FresnelSchlick_Evaluate(ks, fabs(dot(sampledDir, H)));\n"
"\n"
"#if defined(PARAM_ENABLE_MAT_GLOSSYCOATING_MULTIBOUNCE)\n"
"		const int multibounce = <<CS_MB_FLAG>>;\n"
"#else\n"
"		const int multibounce = 0;\n"
"#endif\n"
"		const float3 coatingF = SchlickBSDF_CoatingF(ks, roughness, anisotropy, multibounce,\n"
"				fixedDir, sampledDir);\n"
"\n"
"		// Blend in base layer Schlick style\n"
"		// assumes coating bxdf takes fresnel factor S into account\n"
"\n"
"		return coatingF + absorption * (WHITE - S) * baseF;\n"
"	} else if (sideTest < -DEFAULT_COS_EPSILON_STATIC) {\n"
"		const float3 lightDirBase = lightDir;\n"
"		const float3 eyeDirBase = eyeDir;\n"
"\n"
"		// Transmission\n"
"		const float3 baseF = <<CS_MAT_BASE_PREFIX>>_Evaluate<<CS_MAT_BASE_POSTFIX>>(&mats[<<CS_MAT_BASE_MATERIAL_INDEX>>],\n"
"				hitPoint, lightDirBase, eyeDirBase, event, directPdfW MATERIALS_PARAM);\n"
"\n"
"		float3 ks = <<CS_KS_TEXTURE>>;\n"
"#if defined(PARAM_ENABLE_MAT_GLOSSYCOATING_INDEX)\n"
"		const float i = <<CS_INDEX_TEXTURE>>;\n"
"		if (i > 0.f) {\n"
"			const float ti = (i - 1.f) / (i + 1.f);\n"
"			ks *= ti * ti;\n"
"		}\n"
"#endif\n"
"		ks = Spectrum_Clamp(ks);\n"
"\n"
"		if (directPdfW) {\n"
"			const float3 fixedDir = eyeDir;\n"
"			const float wCoating = fixedDir.z > 0.f ? SchlickBSDF_CoatingWeight(ks, fixedDir) : 0.f;\n"
"			const float wBase = 1.f - wCoating;\n"
"\n"
"			*directPdfW *= wBase;\n"
"		}\n"
"\n"
"#if defined(PARAM_ENABLE_MAT_GLOSSYCOATING_ABSORPTION)\n"
"		// Absorption\n"
"		const float cosi = fabs(sampledDir.z);\n"
"		const float coso = fabs(fixedDir.z);\n"
"\n"
"		const float3 alpha = Spectrum_Clamp(<<CS_KA_TEXTURE>>);\n"
"		const float3 absorption = CoatingAbsorption(cosi, coso, alpha, <<CS_DEPTH_TEXTURE>>);\n"
"#else\n"
"		const float3 absorption = WHITE;\n"
"#endif\n"
"\n"
"		// Coating fresnel factor\n"
"		const float3 H = normalize((float3)(sampledDir.x + fixedDir.x, sampledDir.y + fixedDir.y,\n"
"			sampledDir.z - fixedDir.z));\n"
"		const float3 S = FresnelSchlick_Evaluate(ks, fabs(dot(fixedDir, H)));\n"
"\n"
"		// filter base layer, the square root is just a heuristic\n"
"		// so that a sheet coated on both faces gets a filtering factor\n"
"		// of 1-S like a reflection\n"
"		return absorption * Spectrum_Sqrt(WHITE - S) * baseF;\n"
"	} else\n"
"		return BLACK;\n"
"}\n"
"\n"
"OPENCL_FORCE_NOT_INLINE float3 Material_Index<<CS_GLOSSYCOATING_MATERIAL_INDEX>>_Sample(__global const Material* restrict material,\n"
"		__global HitPoint *hitPoint, const float3 fixedDir, float3 *sampledDir, const float u0, const float u1,\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		const float passThroughEvent,\n"
"#endif\n"
"		float *pdfW, float *cosSampledDir, BSDFEvent *event\n"
"		MATERIALS_PARAM_DECL) {\n"
"	float3 ks = <<CS_KS_TEXTURE>>;\n"
"#if defined(PARAM_ENABLE_MAT_GLOSSYCOATING_INDEX)\n"
"	const float i = <<CS_INDEX_TEXTURE>>;\n"
"	if (i > 0.f) {\n"
"		const float ti = (i - 1.f) / (i + 1.f);\n"
"		ks *= ti * ti;\n"
"	}\n"
"#endif\n"
"	ks = Spectrum_Clamp(ks);\n"
"\n"
"	// Coating is used only on the front face\n"
"	const float wCoating = SchlickBSDF_CoatingWeight(ks, fixedDir);\n"
"	const float wBase = 1.f - wCoating;\n"
"\n"
"	const float u = clamp(<<CS_NU_TEXTURE>>, 1e-9f, 1.f);\n"
"#if defined(PARAM_ENABLE_MAT_GLOSSYCOATING_ANISOTROPIC)\n"
"	const float v = clamp(<<CS_NU_TEXTURE>>, 1e-9f, 1.f);\n"
"	const float u2 = u * u;\n"
"	const float v2 = v * v;\n"
"	const float anisotropy = (u2 < v2) ? (1.f - u2 / v2) : u2 > 0.f ? (v2 / u2 - 1.f) : 0.f;\n"
"	const float roughness = u * v;\n"
"#else\n"
"	const float anisotropy = 0.f;\n"
"	const float roughness = u * u;\n"
"#endif\n"
"\n"
"#if defined(PARAM_ENABLE_MAT_GLOSSYCOATING_MULTIBOUNCE)\n"
"	const int multibounce = <<CS_MB_FLAG>>;\n"
"#else\n"
"	const int multibounce = 0;\n"
"#endif\n"
"\n"
"	float basePdf, coatingPdf;\n"
"	float3 baseF, coatingF;\n"
"	if (passThroughEvent < wBase) {\n"
"		const float3 fixedDirBase = fixedDir;\n"
"\n"
"		// Sample base BSDF\n"
"		baseF = <<CS_MAT_BASE_PREFIX>>_Sample<<CS_MAT_BASE_POSTFIX>>(&mats[<<CS_MAT_BASE_MATERIAL_INDEX>>],\n"
"			hitPoint, fixedDirBase, sampledDir, u0, u1,\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"				passThroughEvent / wBase,\n"
"#endif\n"
"				&basePdf, cosSampledDir, event MATERIALS_PARAM);\n"
"\n"
"		if (Spectrum_IsBlack(baseF))\n"
"			return BLACK;\n"
"\n"
"		baseF *= basePdf;\n"
"\n"
"		// Don't add the coating scattering if the base sampled\n"
"		// component is specular\n"
"		if (!(*event & SPECULAR)) {\n"
"			coatingF = SchlickBSDF_CoatingF(ks, roughness, anisotropy, multibounce,\n"
"					fixedDir, *sampledDir);\n"
"			coatingPdf = SchlickBSDF_CoatingPdf(roughness, anisotropy, fixedDir, *sampledDir);\n"
"		} else\n"
"			coatingPdf = 0.f;\n"
"	} else {\n"
"		// Sample coating BSDF (Schlick BSDF)\n"
"		coatingF = SchlickBSDF_CoatingSampleF(ks, roughness, anisotropy,\n"
"				multibounce, fixedDir, sampledDir, u0, u1, &coatingPdf);\n"
"		if (Spectrum_IsBlack(coatingF))\n"
"			return BLACK;\n"
"\n"
"		*cosSampledDir = fabs((*sampledDir).z);\n"
"		if (*cosSampledDir < DEFAULT_COS_EPSILON_STATIC)\n"
"			return BLACK;\n"
"\n"
"		coatingF *= coatingPdf;\n"
"\n"
"		// Evaluate base BSDF\n"
"		const float3 lightDirBase = *sampledDir;\n"
"		const float3 eyeDirBase = fixedDir;\n"
"\n"
"		baseF = <<CS_MAT_BASE_PREFIX>>_Evaluate<<CS_MAT_BASE_POSTFIX>>(&mats[<<CS_MAT_BASE_MATERIAL_INDEX>>],\n"
"				hitPoint, lightDirBase, eyeDirBase, event, &basePdf MATERIALS_PARAM);\n"
"		*event = GLOSSY | REFLECT;\n"
"	}\n"
"\n"
"	*pdfW = coatingPdf * wCoating + basePdf * wBase;\n"
"\n"
"#if defined(PARAM_ENABLE_MAT_GLOSSYCOATING_ABSORPTION)\n"
"	// Absorption\n"
"	const float cosi = fabs((*sampledDir).z);\n"
"	const float coso = fabs(fixedDir.z);\n"
"\n"
"	const float3 alpha = Spectrum_Clamp(<<CS_KA_TEXTURE>>);\n"
"	const float3 absorption = CoatingAbsorption(cosi, coso, alpha, <<CS_DEPTH_TEXTURE>>);\n"
"#else\n"
"	const float3 absorption = WHITE;\n"
"#endif\n"
"\n"
"	// Note: this is the same side test used by matte translucent material and\n"
"	// it is different from the CPU test because HitPoint::dpdu and HitPoint::dpdv\n"
"	// are not available here without bump mapping.\n"
"	const float sideTest = CosTheta(fixedDir) * CosTheta(*sampledDir);\n"
"	if (sideTest > DEFAULT_COS_EPSILON_STATIC) {\n"
"		// Reflection\n"
"\n"
"		if (!(fixedDir.z > 0.f)) {\n"
"			// Back face reflection: no coating\n"
"			return baseF / basePdf;\n"
"		} else {\n"
"			// Front face reflection: coating+base\n"
"\n"
"			// Coating fresnel factor\n"
"			const float3 H = normalize(fixedDir + *sampledDir);\n"
"			const float3 S = FresnelSchlick_Evaluate(ks, fabs(dot(*sampledDir, H)));\n"
"\n"
"			// blend in base layer Schlick style\n"
"			// coatingF already takes fresnel factor S into account\n"
"			return (coatingF + absorption * (WHITE - S) * baseF) / *pdfW;\n"
"		}\n"
"	} else if (sideTest < -DEFAULT_COS_EPSILON_STATIC) {\n"
"		// Transmission\n"
"		// Coating fresnel factor\n"
"		const float3 H = normalize((float3)((*sampledDir).x + fixedDir.x, (*sampledDir).y + fixedDir.y,\n"
"			(*sampledDir).z - fixedDir.z));\n"
"		const float3 S = FresnelSchlick_Evaluate(ks, fabs(dot(fixedDir, H)));\n"
"\n"
"		// filter base layer, the square root is just a heuristic\n"
"		// so that a sheet coated on both faces gets a filtering factor\n"
"		// of 1-S like a reflection\n"
"		return absorption * Spectrum_Sqrt(WHITE - S) * baseF / *pdfW;\n"
"	} else\n"
"		return BLACK;\n"
"}\n"
"\n"
"float3 Material_Index<<CS_GLOSSYCOATING_MATERIAL_INDEX>>_GetEmittedRadiance(__global const Material* restrict material,\n"
"		__global HitPoint *hitPoint\n"
"		MATERIALS_PARAM_DECL) {\n"
"	if (material->emitTexIndex != NULL_INDEX)\n"
"		return Material_GetEmittedRadianceWithoutDynamic(material, hitPoint MATERIALS_PARAM);\n"
"	else\n"
"		return <<CS_MAT_BASE_PREFIX>>_GetEmittedRadiance<<CS_MAT_BASE_POSTFIX>>(&mats[<<CS_MAT_BASE_MATERIAL_INDEX>>],\n"
"				   hitPoint\n"
"				   MATERIALS_PARAM);\n"
"}\n"
"\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"OPENCL_FORCE_NOT_INLINE uint Material_Index<<CS_GLOSSYCOATING_MATERIAL_INDEX>>_GetInteriorVolume(__global const Material* restrict material,\n"
"		__global HitPoint *hitPoint, const float passThroughEvent\n"
"		MATERIALS_PARAM_DECL) {\n"
"		if (material->interiorVolumeIndex != NULL_INDEX)\n"
"			return material->interiorVolumeIndex;\n"
"		else\n"
"			return <<CS_MAT_BASE_PREFIX>>_GetInteriorVolume<<CS_MAT_BASE_POSTFIX>>(&mats[<<CS_MAT_BASE_MATERIAL_INDEX>>],\n"
"				hitPoint, passThroughEvent\n"
"				MATERIALS_PARAM);\n"
"}\n"
"\n"
"OPENCL_FORCE_NOT_INLINE uint Material_Index<<CS_GLOSSYCOATING_MATERIAL_INDEX>>_GetExteriorVolume(__global const Material* restrict material,\n"
"		__global HitPoint *hitPoint, const float passThroughEvent\n"
"		MATERIALS_PARAM_DECL) {\n"
"		if (material->exteriorVolumeIndex != NULL_INDEX)\n"
"			return material->exteriorVolumeIndex;\n"
"		else\n"
"			return <<CS_MAT_BASE_PREFIX>>_GetExteriorVolume<<CS_MAT_BASE_POSTFIX>>(&mats[<<CS_MAT_BASE_MATERIAL_INDEX>>],\n"
"					hitPoint, passThroughEvent\n"
"					MATERIALS_PARAM);\n"
"}\n"
"#endif\n"
; } }
