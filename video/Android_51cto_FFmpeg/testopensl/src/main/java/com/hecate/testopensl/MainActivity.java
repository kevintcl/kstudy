package com.hecate.testopensl;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        opensl("");
    }

    public native void opensl(String url);

    static {
        System.loadLibrary("audio-lib");
    }
}
