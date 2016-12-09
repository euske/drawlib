//
//  DrawLib.java
//  ウィンドウに図形を描くためのライブラリ関数。
//  by Yusuke Shinyama
//
import java.io.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import javax.imageio.*;


//  DrawLib クラス
//
public class DrawLib extends Canvas
    implements WindowListener, KeyListener {

    // 内部の画像バッファ。
    private BufferedImage _image;
    // バッファ用 GC。
    private Graphics _graphics;
    // 現在押されているキーコード。
    private int _key;
    // ウィンドウの状態。
    private boolean _open;

    // DrwaLib(title, width, height)
    //   与えられたタイトル・幅・高さでウィンドウを開く。
    public DrawLib(String title, int width, int height) {
	_image = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);
	_graphics = _image.getGraphics();
	setSize(width, height);
	// ウィンドウの枠を作成・表示。
	// (実際には、Canvas クラス全体がウィンドウで囲まれる)
	Frame frame = new Frame();
	frame.addKeyListener(this);
	frame.addWindowListener(this);
	frame.setTitle(title);
	frame.setResizable(false);
	frame.add(this);
	frame.pack();
	frame.setVisible(true);
    }

    /// 内部的に使用されるメソッド。
    ///

    // update(g): ウィンドウ内部を再描画するときに呼ばれる。
    public void update(Graphics g) {
	g.drawImage(_image, 0, 0, null);
    }
    
    // paint(g): ウィンドウ内部を再描画するときに呼ばれる。
    public void paint(Graphics g) {
	super.paint(g);
	update(g);
    }

    // keyPressed(e): キーが押されたときに呼ばれる。
    public void keyPressed(KeyEvent e) {
	_key = e.getKeyCode();
    }
    // keyReleased(e): キーが離されたときに呼ばれる。
    public void keyReleased(KeyEvent e) {
	if (_key == e.getKeyCode()) {
	    _key = 0;
	}
    }
    public void keyTyped(KeyEvent e) { }

    // windowOpened(): ウィンドウが表示されたときに呼ばれる。
    public void windowOpened(WindowEvent e) {
	e.getWindow().requestFocusInWindow();
	_open = true;
    }
    // windowClosing(): ウィンドウが閉じられるときに呼ばれる。
    public void windowClosing(WindowEvent e) {
	e.getWindow().dispose();
	_open = false;
    }
    public void windowClosed(WindowEvent e) { }
    public void windowActivated(WindowEvent e) { }
    public void windowDeactivated(WindowEvent e) { }
    public void windowIconified(WindowEvent e) { }
    public void windowDeiconified(WindowEvent e) { }

    /// 実際の API。
    ///
    
    // setColor(R値, G値, B値): 現在の色を設定する。
    public void setColor(int r, int g, int b) {
	_graphics.setColor(new Color(r, g, b));
    }

    // fill(x座標, y座標, 幅, 高さ): 指定された位置・大きさの矩形を描画する。
    public void fill(int x, int y, int w, int h) {
	_graphics.fillRect(x, y, w, h);
    }

    // clear(): 画面を消去する。
    public void clear() {
	_graphics.fillRect(0, 0, _image.getWidth(), _image.getHeight());
    }

    // pset(x座標, y座標, RGB値): 指定された位置にピクセルを描画する。
    public void pset(int x, int y, int c) {
	_image.setRGB(x, y, c);
    }

    // pget(x座標, y座標): 指定されたピクセルの RGB値を返す。
    public int pget(int x, int y) {
	return _image.getRGB(x, y);
    }

    // refresh(): 画像バッファの変更を反映する。
    public void refresh() {
	repaint();
    }

    // sleep(ミリ秒): 指定された時間だけ待つ。
    public void sleep(int msec) {
	try {
	    Thread.sleep(msec);
	} catch (InterruptedException e) {
	}
    }
    
    // getKey(): 現在押されているキーの番号を返す。
    public int getKey() {
	return _key;
    }

    // isOpen(): ウィンドウが開いていれば true を返す。
    public boolean isOpen() {
	return _open;
    }

    // loadImage(ファイル名, x座標, y座標):
    //    画像ファイルを読み込み、指定された位置に表示する。
    public void loadImage(String path, int x, int y) {
	try {
	    Image img = ImageIO.read(new File(path));
	    _graphics.drawImage(img, x, y, null);
	} catch (IOException e) {
	}
    }

    // saveImage(ファイル名):
    //    現在のウィンドウの内容を画像ファイルとして保存する。
    public void saveImage(String path) {
	int i = path.lastIndexOf('.');
	if (0 < i) {
	    try {
		ImageIO.write(_image, path.substring(i+1), new File(path));
	    } catch (IOException e) {
	    }
	}
    }

    /// テスト用プログラム
    ///
    public static void test1() {
        DrawLib d = new DrawLib("test1", 400, 300);
	d.setColor(255, 255, 255);
	d.clear();
        d.setColor(255, 0, 0);
        d.fill(50, 100, 200, 100);
        d.setColor(0, 0, 255);
        d.fill(150, 150, 150, 100);
    }
    
    public static void test2() {
        DrawLib d = new DrawLib("test2", 600, 400);
	int y, vy;
	y = 0;
	vy = 1;
	for (int x = 0; x < 500; x++) {
	    d.setColor(255, x % 256, 0);
	    d.fill(x, y, 100, 100);
	    if (y <= 0) {
		vy = 1;
	    } else if (300 <= y) {
		vy = -1;
	    }
	    y = y + vy*4;
	    d.refresh();
	    d.sleep(20);
	}
    }
    
    public static void main(String[] args) {
	test1();
    }
}
