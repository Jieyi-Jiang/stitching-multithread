# git submodule怎么使用

`git submodule` 主要用来在一个 Git 仓库里嵌套另一个仓库（常用来引入第三方库，比如 yaml-cpp），这样不会直接把第三方代码复制进来，而是保持它的独立性。

我给你按 **使用场景** 来讲，从零开始到日常维护全流程都涵盖。

---

## **1. 添加 submodule**

假设你有一个 C++ 项目，要把 `yaml-cpp` 作为子模块引入到 `extern/` 目录：

```bash
# 进入你的项目根目录
cd MyProject

# 添加子模块
git submodule add https://github.com/jbeder/yaml-cpp extern/yaml-cpp

# 初始化并更新（第一次添加会自动做）
git submodule update --init --recursive
```

这样你的目录结构可能是：

```
MyProject/
  ├── extern/
  │    └── yaml-cpp/   # 这是一个独立的 Git 仓库
  ├── src/
  ├── CMakeLists.txt
  └── ...
```

Git 会在 `.gitmodules` 文件里记录子模块的信息：

```ini
[submodule "extern/yaml-cpp"]
    path = extern/yaml-cpp
    url = https://github.com/jbeder/yaml-cpp
```

---

## **2. 克隆带 submodule 的项目**

如果别人克隆了你的项目，默认 **子模块内容是空的**，需要额外一步：

```bash
git clone https://github.com/you/MyProject.git
cd MyProject
git submodule update --init --recursive
```

或者直接一步到位：

```bash
git clone --recursive https://github.com/you/MyProject.git
```

---

## **3. 更新 submodule 到最新版本**

进入子模块目录，正常执行 Git 操作：

```bash
cd extern/yaml-cpp
git checkout master
git pull
```

然后回到主仓库：

```bash
cd ../..
git add extern/yaml-cpp
git commit -m "Update yaml-cpp submodule to latest"
```

> ⚠ 子模块的版本由主仓库“锁定”在某个 commit，你更新了子模块，需要在主仓库提交一次才能生效。

---

## **4. 切换 submodule 到特定版本**

```bash
cd extern/yaml-cpp
git checkout <commit-hash-or-tag>
cd ../..
git add extern/yaml-cpp
git commit -m "Use yaml-cpp v0.8.0"
```

---

## **5. 删除 submodule**

步骤稍微多一些：

```bash
# 1. 删除子模块目录
git rm -f extern/yaml-cpp

# 2. 删除配置信息
rm -rf .git/modules/extern/yaml-cpp

# 3. 删除 .gitmodules 里对应条目
nano .gitmodules
```

---

✅ **总结**

* **添加** → `git submodule add <url> <path>`
* **克隆并初始化** → `git clone --recursive ...` 或 `git submodule update --init --recursive`
* **更新** → 进入子模块目录 `git pull`，再回到主仓库提交
* **删除** → `git rm -f` + 清理 `.gitmodules`

---

如果你愿意，我可以帮你直接做一个 **CMake + yaml-cpp + git submodule** 的最小工程模板，你拉下来就能编译运行。
这样你不需要自己配置 submodule 和 CMake 链接逻辑。
