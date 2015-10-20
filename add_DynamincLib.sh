#/bin/sh
path="/usr/local/Library/boost"


list_exe=$(ls -F current_head/bin/|grep -v -E "\.fel|\.xml|\.pdf|\.sh|\.yaml|/")
#echo ${list_exe}
for element in ${list_exe}
do
#echo ${element%?}
list_libboost=$(otool -L current_head/bin/${element%?}| grep libboost)
for ele in ${list_libboost}
do
if [ ${ele:0:8} = "libboost" ]; then
#echo ${ele}
install_name_tool -change ${ele} ${path}/lib/${ele} current_head/bin/${element%?}
fi
done
done
#str=("current_head/lib/libasmlib.dylib")
#str=("/usr/local/Library/boost/lib/libboost_filesystem.dylib")
#str=("/usr/local/Library/boost/lib/libboost_chrono.dylib")
#str=("/usr/local/Library/boost/lib/libboost_timer.dylib")
#str=("/usr/local/Library/boost/lib/libboost_thread.dylib")
#str=("current_head/lib/libjzplib.dylib")
#str=("current_head/lib/libasmlib.dylib" "${path}/lib/libboost_filesystem.dylib" "${path}/lib/libboost_chrono.dylib" "${path}/lib/libboost_timer.dylib" "${path}/lib/libboost_thread.dylib" "current_head/lib/libjzplib.dylib")
str="current_head/lib/libasmlib.dylib ${path}/lib/libboost_filesystem.dylib ${path}/lib/libboost_chrono.dylib ${path}/lib/libboost_timer.dylib ${path}/lib/libboost_thread.dylib current_head/lib/libjzplib.dylib"

for element in ${str}
do
#echo ${element}
        list_libboost=$(otool -L ${element}| grep libboost)
        for ele in ${list_libboost}
        do
                if [ ${ele:0:8} = "libboost" ]; then
                        #echo ${ele}
                        install_name_tool -change ${ele} ${path}/lib/${ele} ${element}
                fi
        done

done
