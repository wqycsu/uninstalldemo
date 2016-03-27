//
// Created by 魏全运 on 16/3/27.
//
#include <string>
#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/file.h>
#include <android/log.h>
#include "com_example_weiquanyun_lib_AppUninstallListener.h"

#define LOG_TAG "AppUninstallListener@native"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

using namespace std;

extern char *__progname;

//使用绑定socket形式判断进程是否存在,有风险
int need_fork_new_process() {
    //create socket
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd==-1)
    {
        LOGD("create socket success");
        return 1;
    }
    printf("socket fd=%d\n",fd);

    //build connection address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9876);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int r = -2;
    r = bind(fd,(struct sockaddr*)&addr,sizeof(addr));
    if(r == -1)
    {
        LOGD("bind socket failed");
        close(fd);
        return 0;
    } else if(r == 0) {
        LOGD("bind socket success");
        return 1;
    }
    return 1;
}

//使用文件锁的形式判断
int need_fork_new_process_with_fl(string packageNameStr) {
    string path = "/data/data/";
    path += packageNameStr;
    path += "/lib";
    int fd = open(path.c_str(), O_RDONLY);
    return flock(fd,LOCK_EX|LOCK_NB);
}

//上传卸载信息到服务端
int postUninstallInfoToServer(string packageNameStr, string versionCodeStr, string ttidStr, string phoneBrandStr, string osVersionStr ) {
    int retCode = 1;
    LOGD("上传信息到服务端");
    return retCode;
}

JNIEXPORT void JNICALL Java_com_example_weiquanyun_lib_AppUninstallListener_uninstallListen
  (JNIEnv *env, jobject obj, jstring packageName, jstring versionCode, jstring ttid,
   jstring phoneBrand, jstring osVersion){

    const char * p_package = env->GetStringUTFChars(packageName, NULL);
    string packageNameStr = p_package;
    env->ReleaseStringUTFChars(packageName, p_package);

    const char * p_version = env->GetStringUTFChars(versionCode, NULL);
    string versionCodeStr = p_version;
    env->ReleaseStringUTFChars(versionCode, p_version);

    const char * p_ttid = env->GetStringUTFChars(ttid, NULL);
    string ttidStr = p_ttid;
    env->ReleaseStringUTFChars(ttid, p_ttid);

    const char * p_phoneBrand = env->GetStringUTFChars(phoneBrand, NULL);
    string phoneBrandStr = p_phoneBrand;
    env->ReleaseStringUTFChars(phoneBrand, p_phoneBrand);

    const char * p_osVersion = env->GetStringUTFChars(osVersion, NULL);
    string osVersionStr = p_phoneBrand;
    env->ReleaseStringUTFChars(osVersion, p_osVersion);

    //if(!need_fork_new_process()) {
        //LOGD("has exist a process on listening");
        //return;
    //}

    if(need_fork_new_process_with_fl(packageNameStr) != 0) {
        LOGD("file has locked by another process");
        return;
    }

    pid_t pid = fork();

    if(pid < 0) {
        //进程fork失败
        LOGD("fork process failed");
    } else if(pid > 0) {
        //父进程中运行
    } else {
        //fork出的进程运行
        LOGD("fork process success,current pid = %d", getpid());
        sprintf(__progname, "uninstall_check");

        //初始化inotify
        int fd = inotify_init();
        if (fd < 0) {
            LOGD("inotify_init failed");
            exit(1);
        }
        string path = "/data/data/";
        path += packageNameStr;
        path += "/lib";
        //LOGD("watching path:%s", path.c_str());
        int wd = inotify_add_watch(fd, path.c_str(), IN_DELETE);
        if (wd < 0) {
            LOGD("inotify_add_watch failed");
            exit(1);
        }

        //分配缓存，以便读取event，缓存大小=一个struct inotify_event的大小，这样一次处理一个event
        void *p_buf = malloc(sizeof(struct inotify_event));
        if (p_buf == NULL) {
            LOGD("malloc failed !!!");
            exit(1);
        }

        //开始监听
        LOGD("start observer");
        while(1) {
            ssize_t readBytes = read(fd, p_buf,sizeof(struct inotify_event));
            LOGD("read event happens");
            //read会阻塞进程，走到这里说明收到目录被删除的事件
            FILE *libDir = fopen(path.c_str(), "r");//判断该目录是否存在,如果存在则是覆盖安装
            if(libDir == NULL) {
                // 应用被卸载了
                LOGD("app uninstall,current version = %s, ttid = %s, phoneBrand = %s, osVersion = %s", versionCodeStr.c_str(), ttidStr.c_str(), phoneBrandStr.c_str(), osVersionStr.c_str());
                inotify_rm_watch(fd, IN_DELETE);
                free(p_buf);
                p_buf = NULL;
                int ret = postUninstallInfoToServer(packageNameStr, versionCodeStr, ttidStr, phoneBrandStr, osVersionStr);
                if(ret) {
                    LOGD("post info success!!!");
                } else {
                    LOGD("post info failed!!!");
                }
                exit(0);
            } else {
                // 覆盖安装
                LOGD("app reinstall");
                fclose(libDir);
                free(p_buf);
                p_buf = NULL;
                exit(0);
            }
        }
        if(p_buf != NULL) {
            free(p_buf);
        }
    }
}

