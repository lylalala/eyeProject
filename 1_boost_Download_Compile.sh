#/bin/sh
#0.configuration
#the version of boost
version="1.58.0"
#the path you want to copy the boost Library
path="/usr/local/Library/boost"
#do you want to delete the raw boost tar.gz/fold in order to save space after it is copied to the path above
#yes/no
delete="no"

#1.downloads the boost and compiles it 
version2=${version//./_}
echo "The version of boost is boost${version}"
#-O是保存在本地，-C是断点传输
curl -C - -O http://nchc.dl.sourceforge.net/project/boost/boost/${version}/boost_"${version2}".tar.gz
filename=boost_${version2}

#judge if the boost has been downloaded
#-f file
if [ ! -f "${filename}.tar.gz" ]; then
	echo "ERROR:The ${filename}.tar.gz does not exist!"
	exit 0
else
	echo "The ${filename}.tar.gz has been downloaded!"
	tar -xzf ${filename}.tar.gz
fi
cd ${filename}

#judge if the fold has been existed and if it is permitted to operate
#-x是判断是否存在并是否具有可执行权限
if [ ! -x "${path}" ]; then
	mkdir ${path}
fi
echo "The fold has been existed!"
./bootstrap.sh --prefix=${path}

#judge if the the boost find the pythoin
pathPythonInJam=$(grep "python" project-config.jam)
pathArray=($(echo ${pathPythonInJam}))
pathPython=${pathArray[5]}
#-d是判断是否存在
if [ ! -d "${pathPython}" ]; then
        echo "ERROR: The boost can't find thr python27!"
        exit 0;
else
        echo "The boost finds the python27!"
fi


./b2
./b2 install
cd ..

if [ ${delete} = "yes" ]; then
        rm -rf boost_${version2}
        rm boost_${version2}.tar.gz
        echo "The boost has been deleted!"
fi