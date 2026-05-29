package com.raplt.test;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends Activity {
    static { System.loadLibrary("jni"); }
    private TextView tv;

    @Override
    protected void onCreate(Bundle s) {
        super.onCreate(s);
        tv = new TextView(this);
        setContentView(tv);
        run();
    }

    private void log(String line) {
        android.util.Log.i("RaPLT", line);
        tv.append(line + "\n");
    }

    private native String nativeRun();

    private void run() {
        log("RaPLT " + nativeRun());
    }
}
