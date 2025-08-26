package com.twt.simplenativeandroidapp.license

import com.google.gson.Gson
import java.util.Base64
import javax.crypto.Cipher
import javax.crypto.spec.IvParameterSpec
import javax.crypto.spec.SecretKeySpec

object LicenseManager {
    // This will be obfuscated
    private val OBFUSCATED_KEY_PART = xor("this-is-a-hardcoded-key-part-1".toByteArray(Charsets.UTF_8), "a-super-secret-xor-key".toByteArray(Charsets.UTF_8))
    private const val SERVER_KEY_PART = "this-is-a-server-key-part-2"

    private fun xor(a: ByteArray, key: ByteArray): ByteArray {
        val out = ByteArray(a.size)
        for (i in a.indices) {
            out[i] = (a[i].toInt() xor key[i % key.size].toInt()).toByte()
        }
        return out
    }

    private fun getFullKey(): ByteArray {
        val deobfuscatedKeyPart = xor(OBFUSCATED_KEY_PART, "a-super-secret-xor-key".toByteArray(Charsets.UTF_8))
        val fullKey = deobfuscatedKeyPart.toString(Charsets.UTF_8) + SERVER_KEY_PART
        return fullKey.toByteArray(Charsets.UTF_8).copyOf(32) // AES-256 key
    }

    fun encryptLicense(license: License): String {
        val gson = Gson()
        val licenseJson = gson.toJson(license)

        val key = getFullKey()
        val iv = ByteArray(16) // For simplicity, using a zero IV. In a real scenario, a random IV should be generated and prepended to the ciphertext.
        val secretKeySpec = SecretKeySpec(key, "AES")
        val ivParameterSpec = IvParameterSpec(iv)

        val cipher = Cipher.getInstance("AES/CBC/PKCS5Padding")
        cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec, ivParameterSpec)

        val encryptedBytes = cipher.doFinal(licenseJson.toByteArray(Charsets.UTF_8))
        return Base64.getEncoder().encodeToString(encryptedBytes)
    }
}
