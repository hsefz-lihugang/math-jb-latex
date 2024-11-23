# 使用 $\LaTeX$ 重新编写的高中数学精编（上海版）

请注意，此项目是由民间维护的，仅供用于学习 $\LaTeX$ 语法，请在下载后 $24$ 小时内删除

相比于原书，本项目：
- 允许用户自定义题目之间的空隙，便于打草稿
- 允许按章节生成 PDF，方便查看和打印
- 使用 `PGF/TikZ` 重绘了所有矢量图，为所有矢量图提供了单独的文件，便于讲解

### 如何参与贡献

在参与贡献前，请确保您本地安装了以下软件，并正确设置了环境变量 PATH:
- $\LaTeX$ (xelatex) (推荐使用 TexLive， 从这下载：[https://mirrors.tuna.tsinghua.edu.cn/CTAN/systems/texlive/Images/texlive2024-20240312.iso](https://mirrors.tuna.tsinghua.edu.cn/CTAN/systems/texlive/Images/texlive2024-20240312.iso)，下载后挂载镜像，如果您使用的操作系统为 Windows，请双击 `install-tl.bat`，如果您使用的是 Linux，请执行 `./install-tl`，或使用包管理器进行安装)
- Git (对于 Windows 用户，请点击这下载：[https://git-scm.com/download/win](https://git-scm.com/download/win)，对于 Linux 用户，请使用包管理器进行安装)
- Bash-like Shell (对于 Windows 用户，可以使用 Git 自带的 Git Bash，对于 Linux 用户，用默认的 Shell 即可)

为了更好地维护本项目，我们维护了一套工具链，其源码在 `toolchain` 文件夹下，在参与贡献前，请先编译和安装该工具，同时我们也在 Issues 中提供了该工具在常用操作系统下的可执行文件，对于无法编译或配置编译环境有较大困难的用户，可以直接从 Issues 中下载可执行文件。

如果你并不擅长 $\LaTeX$ 但又希望对本项目作出贡献，可以参考：[https://oi-wiki.org/tools/latex/](https://oi-wiki.org/tools/latex/#%E5%85%AC%E5%BC%8F)，我们并不推荐使用 `TexLive` 自带的编辑器 `TexWorks`，推荐使用 `vscode` + `latex-workshop` 插件或 `vim`.

#### 如何构建工具

1. 请确保您本地安装了 `C++` 编译器，推荐使用 `GNU/GCC`，对于使用 Windows 操作系统的用户，可以从这下载 MinGW 套件：[https://github.com/niXman/mingw-builds-binaries/releases/download/14.2.0-rt_v12-rev0/x86_64-14.2.0-release-posix-seh-ucrt-rt_v12-rev0.7z](https://github.com/niXman/mingw-builds-binaries/releases/download/14.2.0-rt_v12-rev0/x86_64-14.2.0-release-posix-seh-ucrt-rt_v12-rev0.7z)，解压后将文件夹内的 `bin`、`include`、`lib`、`share` 子文件夹添加到环境变量 `Path` 中即可。

2. 请确保您本地的 `include` 文件夹中有 `cJSON` 和 `libcurl` 的库。
3. 对于 Windows 用户，请执行 `g++ ./toolchain/main.cpp -o math -std=c++20 -O2 -Wall -lcJSON -lcurl -lws2_32` 以编译，编译成功后当前目录下应该会出现 `math.exe`
4. 对于 Linux 用户，请执行 `g++ ./toolchain/main.cpp -o math -std=c++20 -O2 -Wall -lcJSON -lcurl` 以编译，编译成功后当前目录下应该会出现名为 `math` 的可执行文件
5. 如果你的命令行不支持颜色字符，请在编译时传入 `-DNO_COLOR` 选项

#### 如何导出 PDF

- 全量导出：`./math export full`
- 导出指定章节：`./math export $chapter.$section`，如：导出第一章第一节，执行 `./math export 1.1`
- 如果你正在编写某一章节的源代码并想预览的话，请执行：`./math preview $chapter.$section`

`preview` 和 `export` 的差别是 `preview` 会使用上次编译时的缓存，这可能会导致一些错误，因此在 `export` 时缓存是被禁用的

如果你想修改题目间距，请在导出时传入 `--spacing.$type=$value`，`$type` 可选的值有 `choice`（选择题），`cloze`（填空题），`answerQuestion`（解答题）

对于 `$type` = `choice`/`cloze`，`value` ，`$value` 可以为 `$number.$unit`，其中 `$number` 为数字，`$unit` 为单位，可以为 `cm` / `mm` / `pt` / `inch`

对于 `$type` = `answerQuestion` ，`$value` 可以为 `spacing-enabled` 或 `spacing-disabled`，前者代表留空，后者代表不留空

如：导出第一章第一节，其中每道选择题中留空 `3cm`，每道填空题中留空 `5cm`，解答题留空，则执行 `./math export 1.1 --spacing.choice=3cm --spacing.cloze=5cm --spacing.answerQuestion=spacing-enabled`

如果你想导出某一章下面的所有节，传入 `$chapter` 即可，如导出第一章：`./math export 1`

特别地，如果你预览想禁用缓存，请传入 `--cache=disabled`

**如果你使用的是 Windows 系统，上文所有命令中的 `math` 都应替换为 `math.exe`**

导出的文件在 `result` 文件夹下。

#### 如何编写新章节

请创建文件： `src/$chapter/$section/main.tex` 并直接在此文件中编写 $\LaTeX$ 源代码即可，请注意，只需要编写题目，不需要引包，调配置，初始化文档等操作，这些操作都会由工具自动完成。
文件内容应类似于：
```latex
\begin{enumerate}
    \item \choice{$y=a^x (a>0, a\neq 1)$ 过定点 $(x, y)$}{$(0, 0)$}{$(0, 1)$}{$(1, 1)$}{$(1, 0)$}
    \item \cloze{若 $f(x) = x^2 + 2x + 1$，则 $f\prime(x) = \blank $}
    \item \question{若 $f(x) = ax^2 + 3x + 4$ 在 $(0, +\infty)$ 上恒成立，求 $a$ 的取值范围.}{\newpage}
\end{enumerate}
```

其中 `\choice`，`\cloze`，`\question` 宏由工具根据传入参数自动展开。`\choice` 为选择题，第 1 个参数为题面，第 2 ~ 5 个参数为选项。`\cloze` 为填空题，`\question` 为解答题，第一个参数为题面，第二个参数为空行（请用 `\newpage` 或 `\vspace{\stretch{$size}}` 调整一页最多能放多少题。）

如果题目中有图片，请建立文件夹：`src/$chapter/$section/graphs/` 并将图片的源代码按题目编号命名，如：`src/1/1/graphs/1.tex`，在题目中使用 `\useImage{1}` 即可调用图片，`\useImage` 宏由工具自动展开。如果你不是很擅长使用 `PGF/TikZ` 进行画图，也可以留空，交绘图的任务交给其他贡献者，在这种情况下，请使用 `\textcolor{red}{TODO}` 以标记。**书上的所有矢量图都应用 `PGF/TikZ` 进行重绘，请不要上传照片或用其他工具生成的图片，如果你的提交中有照片或其他图片，你的提交会被 Reject。**

在完成一章的编写后，请使用 `./math commit $chapter.$section --message=$message`，该操作会自动生成章节的元信息并提交到 GitHub 上，章节的状态会被标记成 `unverified`，需要等待其他贡献者审核章节是否出现错误。`--message=$message$` 为提交信息，为可选选项，最多为 $100$ 字。

#### 如何审核新章节

执行：`./math preview $chapter.$section` 然后在 `result` 文件夹中查看 PDF 即可。

如果代码无误，请执行：`./math accept $chapter.$section`。如果有误，请帮忙改正并重新执行 `./math commit $chapter.$section $message`，或是在 GitHub 上发出 Issue 等待其他贡献者修正。

支持多人进行审核。

只有一个新章节并审核过后，才会出现在导出的文件中。

### 约定

1. `amssymb` 、 `amsmath` 和 `xeCJKfntef` 宏包默认导入。
2. 对于实数集等常用数集，请使用 `\mathbb{}` 包裹，如：`\mathbb{R}, \mathbb{Z}, \mathbb{C}`，效果为：$\mathbb{R}, \mathbb{Z}, \mathbb{C}$

3. 对于空集，请使用 `\varnothing` 而不是 `\emptyset`，`\varnothing` 的效果为 $\varnothing$，`\emptyset` 的效果为 $\emptyset$.
4. 对于行内公式，请使用 `$x^2+y^2=z^2$`；对于行间公式，请使用 `$$x^2+y^2=z^2$$`.
5. 对于大括号，请使用
```latex
\left\{
    \begin{aligned}
        a + b & = & 1 \\
        c & = & 2
    \end{aligned}
\right.
```
效果如下：
$$
\left\{
    \begin{aligned}
        a + b & = & 1 \\
        c & = & 2
    \end{aligned}
\right.
$$
6. 对于向量，请使用 `\overrightarrow` 而不是 `\vec`，`\overrightarrow` 的效果为 $\overrightarrow{a}, \overrightarrow{AB}$，`\vec` 的效果为 $\vec{a}, \vec{AB}$.
7. 对于分数，请使用 `\dfrac` 而不是 `\frac`，`\dfrac` 的效果为 $\dfrac{1}{2}$，`\frac` 的效果为 $\frac{1}{2}$.
8. 对于大于等于和小于等于号，请使用 `\geqslant` 和 `\leqslant` 效果为：$\geqslant, \leqslant$
9. 请不要修改 `template` 和 `workspaceData` 文件夹里的内容，这可能会导致工具不能正常运行。

这些约定可能会随时添加和修改，请关注您的邮箱。