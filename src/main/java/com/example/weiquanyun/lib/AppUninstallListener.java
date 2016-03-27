package com.example.weiquanyun.lib;

/**
 * Created by weiquanyun on 16/3/27.
 */
public class AppUninstallListener {
    private static final AppUninstallListener INSTANCE = new AppUninstallListener();

    private AppUninstallListener(){

    }

    public static AppUninstallListener getInstance() {
        return INSTANCE;
    }

    public native void uninstallListen(String packageName, String versionName, String ttid, String phoneInfo, String osVersion);
}
