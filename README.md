# 使用说明
## 1. 编译与运行

* 克隆本项目：
```bash
    复制代码
    git clone https://github.com/puoxiu/tcp_scan.git
```
* 进入项目目录并编译：

```bash
    cd tcp_scan
    mkdir build
    cd build
    cmake ..
    make
```
* 运行开始扫描：

```bash
    ./tcp_scan
```

## 2. 配置
你可以修改 main.cpp 中的 ip 地址和端口范围来扫描不同的目标。例如，修改 ip 变量来扫描不同的主机，修改端口范围（例如 21 到 1024）来扫描不同的端口(注意遵守法律)