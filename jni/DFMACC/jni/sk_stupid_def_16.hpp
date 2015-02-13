/*
 * Copyright (C) 2015 zheng qian <xqq@0ginr.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if !defined(_SK_STUPID_DEF_16_HPP) && defined(TAG_SK_STUPID_RENDERER_16)
#define _SK_STUPID_DEF_16_HPP

#include <cstdint>
#include <cstring>
#include <sk_stupid_common_def.hpp>

#define SK_GR_GL_FRAMEBUFFER_BINDING  0x8CA6

enum GrEngine_t {
    kOpenGL_Shaders_GrEngine,
    kOpenGL_Fixed_GrEngine,
};

enum GrPixelConfig_t {
    kUnknown_GrPixelConfig,
    kAlpha_8_GrPixelConfig,
    kIndex_8_GrPixelConfig,
    kRGB_565_GrPixelConfig,
    kRGBA_4444_GrPixelConfig,      // Premultiplied
    kRGBA_8888_PM_GrPixelConfig,   // Premultiplied. Byte order is r,g,b,a
    kRGBA_8888_UPM_GrPixelConfig,  // Unpremultiplied. Byte order is r,g,b,a
    kBGRA_8888_PM_GrPixelConfig,   // Premultiplied. Byte order is b,g,r,a
    kBGRA_8888_UPM_GrPixelConfig,  // Unpremultiplied. Byte order is b,g,r,a

    kGrPixelConfigCount
};

static const GrPixelConfig_t kSkia8888_PM_GrPixelConfig = kRGBA_8888_PM_GrPixelConfig;

typedef intptr_t GrPlatform3DObject_t;
typedef intptr_t GrPlatform3DContext_t;

struct GrPlatformRenderTargetDesc_t {
	GrPlatformRenderTargetDesc_t() {
		memset(this, 0, sizeof(*this));
	}

    int                    width;              // width in pixels
    int                    height;             // height in pixels
    GrPixelConfig_t        config;             // color format
    int                    sampleCount;        // MSAA sample count while > 0
    int                    stencilBits;        // number of bits of stencil per-pixel
    GrPlatform3DObject_t   renderTargetHandle; // handle to the 3D API object
};

// Function pointer declares for GrGLInterface::functions()
typedef GrGLInterface_t* (*Func_GrGLCreateNativeInterface)();

// Function pointer declares for GrContext::functions()
typedef GrContext_t* (*Func_Create)(GrEngine_t, GrPlatform3DContext_t);
typedef void (*Func_ContextDestroyed)(GrContext_t* context);
typedef GrRenderTarget_t* (*Func_CreatePlatformRenderTarget)(GrContext_t* context, GrPlatformRenderTargetDesc_t* desc);
typedef void (*Func_Flush)(GrContext_t* context, int flagsBitfield);
typedef void (*Func_GrContext_Dtor)(GrContext_t* context);

//typedef void (*Func_GrRenderTarget_Dtor)(GrRenderTarget_t* renderTarget);

// Function pointer declares for SkGpuDevice
typedef void (*Func_SkGpuDeviceCtor)(SkGpuDevice_t* gpuDevice, GrContext_t* context, GrRenderTarget_t* renderTarget);
typedef void (*Func_SkGpuDeviceDtor)(SkGpuDevice_t* gpuDevice);

// Function pointer declares for SkCanvas
typedef void (*Func_SkCanvasCtor)(SkCanvas_t* skcanvas, SkGpuDevice_t* skdevice);
typedef void (*Func_SkCanvasDtor)(SkCanvas_t* skcanvas);


static const size_t sizeof_SkGpuDevice = 96;
static const size_t sizeof_SkGpuDevice_16 = 0;
static const size_t sizeof_SkGpuDevice_17 = 88;

static const size_t sizeof_SkCanvas = 384;
static const size_t sizeof_SkCanvas_16 = 0;
static const size_t sizeof_SkCanvas_17 = 324;


static const char* symbol_GrGLCreateNativeInterface = "_Z25GrGLCreateNativeInterfacev";

static const char* symbol_Create = "_ZN9GrContext6CreateE8GrEnginei";
static const char* symbol_ContextDestroyed = "_ZN9GrContext16contextDestroyedEv";
static const char* symbol_CreatePlatformRenderTarget = "_ZN9GrContext26createPlatformRenderTargetERK26GrPlatformRenderTargetDesc";
static const char* symbol_Flush = "_ZN9GrContext5flushEi";

static const char* symbol_GrContext_Dtor = "_ZN9GrContextD1Ev";
//static const char* symbol_GrRenderTarget_Dtor = "_ZN10GrResourceD1Ev"; //"_ZN14GrRenderTargetD1Ev";

static const char* symbol_SkGpuDevice_Ctor = "_ZN11SkGpuDeviceC1EP9GrContextP14GrRenderTarget";
static const char* symbol_SkGpuDevice_Dtor = "_ZN11SkGpuDeviceD1Ev";

static const char* symbol_SkCanvas_Ctor = "_ZN8SkCanvasC1EP8SkDevice";
static const char* symbol_SkCanvas_Dtor = "_ZN8SkCanvasD1Ev";


struct GrGLInterface_Symbol_t {
    Func_GrGLCreateNativeInterface  GrGLCreateNativeInterface = nullptr;
};

struct GrContext_Symbol_t {
    Func_Create                     Create = nullptr;
    Func_ContextDestroyed           contextDestroyed = nullptr;
    Func_CreatePlatformRenderTarget createPlatformRenderTarget = nullptr;
    Func_Flush                      flush = nullptr;
    Func_GrContext_Dtor             Dtor = nullptr;
};

struct GrRenderTarget_Symbol_t {
    //Func_GrRenderTarget_Dtor     Dtor = nullptr;
};

struct SkGpuDevice_Symbol_t {
    Func_SkGpuDeviceCtor         Ctor = nullptr;
    Func_SkGpuDeviceDtor         Dtor = nullptr;
};

struct SkCanvas_Symbol_t {
    Func_SkCanvasCtor            Ctor = nullptr;
    Func_SkCanvasDtor            Dtor = nullptr;
};

#endif // _SK_STUPID_DEF_16_HPP
