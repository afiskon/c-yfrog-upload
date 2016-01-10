#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <curl/curl.h>

#define USER_AGENT_STRING "User-Agent: Mozilla/5.0 (X11; Linux x86_64) " \
    "AppleWebKit/537.36 (KHTML, like Gecko) Ubuntu Chromium/47.0.2526.73 " \
    "Chrome/47.0.2526.73 Safari/537.36"

#define WRITE_FUNC_CTX_INIT_BUFF_SIZE 4096

typedef struct
{
    char* buff;
    size_t buffSize;
    size_t used;
} WriteFuncCtx;

bool writeFuncCtxInit(WriteFuncCtx* ctx)
{
    ctx->used = 0;
    ctx->buffSize = 0;
    ctx->buff = (char*)malloc(WRITE_FUNC_CTX_INIT_BUFF_SIZE);
    if(ctx->buff == NULL)
        return false;
    
    ctx->buffSize = WRITE_FUNC_CTX_INIT_BUFF_SIZE;
    return true;
}

bool writeFuncCtxAppend(WriteFuncCtx* ctx, const char* data, size_t size)
{
    size_t newSize = ctx->buffSize;
    
    while(ctx->used + size > newSize)
        newSize *= 2;

    if(newSize != ctx->buffSize)
    {
        char* newPtr = (char*)realloc(ctx->buff, newSize);
        if(!newPtr)
            return false;

        ctx->buff = newPtr;
        ctx->buffSize = newSize;
    }

    memcpy(ctx->buff + ctx->used, data, size);
    ctx->used += size;
    return true;
}

void writeFuncCtxFree(WriteFuncCtx* ctx)
{
    if(ctx->buff != NULL)
        free(ctx->buff);
}

size_t writeCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    WriteFuncCtx* ctx = (WriteFuncCtx*)userdata;
    size_t totalSize = size*nmemb;

    if(writeFuncCtxAppend(ctx, ptr, totalSize))
        return totalSize;
    else
        return 0;
}

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        printf("Usage: %s <fname>\n", argv[0]);
        return 1;
    }

    char* fname = argv[1];

    char url[64];
    srandom(time(0));
    snprintf(url, sizeof(url),
        "http://iload%u.imageshack.us/upload_api.php",
        1 + ((unsigned int)random()) % 10);
    
    if(curl_global_init(CURL_GLOBAL_ALL) != 0)
    {
        fprintf(stderr, "curl_global_init() failed\n");
        return 1;  
    }

    CURL* curl = curl_easy_init();
    if(curl == NULL)
    {
        fprintf(stderr, "curl_easy_init() failed\n");
        return 1;
    }

    struct curl_slist* curlHeaders = NULL;
    curlHeaders = curl_slist_append(curlHeaders, USER_AGENT_STRING);

    if(curlHeaders == NULL)
    {
        fprintf(stderr, "curl_slist_append() failed\n");
        curl_easy_cleanup(curl);
        return 1;
    }

    struct curl_httppost* formFirstItem = NULL;
    struct curl_httppost* formLastItem = NULL;

    curl_formadd(&formFirstItem,
        &formLastItem,
        CURLFORM_COPYNAME, "key",
        CURLFORM_COPYCONTENTS, "015EFMNVfe7f6f7e93cb4a7b0a41e19956ce59f8",
        CURLFORM_END);

    curl_formadd(&formFirstItem,
        &formLastItem,
        CURLFORM_COPYNAME, "Filedata",
        CURLFORM_FILE, fname,
        CURLFORM_END);

    WriteFuncCtx ctx;

    if(!writeFuncCtxInit(&ctx))
    {
        fprintf(stderr, "writeFuncCtxInit() failed\n");
        curl_formfree(formFirstItem);
        curl_slist_free_all(curlHeaders);
        curl_easy_cleanup(curl);
        return 1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curlHeaders);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formFirstItem);

    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
        writeFuncCtxFree(&ctx);
        curl_formfree(formFirstItem);
        curl_slist_free_all(curlHeaders);
        curl_easy_cleanup(curl);
        return 1;
    }

    if(!writeFuncCtxAppend(&ctx, "", 1))
    {
        fprintf(stderr, "Final writeFuncCtxAppend failed\n");
        writeFuncCtxFree(&ctx);
        curl_formfree(formFirstItem);
        curl_slist_free_all(curlHeaders);
        curl_easy_cleanup(curl);
        return 1;
    }

    static char begin_str[] = "<image_link>";
    char* end = NULL;
    char* begin = strstr(ctx.buff, begin_str);
    if(begin != NULL)
    {
        begin += (sizeof(begin_str) - 1);
        end = strstr(begin, "</image_link>");
    }

    if(begin != NULL && end != NULL)
    {
        *end = '\0';
        printf("%s\n", begin);
    }
    else
    {
        fprintf(stderr, "Failed to parse response\n");
    }

    writeFuncCtxFree(&ctx);
    curl_formfree(formFirstItem);
    curl_slist_free_all(curlHeaders);
    curl_easy_cleanup(curl);
      
    return 0;
}
