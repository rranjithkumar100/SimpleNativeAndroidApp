package com.twt.simplenativeandroidapp.license

data class License(
    val bundle_id: String,
    val os: String,
    val expiry_millis: Long,
    val plan: String
)
