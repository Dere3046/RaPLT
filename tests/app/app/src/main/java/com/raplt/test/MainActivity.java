package com.raplt.test;

import android.app.Activity;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.widget.*;
import android.graphics.Color;
import android.graphics.Typeface;

public class MainActivity extends Activity {
    static { System.loadLibrary("jni"); }

    private TextView display;
    private Button[] hk = new Button[4]; /* + - × ÷ hook toggles */
    private boolean[] hkOn = new boolean[4];

    private int curA = 0, curB = 0;
    private char curOp = ' ';
    private boolean hasOp = false, hasEq = false;

    /* native */
    private native int nativeAdd(int a, int b);
    private native int nativeSub(int a, int b);
    private native int nativeMul(int a, int b);
    private native int nativeDiv(int a, int b);
    private native int nativeHookAdd();
    private native int nativeHookSub();
    private native int nativeHookMul();
    private native int nativeHookDiv();
    private native int nativeUnhookAdd();
    private native int nativeUnhookSub();
    private native int nativeUnhookMul();
    private native int nativeUnhookDiv();
    private native int nativeUnhookAll();
    private native String nativeVersion();

    @Override
    protected void onCreate(Bundle s) {
        super.onCreate(s);

        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setBackgroundColor(Color.parseColor("#1a1a2e"));
        int pd = dp(16);
        root.setPadding(pd, pd, pd, pd);

        /* ── title ── */
        root.addView(tv("#00d2ff", 20, "RaPLT Calculator", true));

        /* ── display ── */
        display = new TextView(this);
        display.setTextSize(36);
        display.setTextColor(Color.WHITE);
        display.setGravity(Gravity.END);
        display.setPadding(dp(12), dp(16), dp(12), dp(16));
        display.setBackgroundColor(Color.parseColor("#16213e"));
        display.setTypeface(Typeface.MONOSPACE);
        display.setText("0");
        root.addView(display);

        /* ── hook toggle row ── */
        LinearLayout hrow = new LinearLayout(this);
        hrow.setPadding(0, dp(8), 0, dp(8));
        String[] labels = {"+H", "-H", "\u00d7H", "\u00f7H"};
        for (int i = 0; i < 4; i++) {
            final int fi = i;
            hk[i] = new Button(this);
            hk[i].setText(labels[i]);
            hk[i].setTextSize(14);
            hk[i].setTextColor(Color.WHITE);
            hk[i].setBackgroundColor(Color.parseColor("#555555"));
            hk[i].setPadding(dp(8), dp(4), dp(8), dp(4));
            hk[i].setOnClickListener(v -> toggleHook(fi));
            LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(0, dp(40), 1);
            lp.setMargins(dp(2), 0, dp(2), 0);
            hrow.addView(hk[i], lp);
        }
        root.addView(hrow);

        /* ── batch buttons ── */
        LinearLayout brow = new LinearLayout(this);
        brow.setPadding(0, 0, 0, dp(8));
        Button applyAll = new Button(this);
        applyAll.setText("Apply All");
        applyAll.setTextSize(14);
        applyAll.setBackgroundColor(Color.parseColor("#0f3460"));
        applyAll.setTextColor(Color.WHITE);
        applyAll.setOnClickListener(v -> { for (int i = 0; i < 4; i++) { if (!hkOn[i]) toggleHook(i); } });
        brow.addView(applyAll, new LinearLayout.LayoutParams(0, dp(40), 1));

        Button restoreAll = new Button(this);
        restoreAll.setText("Restore All");
        restoreAll.setTextSize(14);
        restoreAll.setBackgroundColor(Color.parseColor("#533483"));
        restoreAll.setTextColor(Color.WHITE);
        restoreAll.setOnClickListener(v -> { nativeUnhookAll(); for (int i = 0; i < 4; i++) { hkOn[i] = false; setHkColor(i); } refresh(); });
        brow.addView(restoreAll, new LinearLayout.LayoutParams(0, dp(40), 1));
        root.addView(brow);

        /* ── keypad ── */
        String[][] keys = {
            {"7","8","9","\u00f7"},
            {"4","5","6","\u00d7"},
            {"1","2","3","-"},
            {"C","0",".","+"}
        };
        for (int r = 0; r < 4; r++) {
            LinearLayout row = new LinearLayout(this);
            for (int c = 0; c < 4; c++) {
                final String k = keys[r][c];
                Button btn = new Button(this);
                btn.setText(k);
                btn.setTextSize(24);
                btn.setTextColor(Color.WHITE);
                btn.setBackgroundColor(Color.parseColor("#0f3460"));
                boolean isOp = k.equals("+")||k.equals("-")||k.equals("\u00d7")||k.equals("\u00f7")||k.equals("C")||k.equals(".");
                if (isOp) btn.setBackgroundColor(Color.parseColor("#16213e"));
                if (k.equals("C")||k.equals(".")) btn.setBackgroundColor(Color.parseColor("#533483"));
                btn.setOnClickListener(v -> keyPress(k));
                LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(0, dp(60), 1);
                lp.setMargins(dp(2), dp(2), dp(2), dp(2));
                row.addView(btn, lp);
            }
            root.addView(row);
        }

        /* ── version ── */
        root.addView(tv("#888888", 12, "RaPLT " + nativeVersion(), false));

        setContentView(root);
    }

