//  DrawLib.java
//
import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;


//  DrawLib
//
public class DrawLib extends Canvas
    implements ActionListener, WindowListener, KeyListener {

    private BufferedImage _image;
    private Graphics _graphics;

    public DrawLib(String title, int width, int height) {
	_image = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);
	_graphics = _image.getGraphics();
	setSize(width, height);
	Frame frame = new Frame();
	frame.addKeyListener(this);
	frame.addWindowListener(this);
	frame.setTitle(title);
	frame.setResizable(false);
	frame.add(this);
	frame.pack();
	frame.setVisible(true);
    }

    public void init() {
	System.out.println("init");
    }

    public void update() {
	System.out.println("update");
    }

    public void refresh() {
	repaint();
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
	e.getWindow().requestFocusInWindow();
	init();
    }
    public void windowClosing(WindowEvent e) {
	e.getWindow().dispose();
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
	_graphics.fillRect(0, 0, _image.getWidth(), _image.getHeight());
    }

    public synchronized void setColor(int r, int g, int b) {
	_graphics.setColor(new Color(r, g, b));
    }

    public static void main(String[] args) {
        DrawLib d = new DrawLib("test", 400, 300);
        d.setColor(255, 0, 0);
        d.fill(50, 100, 200, 100);
        d.setColor(0, 0, 255);
        d.fill(150, 150, 150, 100);
        d.refresh();
    }
}
