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

#define TAG_SK_STUPID_RENDERER_16 16

#include <malloc.h>
#include <dlfcn.h>
#include <GLES2/gl2.h>
#include <android/log.h>
#include "version_utils.hpp"
#include "sk_stupid_def_16.hpp"
#include "sk_stupid_renderer_16.hpp"

static GrGLInterface_Symbol_t  GrGLInterface_Symbol;
static GrContext_Symbol_t      GrContext_Symbol;
//static GrRenderTarget_Symbol_t GrRenderTarget_Symbol;
static SkGpuDevice_Symbol_t    SkGpuDevice_Symbol;
static SkCanvas_Symbol_t       SkCanvas_Symbol;

bool SkStupidRenderer_16::supportApi(int api) {
    return (api >= minSdkVersion) && (api <= maxSdkVersion);
}

SkStupidRenderer_16::SkStupidRenderer_16(void* nativeHandle) {
    mApiLevel = getDeviceApiLevel();
    mAndroidVersion = getDeviceAndroidVersion();
    loadSymbols();
}

SkStupidRenderer_16::~SkStupidRenderer_16() {
    if (hasBackend) {
        teardownBackend();
    }

    if (mLibSkiaGpu) {
        dlclose(mLibSkiaGpu);
        mLibSkiaGpu = nullptr;
    }

    if (mLibSkia) {
        dlclose(mLibSkia);
        mLibSkia = nullptr;
    }
}

void SkStupidRenderer_16::setExtraData(void* data) {
    mExtraData = data;
}

void* SkStupidRenderer_16::getExtraData() {
    return mExtraData;
}

void SkStupidRenderer_16::loadSymbols() {
    if (this->supportApi(mApiLevel) == false) {
        mSymbolsLoaded = false; mSymbolsComplete = false;
        return;
    }

    mLibSkia = dlopen("libskia.so", RTLD_NOW | RTLD_LOCAL);
    if (mLibSkia == nullptr) {
        mSymbolsLoaded = false; mSymbolsComplete = false;
        __android_log_print(ANDROID_LOG_ERROR, "SkStupidRenderer_16", "Open libskia.so failed");
        return;
    }

    mLibSkiaGpu = dlopen("libskiagpu.so", RTLD_NOW | RTLD_LOCAL);
    if (mLibSkiaGpu == nullptr) {
        mSymbolsLoaded = false; mSymbolsComplete = false;
        __android_log_print(ANDROID_LOG_ERROR, "SkStupidRenderer_16", "Open libskiagpu.so failed");
        return;
    }

    GrGLInterface_Symbol.GrGLCreateNativeInterface = (Func_GrGLCreateNativeInterface)dlsym(mLibSkiaGpu, symbol_GrGLCreateNativeInterface);

    GrContext_Symbol.Create = (Func_Create)dlsym(mLibSkiaGpu, symbol_Create);
    GrContext_Symbol.contextDestroyed = (Func_ContextDestroyed)dlsym(mLibSkiaGpu, symbol_ContextDestroyed);
    GrContext_Symbol.createPlatformRenderTarget = (Func_CreatePlatformRenderTarget)dlsym(mLibSkiaGpu, symbol_CreatePlatformRenderTarget);
    GrContext_Symbol.flush = (Func_Flush)dlsym(mLibSkiaGpu, symbol_Flush);
    GrContext_Symbol.Dtor = (Func_GrContext_Dtor)dlsym(mLibSkiaGpu, symbol_GrContext_Dtor);

    SkGpuDevice_Symbol.Ctor = (Func_SkGpuDeviceCtor)dlsym(mLibSkiaGpu, symbol_SkGpuDevice_Ctor);
    SkGpuDevice_Symbol.Dtor = (Func_SkGpuDeviceDtor)dlsym(mLibSkiaGpu, symbol_SkGpuDevice_Dtor);

    SkCanvas_Symbol.Ctor = (Func_SkCanvasCtor)dlsym(mLibSkia, symbol_SkCanvas_Ctor);
    SkCanvas_Symbol.Dtor = (Func_SkCanvasDtor)dlsym(mLibSkia, symbol_SkCanvas_Dtor);

    mSymbolsLoaded = true;
    mSymbolsComplete = checkSymbols();
}