    /* ─── helpers ─── */

    private int dp(int n) { return (int)(n * getResources().getDisplayMetrics().density); }

    private TextView tv(String color, int sz, String txt, boolean bold) {
        TextView t = new TextView(this);
        t.setText(txt);
        t.setTextSize(sz);
        t.setTextColor(Color.parseColor(color));
        if (bold) t.setTypeface(Typeface.DEFAULT_BOLD);
        return t;
    }

    private void setHkColor(int i) {
        hk[i].setBackgroundColor(Color.parseColor(hkOn[i] ? "#00d2ff" : "#555555"));
        hk[i].setTextColor(Color.parseColor(hkOn[i] ? "#1a1a2e" : "#ffffff"));
    }

    private void keyPress(String k) {
        if (k.equals("C")) { clear(); return; }

        String ops = "+-\u00d7\u00f7";

        if (ops.contains(k)) {
            if (hasEq) {
                /* result already shown, use it as curA */
                String[] parts = display.getText().toString().split(" = ");
                try { curA = Integer.parseInt(parts[1].trim()); } catch (Exception e) { curA = 0; }
                hasEq = false;
            } else if (hasOp) {
                /* compute previous operation */
                curB = (int)num();
                int r = calc(curOp, curA, curB);
                display.setText(curA + " " + curOp + " " + curB + " = " + r);
                curA = r;
            } else {
                curA = (int)num();
            }
            curOp = k.charAt(0);
            hasOp = true;
            return;
        }

        /* number input */
        if (hasEq) { clear(); }
        String cur = display.getText().toString();
        if (hasOp) {
            String right = cur.contains(" ") ? cur.substring(cur.lastIndexOf(' ') + 1) : cur;
            if (right.equals("0")) right = k; else right += k;
            display.setText(curA + " " + curOp + " " + right);
        } else {
            if (cur.equals("0")) cur = k; else cur += k;
            display.setText(cur);
        }
    }

    private long num() {
        String s = display.getText().toString().replaceAll(".* ", "");
        try { return Long.parseLong(s); } catch (Exception e) { return 0; }
    }

    private void clear() { display.setText("0"); curA = curB = 0; curOp = ' '; hasOp = hasEq = false; }

    private int calc(char op, int a, int b) {
        switch (op) {
            case '+': return nativeAdd(a, b);
            case '-': return nativeSub(a, b);
            case '\u00d7': return nativeMul(a, b);
            case '\u00f7': return b == 0 ? 0 : nativeDiv(a, b);
            default: return 0;
        }
    }

    private void refresh() {
        if (!hasOp) return;
        int r = calc(curOp, curA, (int)num());
        display.setText(curA + " " + curOp + " " + (int)num() + " = " + r);
        hasEq = true;
    }

    /* after op key → compute */
    private String opLabel(int i) { return new String[]{"+H","-H","\u00d7H","\u00f7H"}[i]; }

    private void toggleHook(int i) {
        if (hkOn[i]) {
            Button[] b = {null,null,null,null};
            if (i == 0) nativeUnhookAdd();
            else if (i == 1) nativeUnhookSub();
            else if (i == 2) nativeUnhookMul();
            else if (i == 3) nativeUnhookDiv();
        } else {
            int r = 0;
            if (i == 0) r = nativeHookAdd();
            else if (i == 1) r = nativeHookSub();
            else if (i == 2) r = nativeHookMul();
            else if (i == 3) r = nativeHookDiv();
        }
        hkOn[i] = !hkOn[i];
        setHkColor(i);
    }
}
