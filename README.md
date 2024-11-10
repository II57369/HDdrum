# HDdrum
读取硬盘指定扇区，发出特定节奏声音  Read specified sectors of the hard drive and make specific rhythmic beats


使用方法：
首先输入物理驱动器号，可在任务管理器中找到。
"load score from score.txt?(Y/N)" 输入y或Y加载 drum.exe 程序同目录下的 SCORE.TXT 。其中第1行表示BPM，从第2行开始到最后一行是谱子（每行不超过1024^2个字符），忽略回车。
输入其他字符或回车则在终端手动输入BPM、谱子，此时谱子只能输入1行。


'#' 表示1拍， '\*' 表示半拍， '-' 表示四分之一拍；
'M' 表示空1拍， 'N' 表示空半拍，'n' 表示空四分之一拍；
'x.' 表示在 'x' 表示的拍数乘1.5，比如 '\*.' 表示0.75拍（类似于附点）；
其他字符会直接忽略，在终端中输出成一字符的空格。
