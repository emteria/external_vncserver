package com.rotationhelpermanager;

import android.content.Context;
import android.util.Log;
import android.hardware.display.DisplayManager;
import android.os.IBinder;
import android.os.RemoteException;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;
import com.rotationhelper.IRotationHelper;

public class RotationHelperManager { 
    private static final String LOGTAG = "RotationHelperManager";
    private static final String REMOTE_SERVICE_NAME = "RotationHelper";
    private static IRotationHelper mRotationHelperService;
    private static Context sContext;

    public RotationHelperManager() {
        Method method = null;
        Log.d(LOGTAG, "Connecting to service " + REMOTE_SERVICE_NAME);
        try {
            method = Class.forName("android.os.ServiceManager").getMethod("getService", String.class);
            IBinder binder = (IBinder) method.invoke(null, REMOTE_SERVICE_NAME);
            if(binder != null) {
                mRotationHelperService = IRotationHelper.Stub.asInterface(binder);
            }
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        } catch (IllegalStateException e) {
            e.printStackTrace();
        }
        if (mRotationHelperService == null) {
            Log.d(LOGTAG, "Connecting to service" + REMOTE_SERVICE_NAME + " failed!");
        } else {
            Log.d(LOGTAG, "Connection to service " + REMOTE_SERVICE_NAME + " is successful!");
        }
    }

    public void setRotation(int rotation) {
        try {
            Log.d(LOGTAG, "Calling" + REMOTE_SERVICE_NAME + "'s setRotation!");
            mRotationHelperService.setRotation(rotation);
        } catch (RemoteException e) {
            throw new RuntimeException("setRotation", e);
        }
    }
}
