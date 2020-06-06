package com.cto.tcl.android_51cto_ffmpeg.c6;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;

/**
 * =======================================
 * Created by kevint on 2020/6/6.
 * =======================================
 */
public class XPlay extends GLSurfaceView implements Runnable, SurfaceHolder.Callback {

    public XPlay(Context context) {
        super(context);
    }

    public XPlay(Context context, AttributeSet attrs) {
        super(context, attrs);
        Log.e("kevint", "XPlay create");

    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
//        super.surfaceCreated(holder);
        new Thread(this).start();
        Log.e("kevint", "surfaceCreated");
    }


    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
//        super.surfaceChanged(holder, format, w, h);
        Log.e("kevint", "surfaceChanged");
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
//        super.surfaceDestroyed(holder);
        Log.e("kevint", "surfaceDestroyed");
    }

    @Override
    public void run() {
        Log.e("kevint", "run");
        open("", getHolder().getSurface());
    }

    public native void open(String url, Object surface);
}
