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

#include <jni.h>
#include <dlfcn.h>
#include "version_utils.hpp"
#include "sk_stupid_common_def.hpp"
#include "sk_stupid_injector.hpp"

bool SkStupidInjector::supportApi(int api) {
    return (api >= minSdkVersion) && (api <= maxSdkVersion);
}

SkStupidInjector::SkStupidInjector(JNIEnv* env) {
    mApiLevel = getDeviceApiLevel();
    if (this->supportApi(mApiLevel) == false) {
        mSymbolsComplete = false;
        return;
    }

    loadSymbols(env);
}

SkStupidInjector::~SkStupidInjector() {

}

void SkStupidInjector::loadSymbols(JNIEnv* env) {
    if (mApiLevel >= 21) {  // Android 5.0+
        mRTLibrary = dlopen("libandroid_runtime.so", RTLD_NOW | RTLD_LOCAL);
        if (mRTLibrary == nullptr) {
            mSymbolsComplete = false;
            return;
        }
        create_canvas = (Func_create_canvas)dlsym(mRTLibrary, "_ZN7android6Canvas13create_canvasEP8SkCanvas");
    }

    jclass clazz = env->FindClass("android/graphics/Canvas");
    mCanvasClass = (jclass)env->NewGlobalRef(clazz);
    env->DeleteLocalRef(clazz);

    if (mApiLevel >= 20) {  // Android 4.4w+
        mCanvasCtorID = env->GetMethodID(mCanvasClass, "<init>", "(J)V"); // 4.4w+ use int64 to store pointer
    } else {                // before
        mCanvasCtorID = env->GetMethodID(mCanvasClass, "<init>", "(I)V");
    }

    if (mApiLevel >= 19) {
        mCanvasReleaseID = env->GetMethodID(mCanvasClass, "release", "()V");
    } else {
        jclass clazz = env->FindClass("android/graphics/Canvas$CanvasFinalizer");
        mCanvasFinalizerClass = (jclass)env->NewGlobalRef(clazz);
        env->DeleteLocalRef(clazz);

        mCanvasFinalizerID = env->GetFieldID(mCanvasClass, "mFinalizer", "Landroid/graphics/Canvas$CanvasFinalizer;");
        mCanvasFinalizerHandleID = env->GetFieldID(mCanvasFinalizerClass, "mNativeCanvas", "I");
        mCanvasFinalizerFinalizeID = env->GetMethodID(mCanvasFinalizerClass, "finalize", "()V");
        mCanvasHandleID = env->GetFieldID(mCanvasClass, "mNativeCanvas", "I");
    }

    mSymbolsComplete = checkSymbols();
}

bool SkStupidInjector::checkSymbols() {
    if (mApiLevel >= 21 && (mRTLibrary == nullptr || create_canvas == nullptr)) {
        return false;
    }

    if (mCanvasClass == nullptr || mCanvasCtorID == nullptr) {
        return false;
    }

    if (mApiLevel >= 19 && mCanvasReleaseID == nullptr) {
        return false;
    }

    if (mApiLevel < 19) {
        if (mCanvasFinalizerClass == nullptr || mCanvasFinalizerID == nullptr || mCanvasFinalizerHandleID == nullptr
            || mCanvasFinalizerFinalizeID == nullptr || mCanvasHandleID == nullptr) {
            return false;
        }
    }

    return true;
}

bool SkStupidInjector::isDeviceSupported() {
    return mSymbolsComplete;
}

void SkStupidInjector::dispose(JNIEnv* env) {
    if (mCanvas) {
        if (mApiLevel >= 19) {
            env->CallVoidMethod(mCanvas, mCanvasReleaseID);
        } else {
        	jobject finalizer = env->GetObjectField(mCanvas, mCanvasFinalizerID);
        	env->CallVoidMethod(finalizer, mCanvasFinalizerFinalizeID);
        	env->SetIntField(finalizer, mCanvasFinalizerHandleID, 0);
        	env->SetIntField(finalizer, mCanvasHandleID, 0);
        	env->DeleteLocalRef(finalizer);
        }
        env->DeleteGlobalRef(mCanvas);
        mCanvas = nullptr;
    }

    if (mCanvasClass) {
        env->DeleteGlobalRef(mCanvasClass);
        mCanvasClass = nullptr;
    }

    if (mApiLevel < 19 && mCanvasFinalizerClass) {
    	env->DeleteGlobalRef(mCanvasFinalizerClass);
    	mCanvasFinalizerClass = nullptr;
    }

    if (mApiLevel >= 21 && mRTLibrary) {
        dlclose(mRTLibrary);
        mRTLibrary = nullptr;
        create_canvas = nullptr;
    }
}

jobject SkStupidInjector::getJavaCanvas(JNIEnv* env, SkCanvas_t* skcanvas) {
    if (mSymbolsComplete == false) {
        return nullptr;
    }

    if (mSkCanvas != skcanvas) {
        mSkCanvas = skcanvas;

        if (mCanvas) {
            if (mApiLevel >= 19) {
                env->CallVoidMethod(mCanvas, mCanvasReleaseID);
            } else {
            	jobject finalizer = env->GetObjectField(mCanvas, mCanvasFinalizerID);
            	env->CallVoidMethod(finalizer, mCanvasFinalizerFinalizeID);
            	env->SetIntField(finalizer, mCanvasFinalizerHandleID, 0);
            	env->SetIntField(finalizer, mCanvasHandleID, 0);
            	env->DeleteLocalRef(finalizer);
            }
            env->DeleteGlobalRef(mCanvas);
            mCanvas = nullptr;
        }

        if (mApiLevel >= 20) {  // Android 4.4w+
            mSkCanvas->ref();
            void* canvasWrapper = nullptr;
            if (mApiLevel >= 21) {  // Android 5.0+
                canvasWrapper = create_canvas(mSkCanvas);
            } else {                // Android 4.4w
                canvasWrapper = mSkCanvas;
            }
            mCanvas = env->NewObject(mCanvasClass, mCanvasCtorID, reinterpret_cast<jlong>(canvasWrapper));
        } else {                // before
            mSkCanvas->ref();
            mCanvas = env->NewObject(mCanvasClass, mCanvasCtorID, reinterpret_cast<jint>(mSkCanvas));
        }

        mCanvas = env->NewGlobalRef(mCanvas);
    }
    return mCanvas;
}
