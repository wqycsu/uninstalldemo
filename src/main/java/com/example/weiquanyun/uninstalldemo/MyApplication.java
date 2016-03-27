package com.example.weiquanyun.uninstalldemo;

import android.app.Application;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.SystemClock;

import com.example.weiquanyun.lib.AppUninstallListener;

/**
 * Created by weiquanyun on 16/3/27.
 */
public class MyApplication extends Application {
    @Override
    public void onCreate() {
        super.onCreate();
        System.loadLibrary("uninstall");
        AppUninstallListener.getInstance().uninstallListen(
                getPackageName(),
                getVersionName(), "12345",
                Build.BRAND + Build.BOARD,
                String.valueOf(Build.VERSION.SDK_INT)
                );
    }

    private String getVersionName() {
        PackageManager packageManager = getPackageManager();
        // getPackageName()是你当前类的包名，0代表是获取版本信息
        String version = null;
        PackageInfo packInfo = null;
        try {
            packInfo = packageManager.getPackageInfo(getPackageName(),0);
            version = packInfo.versionName;
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
        return version;
    }
}
