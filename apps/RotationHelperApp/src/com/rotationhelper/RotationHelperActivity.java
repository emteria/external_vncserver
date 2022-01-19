package com.rotationhelperapp;

import android.view.View;
import android.util.Log;
import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.ServiceManager;
import android.util.Log;
import android.hardware.display.DisplayManager;
import android.view.Surface;
import android.view.WindowManager;
import android.content.pm.ActivityInfo;
import com.rotationhelpermanager.RotationHelperManager;

public class RotationHelperActivity extends Activity {
    private static final String LOGTAG = "RotationHelperActivity";
    private static final Object sRotationLock = new Object();
    private static int sDeviceRotation = Surface.ROTATION_0;
    private static RotationHelperListener mRotationHelperListener;
    private static RotationHelperManager mRotationHelperManager;
    private static Context sContext;
    private static RotationHandler mRotationHandler;
    private static Boolean isLandscape = true;

    private class RotationHandler extends Handler {
        public static final int EVENT_DISPLAY_CHANGED = 2;

        @Override
        public synchronized void handleMessage(Message msg) {
            Log.d(LOGTAG, "Handling message.");
            updateOrientation();
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
    }

    public void onButtonConnect(View view)
    {
        sContext = getApplicationContext();
        mRotationHandler = new RotationHandler();

        mRotationHelperListener = new RotationHelperListener();
        mRotationHelperManager = new RotationHelperManager();
        ((DisplayManager) sContext.getSystemService(Context.DISPLAY_SERVICE))
                .registerDisplayListener(mRotationHelperListener, mRotationHandler);
    }

    public void onButtonDisconnect(View view)
    {
        ((DisplayManager) sContext.getSystemService(Context.DISPLAY_SERVICE))
                .unregisterDisplayListener(mRotationHelperListener);
    }

    public void onButtonRotate(View view)
    {
        if (isLandscape == true) {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR_PORTRAIT);
            Log.d(LOGTAG, "Set Portrait orientation");
            isLandscape = false;
        } else {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE);
            Log.d(LOGTAG, "Set Landscape orientation");
            isLandscape = true;
        }
    }

    /**
     * Query current display rotation and publish the change if any.
     */
    static void updateOrientation() {
        Log.d(LOGTAG, "Getting newRotation");
        int newRotation = ((WindowManager) sContext.getSystemService(
                Context.WINDOW_SERVICE)).getDefaultDisplay().getRotation();
        Log.d(LOGTAG, "newRotation: " + newRotation);
        synchronized(sRotationLock) {
            if (newRotation != sDeviceRotation) {
                sDeviceRotation = newRotation;
                //publishRotation(sDeviceRotation);
            }
            Log.d(LOGTAG, "Calling mRotationHelperManager.setRotation(newRotation)");
            mRotationHelperManager.setRotation(newRotation);
        }
    }

    /**
     * Uses android.hardware.display.DisplayManager.DisplayListener
     */
    final static class RotationHelperListener implements DisplayManager.DisplayListener {
        @Override
        public void onDisplayAdded(int displayId) {
        }

        @Override
        public void onDisplayRemoved(int displayId) {
        }

        @Override
        public void onDisplayChanged(int displayId) {
            Log.d(LOGTAG, "onDisplayChanged");
            updateOrientation();
        }
    }
}

