package com.raplt.test;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends Activity {
    static { System.loadLibrary("test_jni"); }

    private TextView tv;

    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);
        tv = new TextView(this);
        tv.setText("RaPLT hook test\n");
        setContentView(tv);

        runTest();
    }

    private void log(String s) {
        android.util.Log.i("RaPLT", s);
        tv.append(s + "\n");
    }

    private native String nativeTest();

    private void runTest() {
        log("starting hook test...");
        String result = nativeTest();
        log(result);
    }
}
