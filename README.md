# 作業簡介

圖學HW1: OBJ Loader，已經建立好C++ project，使用`make file`即可運行。 

* 快捷鍵：
  * ESC：Exit
  * Mouse click right：open menu
  * F1: Point
  * F2: Line
  * F3: Fill 

※ 使用VSCode建立環境方法： https://blog.csdn.net/plairlli/article/details/129354055

---

# 程式簡介

## 一、讀取obj並更新資料
1. 首先先將`v、vt、vn`的資訊分別加入到三個vector中，以便之後用於建立`VertexPTN`。
2. 更新`std::vector<VertexPTN> vertices;`，但有些PTN組會是重複的，對於`Cube.obj`來說，會有$6*2*3=36$ 組PTN，但我們不應該要存$36$組，因為其中有些是重複的，在儲存PTN時候，應該要注意是否有重複，並為其建立一個`int findVertexPTNIndex(VertexPTN VertexPTN) const;`去找出在`vertices`中的index，若沒有找到則加入vector中，並`return -1`。
3. 為了解決多邊形需要分解成多個三角形的問題，建立一個`std::vector<unsigned int> polyIndices;`，讀取每一行f就會先將VertexPTN的index存入，若先分解的話，這`findVertexPTNIndex`要做很多次，會讓效能降低。
4. 針對`polyIndices`處理好分解三角形的問題，再存入`vertexIndices`中。

※ 在`structure VertexPTN`多寫方法`isEqual()`去比較兩`VertexPTN`物件

## 二、標準化所有頂點
1. 有了`vertices`中的所有頂點，我們可以找出minVertex和maxVertex，並計算出Bounding box以後，將最長邊縮放為1，讓所有`vertex.position`都進行縮放。
2. 中心點的計算則是`(minVertex+maxVertex)/2`以後，進行縮放得到新的中心點。
3. 最後，將所有點減去新的中心點，會將整個模型移動道中心位置。

## 三、建立buffer和render
1. 建立`&vboId`和`&iboId` buffer
2. Render中，我認為比較重要的是下面這裡stride對應到`sizeof(VertexPTN)`，`offset`則對應到`(void *)offsetof(VertexPTN, position))`，因為有定義`structure VertexPTN`，所以stride要以元素間隔為主，`offset`則是指第n個元素需要的位移量，但`VertexPTN.position`不用位移。
   ```c
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), (void *)offsetof(VertexPTN, position)); // pos offset is 0
   ```

## 四、資源釋放
1. 在`ReleaseResources()`這一塊，我是將前面建立的`&vboId`和`&iboId` buffer刪除，並把`mesh`物件也刪除。(不是很確定是不是這樣寫)

## 五、UI介面
1. 這邊我沒有增加快捷鍵，而是用右鍵彈跳出menu的方式，讓使用者可以切換models(但menu是寫死的，沒辦法動態新增obj檔案後就更新在上面，因為好像要用fileSystem header)。
2. 介面上方增加熱鍵提示。

## 六、結果(截圖)

![結果(截圖)](result.png "結果(截圖)")