# Compiler Final Project: Mini-LISP
**姓名：洪啟展
系級：資工三B
學號：112502548**

## 檔案路徑
```text
├─ finalproject.l      # Lexer 定義檔
├─ finalproject.y      # Parser 定義檔
├─ node.c              # AST 節點結構與函數原型的標頭檔
├─ node.h              # AST 節點操作與運算邏輯的實作檔
└─ readme.md           # 這份檔案
```
## 建置與執行
以在 Visual Studio Code 終端機的 powershell 為例。
#### 編譯專案
```bash
flex finalproject.l
bison -d finalproject.y
gcc lex.yy.c finalproject.tab.c node.c -o execution
```
#### 執行專案
- 一次執行所有側資
    ```bash
    cmd.exe /C "for %f in (public_test_data\*.lsp) do execution.exe < %f > output\%~nf.out"
    ```
- 執行單筆測資（檔名需自行替換）
  ```bash
  cmd.exe /C "execution.exe < {FileName}.lsp > {FileName}.out"
  ```

## 功能實作解釋
### 1. Syntax Validation

當輸入的程式碼不符合 LISP 語法結構或規則時，程式會直接輸出 `syntax error` 並終止 AST 的建構。

### 2. Print
* **LISP 範例**:
`(print-num EXP)`
`(print-bool EXP)`
* **AST 結構**:

  ```text
      NODE_STMT_PRINT_NUM
              |
            Left
              |
            EXPR 
  ```

* **運作過程**:

  1. `traverse` 函數進入 `NODE_STMT_PRINT_NUM` 節點。
  2. `traverse` 先走訪左節點，將底層數值計算出來。
  3. 存取 `father->left->value` 獲取計算結果。
  4. 使用 `printf` 將整數或是 `#t` / `#f` 印出。

### 3. Numerical Operations
* **LISP 範例**:
`(+ 1 2 ...)`
`(- 1 2)`
`(* 1 2 ...)`
`(/ 1 2)`
`(mod 1 2)`
`(> 1 2)`
`(< 1 2)`
`(= 1 2 ...)`
其中 `+`、`*`、`=` 支援多參數。

* **AST 結構**:
    * 以加法為例
      ```
              NODE_OP_ADD
             /          \
           Left        Right
            |            |
         NODE_INT    NODE_LIST (多參數列表)
           (1)      /         \
                NODE_INT    NODE_INT
                  (2)          (3)
      ```
    * 以減法為例
      ```
              NODE_OP_SUB
             /          \
           Left        Right
            |            |
         NODE_INT     NODE_INT
           (1)           (2)
      ```

* **運作過程**:

  1. `traverse` 函數進入 `NODE_OP_{}` 系列節點。
  1. `traverse` 先走訪左節點和右節點，將底層數值計算出來。
  2. 
      - 在支援多參數的函數如 `do_add` 中，會檢查子節點是否為 `NODE_LIST`，如果是則遞迴呼叫 `do_add` 自身，直到所有列表中的數字都被累加。
      - 在不支援多參數的函數如 `do_sub` 中，則直接將左節點與右節點數字相減。
  4. 運算結果存入全域變數 `cal_result`，最後賦值給當前節點的 `value` 屬性。

### 4. Logical Operations
* **LISP 範例**:
`(and #t #f ...)`
`(or #t #f ...)`
`(not #t)`
其中 `and` 和 `or` 支援多參數。

* **AST 結構**:
  與 _Numerical Operations_ 類似。
  
* **運作過程**:
  1. `traverse` 函數進入 `NODE_OP_AND`、`NODE_OP_OR`、`NODE_OP_NOT` 節點。
  1. 
      - **AND 和 OR**: `traverse` 先走訪左節點和右節點，將底層數值計算出來。
      - **NOT**: `traverse` 先走訪左節點，將底層數值計算出來。
  3. 
      - **AND**: 初始化 `cal_result = 1`，呼叫 `do_and` 遍歷所有子節點，只要有一個為 0，結果即為 0。
      - **OR**: 初始化 `cal_result = 0`，呼叫 `do_or` 遍歷所有子節點，只要有一個為 1，結果即為 1。
      - **NOT**: 直接將左子節點的 `value` 取反。
  4. 運算結果存入全域變數 `cal_result`，最後賦值給當前節點的 `value` 屬性。

### 5. If Expression
* **LISP 範例**:
`(if #t 1 2)`
`(if #f 1 2)`

