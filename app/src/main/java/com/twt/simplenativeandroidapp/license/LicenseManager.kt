package com.twt.simplenativeandroidapp.license

import com.google.gson.Gson
import java.util.Base64
import javax.crypto.Cipher
import javax.crypto.spec.IvParameterSpec
import javax.crypto.spec.SecretKeySpec

object LicenseManager {
    // A single, hardcoded 32-byte key for AES-256.
    private val AES_KEY = "a-super-secret-key-for-aes-256!".toByteArray(Charsets.UTF_8)

    fun encryptLicense(license: License): String {
        val gson = Gson()
        val licenseJson = gson.toJson(license)

        val key = AES_KEY
        val iv = ByteArray(16) // For simplicity, using a zero IV.
        val secretKeySpec = SecretKeySpec(key, "AES")
        val ivParameterSpec = IvParameterSpec(iv)

        val cipher = Cipher.getInstance("AES/CBC/PKCS5Padding")
        cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec, ivParameterSpec)

        val encryptedBytes = cipher.doFinal(licenseJson.toByteArray(Charsets.UTF_8))
        return Base64.getEncoder().encodeToString(encryptedBytes)
    }
}
