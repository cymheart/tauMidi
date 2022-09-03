# Install script for directory: G:/cymheart/tau/SDL2

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "G:/cymheart/tau/SDL2/out/install/x64-Debug (默认值)")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "G:/cymheart/tau/SDL2/out/build/x64-Debug (默认值)/SDL2-staticd.lib")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "G:/cymheart/tau/SDL2/out/build/x64-Debug (默认值)/SDL2d.lib")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "G:/cymheart/tau/SDL2/out/build/x64-Debug (默认值)/SDL2d.dll")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "G:/cymheart/tau/SDL2/out/build/x64-Debug (默认值)/SDL2maind.lib")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cmake/SDL2Targets.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cmake/SDL2Targets.cmake"
         "G:/cymheart/tau/SDL2/out/build/x64-Debug (默认值)/CMakeFiles/Export/cmake/SDL2Targets.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cmake/SDL2Targets-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cmake/SDL2Targets.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake" TYPE FILE FILES "G:/cymheart/tau/SDL2/out/build/x64-Debug (默认值)/CMakeFiles/Export/cmake/SDL2Targets.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake" TYPE FILE FILES "G:/cymheart/tau/SDL2/out/build/x64-Debug (默认值)/CMakeFiles/Export/cmake/SDL2Targets-debug.cmake")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake" TYPE FILE FILES
    "G:/cymheart/tau/SDL2/SDL2Config.cmake"
    "G:/cymheart/tau/SDL2/out/build/x64-Debug (默认值)/SDL2ConfigVersion.cmake"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/SDL2" TYPE FILE FILES
    "G:/cymheart/tau/SDL2/include/SDL.h"
    "G:/cymheart/tau/SDL2/include/SDL_assert.h"
    "G:/cymheart/tau/SDL2/include/SDL_atomic.h"
    "G:/cymheart/tau/SDL2/include/SDL_audio.h"
    "G:/cymheart/tau/SDL2/include/SDL_bits.h"
    "G:/cymheart/tau/SDL2/include/SDL_blendmode.h"
    "G:/cymheart/tau/SDL2/include/SDL_clipboard.h"
    "G:/cymheart/tau/SDL2/include/SDL_config_android.h"
    "G:/cymheart/tau/SDL2/include/SDL_config_iphoneos.h"
    "G:/cymheart/tau/SDL2/include/SDL_config_macosx.h"
    "G:/cymheart/tau/SDL2/include/SDL_config_minimal.h"
    "G:/cymheart/tau/SDL2/include/SDL_config_os2.h"
    "G:/cymheart/tau/SDL2/include/SDL_config_pandora.h"
    "G:/cymheart/tau/SDL2/include/SDL_config_psp.h"
    "G:/cymheart/tau/SDL2/include/SDL_config_windows.h"
    "G:/cymheart/tau/SDL2/include/SDL_config_winrt.h"
    "G:/cymheart/tau/SDL2/include/SDL_config_wiz.h"
    "G:/cymheart/tau/SDL2/include/SDL_copying.h"
    "G:/cymheart/tau/SDL2/include/SDL_cpuinfo.h"
    "G:/cymheart/tau/SDL2/include/SDL_egl.h"
    "G:/cymheart/tau/SDL2/include/SDL_endian.h"
    "G:/cymheart/tau/SDL2/include/SDL_error.h"
    "G:/cymheart/tau/SDL2/include/SDL_events.h"
    "G:/cymheart/tau/SDL2/include/SDL_filesystem.h"
    "G:/cymheart/tau/SDL2/include/SDL_gamecontroller.h"
    "G:/cymheart/tau/SDL2/include/SDL_gesture.h"
    "G:/cymheart/tau/SDL2/include/SDL_haptic.h"
    "G:/cymheart/tau/SDL2/include/SDL_hints.h"
    "G:/cymheart/tau/SDL2/include/SDL_joystick.h"
    "G:/cymheart/tau/SDL2/include/SDL_keyboard.h"
    "G:/cymheart/tau/SDL2/include/SDL_keycode.h"
    "G:/cymheart/tau/SDL2/include/SDL_loadso.h"
    "G:/cymheart/tau/SDL2/include/SDL_locale.h"
    "G:/cymheart/tau/SDL2/include/SDL_log.h"
    "G:/cymheart/tau/SDL2/include/SDL_main.h"
    "G:/cymheart/tau/SDL2/include/SDL_messagebox.h"
    "G:/cymheart/tau/SDL2/include/SDL_metal.h"
    "G:/cymheart/tau/SDL2/include/SDL_misc.h"
    "G:/cymheart/tau/SDL2/include/SDL_mouse.h"
    "G:/cymheart/tau/SDL2/include/SDL_mutex.h"
    "G:/cymheart/tau/SDL2/include/SDL_name.h"
    "G:/cymheart/tau/SDL2/include/SDL_opengl.h"
    "G:/cymheart/tau/SDL2/include/SDL_opengl_glext.h"
    "G:/cymheart/tau/SDL2/include/SDL_opengles.h"
    "G:/cymheart/tau/SDL2/include/SDL_opengles2.h"
    "G:/cymheart/tau/SDL2/include/SDL_opengles2_gl2.h"
    "G:/cymheart/tau/SDL2/include/SDL_opengles2_gl2ext.h"
    "G:/cymheart/tau/SDL2/include/SDL_opengles2_gl2platform.h"
    "G:/cymheart/tau/SDL2/include/SDL_opengles2_khrplatform.h"
    "G:/cymheart/tau/SDL2/include/SDL_pixels.h"
    "G:/cymheart/tau/SDL2/include/SDL_platform.h"
    "G:/cymheart/tau/SDL2/include/SDL_power.h"
    "G:/cymheart/tau/SDL2/include/SDL_quit.h"
    "G:/cymheart/tau/SDL2/include/SDL_rect.h"
    "G:/cymheart/tau/SDL2/include/SDL_render.h"
    "G:/cymheart/tau/SDL2/include/SDL_revision.h"
    "G:/cymheart/tau/SDL2/include/SDL_rwops.h"
    "G:/cymheart/tau/SDL2/include/SDL_scancode.h"
    "G:/cymheart/tau/SDL2/include/SDL_sensor.h"
    "G:/cymheart/tau/SDL2/include/SDL_shape.h"
    "G:/cymheart/tau/SDL2/include/SDL_stdinc.h"
    "G:/cymheart/tau/SDL2/include/SDL_surface.h"
    "G:/cymheart/tau/SDL2/include/SDL_system.h"
    "G:/cymheart/tau/SDL2/include/SDL_syswm.h"
    "G:/cymheart/tau/SDL2/include/SDL_test.h"
    "G:/cymheart/tau/SDL2/include/SDL_test_assert.h"
    "G:/cymheart/tau/SDL2/include/SDL_test_common.h"
    "G:/cymheart/tau/SDL2/include/SDL_test_compare.h"
    "G:/cymheart/tau/SDL2/include/SDL_test_crc32.h"
    "G:/cymheart/tau/SDL2/include/SDL_test_font.h"
    "G:/cymheart/tau/SDL2/include/SDL_test_fuzzer.h"
    "G:/cymheart/tau/SDL2/include/SDL_test_harness.h"
    "G:/cymheart/tau/SDL2/include/SDL_test_images.h"
    "G:/cymheart/tau/SDL2/include/SDL_test_log.h"
    "G:/cymheart/tau/SDL2/include/SDL_test_md5.h"
    "G:/cymheart/tau/SDL2/include/SDL_test_memory.h"
    "G:/cymheart/tau/SDL2/include/SDL_test_random.h"
    "G:/cymheart/tau/SDL2/include/SDL_thread.h"
    "G:/cymheart/tau/SDL2/include/SDL_timer.h"
    "G:/cymheart/tau/SDL2/include/SDL_touch.h"
    "G:/cymheart/tau/SDL2/include/SDL_types.h"
    "G:/cymheart/tau/SDL2/include/SDL_version.h"
    "G:/cymheart/tau/SDL2/include/SDL_video.h"
    "G:/cymheart/tau/SDL2/include/SDL_vulkan.h"
    "G:/cymheart/tau/SDL2/include/begin_code.h"
    "G:/cymheart/tau/SDL2/include/close_code.h"
    "G:/cymheart/tau/SDL2/out/build/x64-Debug (默认值)/include/SDL_config.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "G:/cymheart/tau/SDL2/out/build/x64-Debug (默认值)/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
