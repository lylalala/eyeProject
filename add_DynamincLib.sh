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
