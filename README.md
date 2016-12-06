drawlib
=======

Windows 上で C/Java を使って簡単な図形を描くためのライブラリ。

使用例 (C)
----------

```C
#include <stdio.h>

int main(void)
{
    /* 400×300 のウィンドウを開く。 */
    initwin("test", 400, 300);
    /* 現在の色を赤色に設定。 (R,G,B = 255,0,0) */
    setcolor(255, 0, 0);
    /* 矩形を塗る。 */
    fill(50, 100, 200, 100);
    /* 現在の色を青色に設定。 (R,G,B = 0,0,255) */
    setcolor(0, 0, 255);
    /* 矩形を塗る。 */
    fill(150, 150, 150, 100);
    /* 画面の更新を反映。 */
    refresh();
    return 0;
}
```

使用例 (Java)
-------------

```Java
public class Test1 {
    public static void main(String[] args) {
        /* 400×300 のウィンドウを開く。 */
        DrawLib d = new DrawLib("test", 400, 300);
        /* 現在の色を赤色に設定。 (R,G,B = 255,0,0) */
        d.setColor(255, 0, 0);
        /* 矩形を塗る。 */
        d.fill(50, 100, 200, 100);
        /* 現在の色を青色に設定。 (R,G,B = 0,0,255) */
        d.setColor(0, 0, 255);
        /* 矩形を塗る。 */
        d.fill(150, 150, 150, 100);
        /* 画面の更新を反映。 */
        d.refresh();
    }
}
```

コンパイル方法
--------------

    C:\> cl test1.c drawlib.c
