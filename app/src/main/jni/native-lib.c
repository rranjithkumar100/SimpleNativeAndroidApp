#include <jni.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "aes.h"
#include "b64.h"

// For logging
#include <android/log.h>
#define LOG_TAG "NativeLicense"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static void xor(const uint8_t* in, size_t in_len, const uint8_t* key, size_t key_len, uint8_t* out) {
    for (size_t i = 0; i < in_len; ++i) {
        out[i] = in[i] ^ key[i % key_len];
    }
}

long long current_time_millis() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000 + (long long)tv.tv_usec / 1000;
}

static size_t b64_decoded_size(const char *in) {
    size_t len = strlen(in);
    if (len % 4 != 0) return 0; // Invalid length

    size_t ret = len / 4 * 3;

    if (in[len - 1] == '=') {
        ret--;
    }
    if (in[len - 2] == '=') {
        ret--;
    }

    return ret;
}

JNIEXPORT jstring JNICALL
Java_com_twt_simplenativeandroidapp_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    char *hello = "Hello from JNI";
    return (*env)->NewStringUTF(env, hello);
}

jboolean checkSignature(JNIEnv* env, jobject thiz) {
    // TODO: Implement a real signature check
    return JNI_TRUE;
}

static void print_hex(const char* label, const uint8_t* data, size_t len) {
    if (len == 0) {
        LOGI("%s: (empty)", label);
        return;
    }
    char hex_str[len * 3 + 1];
    for (size_t i = 0; i < len; ++i) {
        sprintf(hex_str + i * 3, "%02x ", data[i]);
    }
    hex_str[len * 3 -1] = '\0'; // Remove trailing space
    LOGI("%s (%zu bytes): %s", label, len, hex_str);
}

static size_t pkcs7_unpad(uint8_t* data, size_t len) {
    if (len == 0) {
        return 0;
    }

    uint8_t pad_len = data[len - 1];

    if (pad_len > AES_BLOCKLEN || pad_len == 0 || pad_len > len) {
        return len;
    }

    for (size_t i = len - pad_len; i < len; ++i) {
        if (data[i] != pad_len) {
            return len;
        }
    }

    return len - pad_len;
}

JNIEXPORT jboolean JNICALL
Java_com_twt_simplenativeandroidapp_MainActivity_validateLicense(
        JNIEnv* env,
        jobject thiz,
        jstring encrypted_license_jstr) {

    if (!checkSignature(env, thiz)) {
        LOGI("Signature check failed");
        return JNI_FALSE;
    }

    const char *encrypted_license_b64 = (*env)->GetStringUTFChars(env, encrypted_license_jstr, 0);

    // 1. Set the key
    const char* key_str = "a-super-secret-key-for-aes-256!";
    uint8_t final_key[32];
    memcpy(final_key, key_str, 32);
    print_hex("AES Key", final_key, 32);

    // 2. Base64 decode the license
    size_t decoded_len = b64_decoded_size(encrypted_license_b64);
    if (decoded_len == 0) {
        LOGI("Base64 decoding failed: invalid length");
        (*env)->ReleaseStringUTFChars(env, encrypted_license_jstr, encrypted_license_b64);
        return JNI_FALSE;
    }

    b64_decoded_t *encrypted_license = decode_base64(encrypted_license_b64);
    if (encrypted_license == NULL) {
        LOGI("Base64 decoding failed");
        (*env)->ReleaseStringUTFChars(env, encrypted_license_jstr, encrypted_license_b64);
        return JNI_FALSE;
    }

    // 3. Decrypt the license
    struct AES_ctx ctx;
    uint8_t iv[AES_BLOCKLEN] = {0};
    print_hex("IV", iv, AES_BLOCKLEN);
    print_hex("B64 Decoded Data", (uint8_t*)encrypted_license, decoded_len);
    AES_init_ctx_iv(&ctx, final_key, iv);
    AES_CBC_decrypt_buffer(&ctx, (uint8_t*)encrypted_license, decoded_len);
    print_hex("Raw Decrypted Data", (uint8_t*)encrypted_license, decoded_len);

    // Remove PKCS#7 padding
    size_t unpadded_len = pkcs7_unpad((uint8_t*)encrypted_license, decoded_len);
    LOGI("Unpadded length: %zu", unpadded_len);

    char* decrypted_license_json = (char*)encrypted_license;
    decrypted_license_json[unpadded_len] = '\0'; // Null-terminate the unpadded string
    LOGI("Decrypted license: %s", decrypted_license_json);


    // 4. Validate the license
    // For simplicity, we use strstr. A real implementation should use a proper JSON parser.
    char* bundle_id_found = strstr(decrypted_license_json, "\"bundle_id\":\"com.rr.test\"");
    char* os_found = strstr(decrypted_license_json, "\"os\":\"android\"");

    if (bundle_id_found == NULL || os_found == NULL) {
        LOGI("Bundle ID or OS mismatch");
        free(encrypted_license);
        (*env)->ReleaseStringUTFChars(env, encrypted_license_jstr, encrypted_license_b64);
        return JNI_FALSE;
    }

    char* expiry_key = "\"expiry_millis\":";
    char* expiry_found = strstr(decrypted_license_json, expiry_key);
    if (expiry_found == NULL) {
        LOGI("Expiry not found");
        free(encrypted_license);
        (*env)->ReleaseStringUTFChars(env, encrypted_license_jstr, encrypted_license_b64);
        return JNI_FALSE;
    }

    long long expiry_millis = 0;
    sscanf(expiry_found + strlen(expiry_key), "%lld", &expiry_millis);

    if (expiry_millis == 0) {
        LOGI("Failed to parse expiry");
        free(encrypted_license);
        (*env)->ReleaseStringUTFChars(env, encrypted_license_jstr, encrypted_license_b64);
        return JNI_FALSE;
    }

    long long now = current_time_millis();
    if (now > expiry_millis) {
        LOGI("License expired");
        free(encrypted_license);
        (*env)->ReleaseStringUTFChars(env, encrypted_license_jstr, encrypted_license_b64);
        return JNI_FALSE;
    }

    // 5. Clean up
    free(encrypted_license);
    (*env)->ReleaseStringUTFChars(env, encrypted_license_jstr, encrypted_license_b64);

    LOGI("License is valid");
    return JNI_TRUE;
}