* **AST 結構**:
  額外使用 `middle` 節點當作 then 時的動作。
  ```text
            NODE_STMT_IF
          /      |      \
      Left    Middle    Right
     (Test)   (Then)    (Else)
       |         |        |
      EXPR      EXPR     EXPR
  ```

* **運作過程**:
  1. `traverse` 函數進入 `NODE_STMT_IF` 節點。
  1. `traverse` 先走訪左節點、中間節點和右節點，將底層數值計算出來。
  2. 檢查 `father->left->value`：
     * 若為 **1**: 將中間節點 (Then) 的值賦給父節點。
     * 若為 **0**: 將右節點 (Else) 的值賦給父節點。

### 6. Variable Definition
* **LISP 範例**:
`(define x 1)`
`(define foo1 (fun (y) y))`
`(define foo2 (fun () 2))`

* **AST 結構**:
    - 以定義變數為例
      ```text
            NODE_STMT_DEF
            /           \
         Left          Right
           |             |
       NODE_VAR         EXPR
      ```
    - 以定義函數為例
      ```text
            NODE_STMT_DEF
            /           \
         Left          Right
           |             |
       NODE_VAR    NODE_FUNC_EXPR
      ```

* **運作過程**:
  1. `traverse` 函數進入 `NODE_STMT_DEF` 節點。
  2. 首先根據右節點的類型來決定儲存策略。
  3. * **定義變數** (若右邊是 `NODE_INT`、`NODE_OP_ADD` 等數值表達式):
       1. 立即走訪右節點，將底層數值計算出來。
       2. 將計算結果與變數名稱存入 `var_table`。
     * **定義函數** (若右邊是 `NODE_FUNC_EXPR` ):
       1. 若函數沒有任何參數，則會當作 _定義變數_ 處理。
       1. 不執行函數本體。
       2. 將函數名稱與函數節點指標直接存入 `func_table`。


### 7. Function
* **LISP 範例**:
`((fun (y) y) 1)`
`((fun () 2))`

* **AST 結構**:
    - 函式結構
      ```text
             NODE_FUNC_EXPR
             /            \
          Left           Right
            |              |
       NODE_LIST          EXPR
      (Parameter IDs)  (Function Body)
      ```
    - 匿名呼叫
        ```text
             NODE_FUNC_CALL
             /            \
        NODE_FUNC_EXPR   NODE_LIST
        (Definition)     (Actual Params)
        ```

* **函數結構**:
  `NODE_FUNC_EXPR` 本身在 `traverse` 中通常不會單獨執行任何運算，它的存在是為了提供資訊給呼叫者:
  * **左節點**: 告訴呼叫者這個函數需要多少參數、參數名稱叫什麼。
  * **右節點**: 告訴呼叫者函數的邏輯是什麼。

* **運作過程**:
  1. `traverse` 函數進入 `NODE_FUNC_CALL` 節點。
  2. 檢查函數結構 (左節點的左節點) 是否包含參數，若有參數則綁定參數名稱與數值至 `par_table`。
  3. 開啟函數模式 (此時 `NODE_VAR` 會讀取 `par_table` 而不是 `var_table` )。
  4. `traverse` 函數結構內容 (左節點的右節點)，運算結果賦值給函數結構內容的 `value` 屬性。
  5. 將剛才算出的結果賦值給父節點 `value`。
  6. 關閉函數模式。


### 8. Named Function Call
* **LISP 範例**:
`(foo1 2)`
`(foo2)`

* **AST 結構**:
  ```text
       NODE_FUNC_CALL_NAMED
        /                \
     Left               Right
  NODE_FUNC_NAME      NODE_LIST
  (Function Name)    (Actual Params)
  ```

* **運作過程**:
  1. `traverse` 函數進入 `NODE_FUNC_CALL_NAMED` 節點。
  1. 開始搜尋函數:
     * 若有輸入參數，去 `func_table` 中搜尋，找到 _Variable Definition_ 所儲存的 `NODE_FUNC_EXPR` 節點。
     * 若沒有輸入參數，去 `var_table` 中搜尋，找到 _Variable Definition_ 所儲存的數值。
  2. 將函數結構 `NODE_FUNC_EXPR` 節點的參數綁定參數名稱與數值至 `par_table`。
  3. 開啟函數模式 (此時 `NODE_VAR` 會讀取 `par_table` 而不是 `var_table` )。
  4. `traverse` 函數結構內容 ( `NODE_FUNC_EXPR` 的右節點)，運算結果賦值給函數結構內容的 `value` 屬性。
  5. 將剛才算出的結果賦值給父節點 `value`。
  6. 關閉函數模式。