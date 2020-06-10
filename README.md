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

- face_detect: 人脸检测样例
- face_detect_mscale: 人脸多尺度检测样例
- face_compare: 人脸比对样例
- face_compare_multi: 多人脸比对样例
- face_query: 人脸检索样例
- face_db_server: 人脸库远程更新样例
- head_detect: 人头检测样例

### 运行

#### 连接外网
必须保证设备连接外网，后续的时间校准和设备认证都需要网络。

#### 校准系统时间
运行之前，请校准系统时间：

```bash
ntpd -qNn -p ntp4.aliyun.com # 该校准校准过程可能失败，需要多次运行这条命令
```
校准成功时，可看到类似的信息
```
ntpd: reply from 203.107.6.88: delay 0.857678 is too high, ignoring
ntpd: setting time to 2020-05-13 07:15:47.034944 (offset +1589348731.741161s)
```

#### 设备注册与激活
编译完成后，脚本将提示获取设备激活的文档，或者您也可以直接从 [QuFaceSDK 官网](https://www.quvision.com/) 登录后获取设备激活的账号信息。
在官网成功完成新增设备之后，可获得DeviceName和DeviceSecret。使用这两个信息，创建一个文本文件device_info.json，并填入如下信息：
```
{
  "ProductKey": "a1KqnJecdnC",
  "DeviceName": "上面获取的DeviceName",
  "DeviceSecret": "上面获取的DeviceSecret"
}
```
将该文件拷贝到编译生成的可运行程序所在目录下，如```**/build/Debug/install/```.

#### 运行example程序
然后按提示，拷贝相关文件至开发板，并保证开发板网络通畅，即可运行程序进行测试。

- face_detect: 小场景人脸检测样例，人脸尺寸最好大于图像最大边的1/5，否则可能检测不到人脸
```
./face_detect -a data/lyf1.jpg -b facemodel_vX.bin
```

> `facemodel_vX.bin`： 注意，具体文件名请查看 `resources/models` 内，这里只是举例，下同。

- face_detect_mscale: 人脸多尺度检测样例，用于测试专门针对1080P的检测API
```
./face_detect_mscale  -b facemodel_vX.bin  -a  data/faces5.jpg
./face_detect_mscale  -b facemodel_vX.bin  -a  data/faces6.jpg
```
检测结果图像保存为data/faces1.png_scale_5.jpg和data/faces1.png_scale_7.jpg。

- face_compare: 人脸比对样例
```
./face_compare -c facemodel_vX.bin -a data/szj1.jpg -b data/szj3.jpg
```

- face_compare_multi: 多人脸比对样例
```
./face_compare_multi  facemodel_vX.bin
```

- face_anti_spoofing: 活体检测样例
```
./face_anti_spoofing  facemodel_vX.bin
```
- face_query: 人脸检索样例

- face_db_server: 人脸库远程更新样例


#### 运行head_detect

- head_detect: 人头检测样例【目前该功能不可免费使用】
```
./head_detect -a data/faces1.png  -b headmodel.bin
```
检测结果图像保存为data/faces1.png_scale_0.jpg。