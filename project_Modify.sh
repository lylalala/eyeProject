#/bin/sh

#add the boost to this project
#modify ../current_head/src/CMakeList.txt
#path="/usr/local/Library/boost"
#cd current_head
#cd src
#sed -i "" 'PROJECT(JZP_EYE_TRACKING)/a\set(BOOST_ROOT ${path})' CMakeLists.txt
#sed -i  "" "s:PROJECT(JZP_EYE_TRACKING):PROJECT(JZP_EYE_TRACKING)\nset(BOOST_ROOT ${path}):g" CMakeLists.txt
#sed -i  "" ":PROJECT(JZP_EYE_TRACKING):a\set(BOOST_ROOT ${path})" CMakeLists.txt


awk 'FNR<4' test.txt > newfile.txt
echo -e "\t" >>newfile.txt
awk 'FNR<5' test.txt >> newfile.txt