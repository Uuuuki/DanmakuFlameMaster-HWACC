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

#ifndef _SKIA_REDIRECTOR_JNI_HPP
#define _SKIA_REDIRECTOR_JNI_HPP

#include <jni.h>

int initSkiaRedirectorJni(JNIEnv* env);
int termSkiaRedirectorJni();

int registerSkiaRedirectorMethods(JNIEnv* env, const char* className);

#endif // _SKIA_REDIRECTOR_JNI_HPP
