# 簡單的圍棋部件

這是一個以 C++ 實做的間單圍棋部件，包含圍棋棋盤基本演算法和完整的 GTP 實做。你可以將棋盤用於你自己的 AI 實做中，或是直接加入神經網路實做，將它擴充成更強的圍棋引擎。

# 編譯

需要支援 C++11 或以上的編譯器

    git clone https://github.com/CGLemon/GoComponent
    cd GoComponent
    g++ src/*.cc -o bot -std=c++11

# 測試

它進入的模式是 GTP ，你可以通過此和它溝通。一些指令說明可以直接進入程式觀看提示或是查看[這裡](https://github.com/CGLemon/pyDLGO/blob/master/docs/dlgoGTP.md)。

# 其它

Python 版本的完整實做請看[這裡](https://github.com/CGLemon/pyDLGO)，此實做包含神經網路和蒙地卡羅樹搜索。
