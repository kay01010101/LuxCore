#include <string>
namespace luxrays { namespace ocl {
std::string KernelSource_motionsystem_types = 
"#line 2 \"motionsystem_types.cl\"\n"
"\n"
"/***************************************************************************\n"
" * Copyright 1998-2013 by authors (see AUTHORS.txt)                        *\n"
" *                                                                         *\n"
" *   This file is part of LuxRender.                                       *\n"
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
"typedef struct {\n"
"	// Scaling\n"
"	float Sx, Sy, Sz;\n"
"	// Shearing\n"
"	float Sxy, Sxz, Syz;\n"
"	// Rotation\n"
"	Matrix4x4 R;\n"
"	// Translation\n"
"	float Tx, Ty, Tz;\n"
"	// Perspective\n"
"	float Px, Py, Pz, Pw;\n"
"	// Represents a valid series of transformations\n"
"	bool Valid;\n"
"} DecomposedTransform;\n"
"\n"
"typedef struct {\n"
"	float startTime, endTime;\n"
"	Transform start, end;\n"
"	DecomposedTransform startT, endT;\n"
"	Quaternion startQ, endQ;\n"
"	int hasRotation, hasTranslation, hasScale;\n"
"	int hasTranslationX, hasTranslationY, hasTranslationZ;\n"
"	int hasScaleX, hasScaleY, hasScaleZ;\n"
"	// false if start and end transformations are identical\n"
"	int isActive;\n"
"} InterpolatedTransform;\n"
"\n"
"typedef struct {\n"
"	unsigned int interpolatedTransformFirstIndex;\n"
"	unsigned int interpolatedTransformLastIndex;\n"
"} MotionSystem;\n"
; } }