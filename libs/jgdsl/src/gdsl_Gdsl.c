/*
 * gdsl_Gdsl.c
 *
 *  Created on: Feb 13, 2014
 *      Author: jucs
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <jni.h>
#include <gdsl.h>
#include <gdsl_multiplex.h>
#include "gdsl_Gdsl.h"

#define THROW_RUNTIME(RET, MSG) {\
		jclass exp = (*env)->FindClass(env, "java/lang/RuntimeException");\
		(*env)->ThrowNew(env, exp, MSG);\
		return RET;\
}

JNIEXPORT jobjectArray JNICALL Java_gdsl_Gdsl_getFrontendsNative(JNIEnv *env, jobject this) {
	struct frontend_desc *descs = NULL;
	size_t descs_length = gdsl_multiplex_frontends_list(&descs);

	jclass Gdsl_Frontend = (*env)->FindClass(env, "gdsl/Frontend");

	jobjectArray jfrontends = (*env)->NewObjectArray(env, descs_length, Gdsl_Frontend, (*env)->NewStringUTF(env, ""));

	for(size_t i = 0; i < descs_length; ++i) {
		jstring name = (*env)->NewStringUTF(env, descs[i].name);
		jstring ext = (*env)->NewStringUTF(env, descs[i].ext);

		jmethodID cons = (*env)->GetMethodID(env, Gdsl_Frontend, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");
		jobject frontend = (*env)->NewObject(env, Gdsl_Frontend, cons, name, ext);

		(*env)->SetObjectArrayElement(env, jfrontends, i, frontend);
	}

	gdsl_multiplex_descs_free(descs, descs_length);

	return jfrontends;
}

JNIEXPORT jlong JNICALL Java_gdsl_Gdsl_getFrontendPtr(JNIEnv *env, jobject this, jobject jfrontend) {
	jclass Gdsl_Frontend = (*env)->FindClass(env, "gdsl/Frontend");

	jmethodID getName = (*env)->GetMethodID(env, Gdsl_Frontend, "getName", "()Ljava/lang/String;");
	jmethodID getExt = (*env)->GetMethodID(env, Gdsl_Frontend, "getExt", "()Ljava/lang/String;");

	jstring jname = (*env)->CallObjectMethod(env, jfrontend, getName);
	jstring jext = (*env)->CallObjectMethod(env, jfrontend, getExt);

	const char *name = (*env)->GetStringUTFChars(env, jname, 0);
	const char *ext = (*env)->GetStringUTFChars(env, jext, 0);

	struct frontend_desc desc;
	desc.name = name;
	desc.ext = ext;

	struct frontend *frontend = (struct frontend*)malloc(sizeof(struct frontend));
	char error = gdsl_multiplex_frontend_get(frontend, desc);

	if(error != GDSL_MULTIPLEX_ERROR_NONE) free(frontend);

	switch(error) {
		case GDSL_MULTIPLEX_ERROR_FRONTENDS_PATH_NOT_SET:
			THROW_RUNTIME(0, "Unable to open frontend: Path to frontends not set")
		case GDSL_MULTIPLEX_ERROR_UNABLE_TO_OPEN:
			THROW_RUNTIME(0, "Unable to open frontend: Unable to open frontend library")
		case GDSL_MULTIPLEX_ERROR_SYMBOL_NOT_FOUND:
			THROW_RUNTIME(0, "Unable to open frontend: Symbol not found")
		case GDSL_MULTIPLEX_ERROR_NONE:
			break;
	}

	(*env)->ReleaseStringUTFChars(env, jname, name);
	(*env)->ReleaseStringUTFChars(env, jext, ext);

	return (jlong)frontend;
}

JNIEXPORT jlong JNICALL Java_gdsl_Gdsl_init(JNIEnv *env, jobject this, jlong frontendPtr) {
	struct frontend *frontend = (struct frontend*)frontendPtr;

	return (jlong)frontend->generic.init();
}

JNIEXPORT void JNICALL Java_gdsl_Gdsl_setCode(JNIEnv *env, jobject this, jlong frontendPtr, jlong gdslStatePtr,
		jobject byteBuffer, jlong base) {
	jclass java_Nio_ByteBuffer = (*env)->FindClass(env, "java/nio/ByteBuffer");
	jmethodID capacity = (*env)->GetMethodID(env, java_Nio_ByteBuffer, "capacity", "()I");

	jint size = (*env)->CallIntMethod(env, byteBuffer, capacity);
	jbyte *buffer = (*env)->GetDirectBufferAddress(env, byteBuffer);

	struct frontend *frontend = (struct frontend*)frontendPtr;
	state_t state = (state_t)gdslStatePtr;

	frontend->generic.set_code(state, (char*)buffer, (uint64_t)size, (uint64_t)base);
}

JNIEXPORT jlong JNICALL Java_gdsl_Gdsl_decodeOne(JNIEnv *env, jobject this, jlong frontendPtr, jlong gdslStatePtr) {
	struct frontend *frontend = (struct frontend*)frontendPtr;
	state_t state = (state_t)gdslStatePtr;

	obj_t insn = frontend->decoder.decode(state, frontend->decoder.config_default(state));
	return (jlong)insn;
}