bool SkStupidRenderer_16::checkSymbols() {
    if (mSymbolsLoaded) {
        if (GrGLInterface_Symbol.GrGLCreateNativeInterface &&
            GrContext_Symbol.Create &&
            GrContext_Symbol.contextDestroyed &&
            GrContext_Symbol.createPlatformRenderTarget &&
            GrContext_Symbol.flush &&
            GrContext_Symbol.Dtor &&
            SkGpuDevice_Symbol.Ctor &&
            SkGpuDevice_Symbol.Dtor &&
            SkCanvas_Symbol.Ctor &&
            SkCanvas_Symbol.Dtor) {
            return true;
            __android_log_print(ANDROID_LOG_WARN, "SkStupidRenderer_16", "Symbols complete!");
        } else {
            __android_log_print(ANDROID_LOG_WARN, "SkStupidRenderer_16", "Symbol miss: %p,%p,%p,%p,%p,%p,%p,%p,%p,%p",
                    GrGLInterface_Symbol.GrGLCreateNativeInterface, GrContext_Symbol.Create, GrContext_Symbol.contextDestroyed,
                    GrContext_Symbol.createPlatformRenderTarget, GrContext_Symbol.flush, GrContext_Symbol.Dtor,
                    SkGpuDevice_Symbol.Ctor, SkGpuDevice_Symbol.Dtor, SkCanvas_Symbol.Ctor, SkCanvas_Symbol.Dtor);
            dlclose(mLibSkiaGpu);
            mLibSkiaGpu = nullptr;
            dlclose(mLibSkia);
            mLibSkia = nullptr;
        }
    }
    return false;
}

bool SkStupidRenderer_16::isDeviceSupported() {
    return mSymbolsLoaded && mSymbolsComplete;
}

bool SkStupidRenderer_16::setupBackend(int width, int height, int msaaSampleCount) {
    if (mSymbolsLoaded == false || mSymbolsComplete == false) {
        return false;
    }

    DbgAssert(hasBackend == false);
    DbgAssert(mContext == nullptr);
    DbgAssert(mInterface == nullptr);
    DbgAssert(mRenderTarget == nullptr);

    mMSAASampleCount = msaaSampleCount;
    mInterface = GrGLInterface_Symbol.GrGLCreateNativeInterface();
    mContext = GrContext_Symbol.Create(GrEngine_t::kOpenGL_Shaders_GrEngine, (GrPlatform3DContext_t)mInterface);

    hasBackend = true;

    if (mContext == nullptr || mInterface == nullptr) {
        __android_log_print(ANDROID_LOG_ERROR, "SkStupidRenderer_16",
                "Fail to setup GrContext/GrRenderTarget: %p, %p", mContext, mInterface);
        Sk_SafeUnref(mContext, (void*)GrContext_Symbol.Dtor);
        Sk_SafeUnref(mInterface);
        mContext = nullptr;
        mInterface = nullptr;
        hasBackend = false;

        return false;
    }

    this->mWidth = width;
    this->mHeight = height;
    this->windowSizeChanged();

    return true;
}

bool SkStupidRenderer_16::teardownBackend() {
    if (mCanvas) {
        mCanvas->unref((void*)SkCanvas_Symbol.Dtor);
        mCanvas = nullptr;
    }

    if (mContext) {
        //GrContext_Symbol.contextDestroyed(mContext);
        mContext->unref((void*)GrContext_Symbol.Dtor);
        mContext = nullptr;
    }

    Sk_SafeUnref(mInterface);
    mInterface = nullptr;
    Sk_SafeUnref(mRenderTarget);
    mRenderTarget = nullptr;

    hasBackend = false;
    return true;
}

void SkStupidRenderer_16::windowSizeChanged() {
    if (mContext) {
        GrPlatformRenderTargetDesc_t desc;
        desc.width = this->mWidth;
        desc.height = this->mHeight;
        desc.config = kSkia8888_PM_GrPixelConfig;
        desc.sampleCount = mMSAASampleCount;
        desc.stencilBits = 8;

        GLint buffer = 0;
        glGetIntegerv(SK_GR_GL_FRAMEBUFFER_BINDING, &buffer);
        desc.renderTargetHandle = buffer;

        Sk_SafeUnref(mRenderTarget);
        mRenderTarget = GrContext_Symbol.createPlatformRenderTarget(mContext, &desc);

        if (mCanvas) {
            mCanvas->unref((void*)SkCanvas_Symbol.Dtor);
            mCanvas = nullptr;

            this->createSkCanvas();
        }
    }
}

void SkStupidRenderer_16::createSkCanvas() {
    SkGpuDevice_t* gpuDevice = (SkGpuDevice_t*)malloc(sizeof_SkGpuDevice);
    SkGpuDevice_Symbol.Ctor(gpuDevice, mContext, mRenderTarget);

    SkCanvas_t* skcanvas = (SkCanvas_t*)malloc(sizeof_SkCanvas);
    SkCanvas_Symbol.Ctor(skcanvas, gpuDevice);
    mCanvas = skcanvas;

    gpuDevice->unref((void*)SkGpuDevice_Symbol.Dtor);
}

void SkStupidRenderer_16::updateSize(int width, int height) {
    this->mWidth = width;
    this->mHeight = height;
    this->windowSizeChanged();
}

SkCanvas_t* SkStupidRenderer_16::lockCanvas() {
    if (mCanvas == nullptr) {
        if (mContext) {
            this->createSkCanvas();
        }
    }
    return mCanvas;
}

void SkStupidRenderer_16::unlockCanvasAndPost(SkCanvas_t* canvas) {
    if (mContext) {
        GrContext_Symbol.flush(mContext, 0);
    }
}
