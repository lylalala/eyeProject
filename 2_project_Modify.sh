#/bin/sh

#add the boost to this project
#modify ../current_head/src/CMakeLists.txt
#path="/usr/local/Library/boost"
#cd current_head
#cd src
#sed -i "" 'PROJECT(JZP_EYE_TRACKING)/a\set(BOOST_ROOT ${path})' CMakeLists.txt
#sed -i  "" "s:PROJECT(JZP_EYE_TRACKING):PROJECT(JZP_EYE_TRACKING)\nset(BOOST_ROOT ${path}):g" CMakeLists.txt
#sed -i  "" ":PROJECT(JZP_EYE_TRACKING):a\set(BOOST_ROOT ${path})" CMakeLists.txt


#awk 'FNR<4' test.txt > newfile.txt
#echo -e "\t" >>newfile.txt
#awk 'FNR<5' test.txt >> newfile.txt

choose="dynamic"
#if you want to use static lib, you choose "other"
#if you want to use dynamic lib, you choose "dynamic"
awk 'FNR<5' current_head/src/CMakeLists.txt > newMakeLists.txt
if [ ${choose} = "dynamic" ]; then
        echo -e "\t" >> newMakeLists.txt
        echo "set(BOOST_ROOT /usr/local/Library/boost)" >> newMakeLists.txt
else
        echo -e "\t" >> newMakeLists.txt
        echo "set(BOOST_ROOT /usr/local/Library/boost)" >> newMakeLists.txt
        echo "set(Boost_USE_STATIC_LIBS ON)" >> newMakeLists.txt
fi
awk 'FNR>=5' current_head/src/CMakeLists.txt >> newMakeLists.txt
rm current_head/src/CMakeLists.txt
mv newMakeLists.txt current_head/src/CMakeLists.txt