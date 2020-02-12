/* =========================================================================
 * This file is part of NITRO
 * =========================================================================
 * 
 * (C) Copyright 2004 - 2014, MDA Information Systems LLC
 *
 * NITRO is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this program; if not, If not, 
 * see <http://www.gnu.org/licenses/>.
 *
 */

/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
#include <import/nitf.h>
/* Header for class nitf_PluginRegistry */

#ifndef _Included_nitf_PluginRegistry
#define _Included_nitf_PluginRegistry
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     nitf_PluginRegistry
 * Method:    loadPluginDir
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_nitf_PluginRegistry_loadPluginDir
  (JNIEnv *, jclass, jstring);
  
 /*
 * Class:     nitf_PluginRegistry
 * Method:    canHandleTRE
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_nitf_PluginRegistry_canHandleTRE
  (JNIEnv *, jclass, jstring);

#ifdef __cplusplus
}
#endif
#endif
