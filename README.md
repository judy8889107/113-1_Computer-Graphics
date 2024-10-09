# 簡介
圖學HW1: OBJ Loader，已經建立好C++ project，使用`make file`即可運行。  
* 快捷鍵：
  * ESC：Exit
  * Mouse click right：open menu
  * F1: Point
  * F2: Line
  * F3: Fill 

---

# 程式思路
1. 建立一個PTN buffer，但有些PTN組會是重複的，所以對於`Cube.obj`來說，會有$6*2*3=36$ 組PTN，但我們不應該要存36組，因為其中有些是重複的，在儲存PTN時候，應該要注意是否有重複，並為其建立一個hash table之類的indices，查看f P/T/N 的排序去對應其hash table的indices。
    
2. 比較重要的tips是下面這裡stride對應到`sizeof(VertexPTN)`，`offset`則對應到`(void *)offsetof(VertexPTN, position))`，因為有定義`structure VertexPTN`，所以stride要以元素間隔為主，`offset`則是指第n個元素需要的位移量，但`VertexPTN.positio`n不用位移
   ```c
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), (void *)offsetof(VertexPTN, position)); // pos offset is 0
   ```