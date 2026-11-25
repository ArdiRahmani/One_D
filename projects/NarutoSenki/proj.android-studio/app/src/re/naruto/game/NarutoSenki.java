
package re.naruto.game;

import android.content.Context;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.WindowManager;

import org.cocos2dx.lib.Cocos2dxActivity;
import org.cocos2dx.lib.Cocos2dxGLSurfaceView;

public class NarutoSenki extends Cocos2dxActivity {

    private static final int IMMERSIVE_FLAGS =
            0x00000004  // FULLSCREEN
            | 0x00000002  // HIDE_NAVIGATION
            | 0x00001000  // IMMERSIVE_STICKY
            | 0x00000100  // LAYOUT_STABLE
            | 0x00000400  // LAYOUT_FULLSCREEN
            | 0x00000200; // LAYOUT_HIDE_NAVIGATION

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().setFlags(
            WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
            WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON
        );

        hideSystemUI();
    }

    private void hideSystemUI() {
        getWindow().getDecorView().setSystemUiVisibility(
            IMMERSIVE_FLAGS
        );
    }

    @Override
    protected void onResume() {
        super.onResume();
        hideSystemUI();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);

        if (hasFocus) {
            hideSystemUI();
        }
    }

    public Cocos2dxGLSurfaceView onCreateGLSurfaceView() {
        return new LuaGLSurfaceView(this);
    }

    static {
        System.loadLibrary("cocos2dcpp");
    }
}

class LuaGLSurfaceView extends Cocos2dxGLSurfaceView {

    public LuaGLSurfaceView(Context context) {
        super(context);
    }

    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            android.os.Process.killProcess(
                android.os.Process.myPid()
            );
        }

        return super.onKeyDown(keyCode, event);
    }
}
