package com.twt.simplenativeandroidapp

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import com.twt.simplenativeandroidapp.license.License
import com.twt.simplenativeandroidapp.license.LicenseManager
import com.twt.simplenativeandroidapp.ui.theme.SimpleNativeAndroidAppTheme

class MainActivity : ComponentActivity() {

    init {
        System.loadLibrary("simplenativeandroidapp")
    }

    private external fun stringFromJNI(): String
    private external fun validateLicense(encryptedLicense: String, serverKeyPart: String): Boolean

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()

        val license = License(
            bundle_id = "com.rr.test",
            os = "android",
            expiry_millis = System.currentTimeMillis() + 1000 * 60 * 60 * 24 * 365, // 1 year
            plan = "demo"
        )

        val encryptedLicense = LicenseManager.encryptLicense(license)
        val serverKeyPart = "this-is-a-server-key-part-2"

        val isLicenseValid = validateLicense(encryptedLicense, serverKeyPart)

        setContent {
            SimpleNativeAndroidAppTheme {
                Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                    Greeting(
                        name = stringFromJNI(),
                        isLicenseValid = isLicenseValid,
                        modifier = Modifier.padding(innerPadding)
                    )
                }
            }
        }
    }
}

@Composable
fun Greeting(name: String, isLicenseValid: Boolean, modifier: Modifier = Modifier) {
    Text(
        text = "Hello $name!\nLicense valid: $isLicenseValid",
        modifier = modifier
    )
}

@Preview(showBackground = true)
@Composable
fun GreetingPreview() {
    SimpleNativeAndroidAppTheme {
        Greeting("Android", true)
    }
}