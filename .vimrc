function! RunCMake()
  execute 'terminal'
  call feedkeys("source ./build_env.sh\<CR>")
  call feedkeys("cd ./build\<CR>")
  call feedkeys("cmake --no-warn-unused-cli \
              \ -DCMAKE_BUILD_TYPE:STRING=$CMAKE_BUILD_TYPE \
              \ -DPICO_BOARD:STRING=$PICO_BOARD \
              \ -DWIFI_SSID:STRING=$WIFI_SSID \
              \ -DWIFI_PASSWORD:STRING=$WIFI_PASSWORD \
              \ -DTEST_TCP_SERVER_IP:STRING=$TEST_TCP_SERVER_IP \
              \ -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \
              \ -DCMAKE_C_COMPILER:FILEPATH=$CMAKE_C_COMPILER \
              \ -DCMAKE_CXX_COMPILER:FILEPATH=$CMAKE_CXX_COMPILER \
              \ -S$SOURCE_DIR -B$BUILD_DIR -G 'Unix Makefiles'\<CR>")
  "call feedkeys("exit\<CR>")
endfunction

function! LoadProgram()
    execute 'terminal'
    call feedkeys("source ./build_env.sh\<CR>")
    call feedkeys("sudo /usr/bin/cmake --build $BUILD_DIR --config Debug --target install -j 10 --\<CR>")
    call feedkeys("exit\<CR>")
endfunction

nnoremap <F5> :call RunCMake()<CR>
nnoremap <F6> :call LoadProgram()<CR>

