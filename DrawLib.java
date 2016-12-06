//  DrawLib.java
//
import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;


//  DrawLib
//
public class DrawLib extends Frame
    implements ActionListener, WindowListener, KeyListener {

    private BufferedImage _image;
    private Graphics _graphics;

    public DrawLib(String title, int width, int height) {
	_image = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);
	_graphics = _image.getGraphics();
	addKeyListener(this);
	addWindowListener(this);
	setTitle(title);
	setSize(width, height);
	setResizable(false);
	setVisible(true);
    }

    public void init() {
	System.out.println("init");
    }

    public void update() {
	System.out.println("update");
    }

    public void paint(Graphics g) {
	super.paint(g);
	System.out.println("paint");
	g.drawImage(_image, 0, 0, null);
    }

    public void keyPressed(KeyEvent e) { }
    public void keyReleased(KeyEvent e) { }
    public void keyTyped(KeyEvent e) { }

    public void windowOpened(WindowEvent e) {
	requestFocusInWindow();
	init();
	synchronized (this) {
	    notify();
	}
    }
    public void windowClosing(WindowEvent e) {
	dispose();
    }
    public void windowClosed(WindowEvent e) { }
    public void windowActivated(WindowEvent e) { }
    public void windowDeactivated(WindowEvent e) { }
    public void windowIconified(WindowEvent e) { }
    public void windowDeiconified(WindowEvent e) { }
    
    public void actionPerformed(ActionEvent e) {
	update();
    }
    
    public synchronized void fill(int x, int y, int w, int h) {
	System.out.println("fill");
	_graphics.fillRect(x, y, w, h);
    }

    public synchronized void clear() {
	System.out.println("clear");
	_graphics.fillRect(x, y, w, h);
    }

    public synchronized void setColor(int r, int g, int b) {
	_graphics.setColor(new Color(r, g, b));
    }

    public static void main(String[] args) {
	DrawLib window = new DrawLib("test", 400, 300);
	window.setColor(255,0,0);
	window.fill(100, 100, 100, 100);
	window.setColor(0,0,255);
	window.fill(200, 200, 100, 100);
	window.repaint();
    }
}
