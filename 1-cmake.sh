mkdir .build 
cd .build 
cmake -DCMAKE_BUILD_TYPE=Release -DBENCHMARK_ENABLE_LTO=true -G Ninja ..