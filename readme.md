#安装使用说明
##1.必要软件下载
* 安装Homebrew。如果已经通过brew安装opencv，被保险起见，卸载brew重新安装
* 卸载 ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/uninstall)"
* 安装 ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)”
* 更新 brew update
* 安装python
* brew install python；确保python安装到了/usr/local/Cellar下
* 如果报错如下：
```javascript
configure: error: in `/private/tmp/python20150928-42030-1orh70/Python-2.7.10':
configure: error: C compiler cannot create executables
```
执行xcode-select —install后安装python
参见网址：https://github.com/Homebrew/homebrew/issues/44405
* 安装opencv
* brew tap homebrew/science
* brew install opencv --with-eigen --with-ffmpeg --with-qt --with-tbb
* 安装X11:安装包附在工程的压缩文件中 

##2.编译boost
* 为了简化安装过程，附录中有shell脚本文件boost_Download_Compile.sh
* boost_Download_Compile.sh放置在current_head所在目录下（这个shell文件和工程文件夹在同级目录下）
* 先修改boost_Download_Compile.sh中的相关配置信息
* version: 是版本选择，默认1.58.0
* path: boost的库文件将会被拷贝到这里，后面进行引用，默认/usr/local/Library/boost
* delete: 拷贝后是否删除Boost压缩包和编译后的工程，默认不删除delete='no'
* 修改后当前目录下执行./boost_Download_Compile.sh

##3.编译工程
* 解压currend_head.zip，为了简化调整编译参数的过程，附录中有shell脚本文件project_Modify.sh
* project_Modify.sh放置在current_head所在目录下（这个shell文件和工程文件夹在同级目录下）
* 先修改参数choose。choose=“dynamic”的话是动态编译，否则是静态编译
* 如果进行静态编译的话，执行完“3.编译工程”就可以结束了，不用执行“4.引入动态库”；如果进行动态编译，则继续下一步，并执行“4.引入动态库”


```javascript
#修改前
PROJECT(JZP_EYE_TRACKING)
include(cmake/OptimizeForArchitecture.cmake)
OptimizeForArchitecture()
include(./dlib/cmake)
......
```
```javascript
#修改后
PROJECT(JZP_EYE_TRACKING)
include(cmake/OptimizeForArchitecture.cmake)
OptimizeForArchitecture()
include(./dlib/cmake)
set(BOOST_ROOT /usr/local/Library/boost)
#静态编译才要添加下面这行
set(Boost_USE_STATIC_LIBS ON)
......
```
* 编译工程，在current_head下执行./directbuild.sh进行编译。至此，工程编译完成，可以执行。如果报错没有引入boost_system等动态库的话，执行第4步

##4.引入动态库（动态编译需要）
* 为了简化过程，附录中有shell脚本文件add_DynamincLib.sh
* add_DynamincLib.sh放置在current_head所在目录下这个shell文件和工程文件夹在同级目录下）



##问题反馈
在使用中有任何问题，欢迎反馈给我，可以用以下联系方式跟我交流

* email: <liuyang070424@gmail.com>
* QQ: 359250464



