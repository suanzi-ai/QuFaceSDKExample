# QuFace SDK 样例

### 获取项目代码

可以直接[下载](https://github.com/suanzi-ai/QuFaceSDKExample/archive/master.zip)。

或者也可以直接克隆方式：

```bash
sudo apt install git git-lfs
git clone https://github.com/suanzi-ai/QuFaceSDKExample.git
```

### 编译依赖：

```bash
sudo apt install ninja-build
pip install cmake
```

### 编译

```bash
cd QuFaceSDKExample
./build.sh --build-type=Debug --build-dir=$(pwd)/build --ninja
```

##### 编译目录说明

编译完的DEMO可执行文件等存放在 build 目录下

- face_compare: 人脸比对样例
- face_compare_multi: 多人脸比对样例
- face_detect: 人脸检测样例
- face_db_server: 人脸库远程更新样例
- face_query: 人脸检索样例

### 运行

运行之前，请校准系统时间：

```bash
ntpd -qNn -p ntp4.aliyun.com # 校准系统时间
```


编译完成后，脚本将提示获取设备激活的文档，或者您也可以直接从 [QuFaceSDK 官网](https://www.quvision.com/) 登录后获取设备激活的账号信息。

然后按提示，拷贝相关文件至开发板，与获取后的账号信息，放在运行目录即可运行程序进行测试。
