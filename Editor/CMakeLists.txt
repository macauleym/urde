add_subdirectory(locale)
add_subdirectory(icons)
add_subdirectory(badging)

if(URDE_DLPACKAGE)
  set(METADATA_VERSION_STRING ${URDE_DLPACKAGE})
else()
  set(METADATA_VERSION_STRING "BETA")
endif()

unset(URDE_PLAT_LIBS)

if(WIN32)
  configure_file(platforms/win/urde.rc.in "${CMAKE_CURRENT_SOURCE_DIR}/platforms/win/urde.rc" @ONLY)
  set(PLAT_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/platforms/win/urde.rc" platforms/win/urde.manifest)
  if(WINDOWS_STORE)
    set(UWP_ASSETS
        platforms/win/Assets/LargeTile.scale-100.png
        platforms/win/Assets/LargeTile.scale-125.png
        platforms/win/Assets/LargeTile.scale-150.png
        platforms/win/Assets/LargeTile.scale-200.png
        platforms/win/Assets/LargeTile.scale-400.png
        platforms/win/Assets/SmallTile.scale-100.png
        platforms/win/Assets/SmallTile.scale-125.png
        platforms/win/Assets/SmallTile.scale-150.png
        platforms/win/Assets/SmallTile.scale-200.png
        platforms/win/Assets/SmallTile.scale-400.png
        platforms/win/Assets/SplashScreen.scale-100.png
        platforms/win/Assets/SplashScreen.scale-125.png
        platforms/win/Assets/SplashScreen.scale-150.png
        platforms/win/Assets/SplashScreen.scale-200.png
        platforms/win/Assets/SplashScreen.scale-400.png
        platforms/win/Assets/Square44x44Logo.scale-100.png
        platforms/win/Assets/Square44x44Logo.scale-125.png
        platforms/win/Assets/Square44x44Logo.scale-150.png
        platforms/win/Assets/Square44x44Logo.scale-200.png
        platforms/win/Assets/Square44x44Logo.scale-400.png
        platforms/win/Assets/Square44x44Logo.altform-unplated_targetsize-16.png
        platforms/win/Assets/Square44x44Logo.altform-unplated_targetsize-24.png
        platforms/win/Assets/Square44x44Logo.altform-unplated_targetsize-32.png
        platforms/win/Assets/Square44x44Logo.altform-unplated_targetsize-48.png
        platforms/win/Assets/Square44x44Logo.altform-unplated_targetsize-256.png
        platforms/win/Assets/Square150x150Logo.scale-100.png
        platforms/win/Assets/Square150x150Logo.scale-125.png
        platforms/win/Assets/Square150x150Logo.scale-150.png
        platforms/win/Assets/Square150x150Logo.scale-200.png
        platforms/win/Assets/Square150x150Logo.scale-400.png
        platforms/win/Assets/urde.scale-100.png
        platforms/win/Assets/urde.scale-125.png
        platforms/win/Assets/urde.scale-150.png
        platforms/win/Assets/urde.scale-200.png
        platforms/win/Assets/urde.scale-400.png
        platforms/win/Assets/WideTile.scale-100.png
        platforms/win/Assets/WideTile.scale-125.png
        platforms/win/Assets/WideTile.scale-150.png
        platforms/win/Assets/WideTile.scale-200.png
        platforms/win/Assets/WideTile.scale-400.png)
    set_property(SOURCE platforms/win/Package.appxmanifest PROPERTY VS_DEPLOYMENT_CONTENT 1)
    set_property(SOURCE ${UWP_ASSETS} PROPERTY VS_DEPLOYMENT_CONTENT 1)
    set_property(SOURCE ${UWP_ASSETS} PROPERTY VS_DEPLOYMENT_LOCATION "Assets")
    list(APPEND PLAT_SRCS ${UWP_ASSETS} platforms/win/Package.appxmanifest)
  endif()
elseif(APPLE)
  set(PLAT_SRCS platforms/mac/mainicon.icns)
  set_source_files_properties(platforms/mac/mainicon.icns PROPERTIES
                              MACOSX_PACKAGE_LOCATION Resources)
  configure_file(platforms/mac/Info.plist.in "${CMAKE_CURRENT_BINARY_DIR}/Info.plist" @ONLY)
elseif(UNIX)
  add_subdirectory(platforms/freedesktop)
  declare_wmicon_target()
  set(PLAT_SRCS mainicon_netwm.cpp)
  set(URDE_PLAT_LIBS rt)
endif()

add_executable(urde WIN32 MACOSX_BUNDLE
               main.cpp ${PLAT_SRCS}
               Space.hpp Space.cpp
               SplashScreen.hpp SplashScreen.cpp
               ResourceBrowser.hpp ResourceBrowser.cpp
               ModelViewer.hpp ModelViewer.cpp
               ParticleEditor.hpp ParticleEditor.cpp
               InformationCenter.hpp InformationCenter.hpp
               ProjectManager.hpp ProjectManager.cpp
               ViewManager.hpp ViewManager.cpp
               Resource.hpp Resource.cpp
               Camera.hpp Camera.cpp
               GameMode.hpp GameMode.cpp)
target_atdna(urde atdna_Space.cpp Space.hpp)
target_atdna(urde atdna_ResourceBrowser.cpp ResourceBrowser.hpp)
target_atdna(urde atdna_ModelViewer.cpp ModelViewer.hpp)
target_atdna(urde atdna_ParticleEditor.cpp ParticleEditor.hpp)
target_atdna(urde atdna_InformationCenter.cpp InformationCenter.hpp)
target_atdna(urde atdna_GameMode.cpp GameMode.hpp)

target_compile_definitions(urde PUBLIC URDE_DLPACKAGE="${URDE_DLPACKAGE}")

target_include_directories(urde PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR})

target_link_libraries(urde
                      UrdeIcons
                      UrdeBadging
                      RuntimeCommon
                      ${URDE_PLAT_LIBS})
if(COMMAND add_sanitizers)
  add_sanitizers(urde)
endif()

set_target_properties(urde PROPERTIES
                      MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist"
                      MACOSX_BUNDLE_BUNDLE_NAME "urde")

if (NOT WINDOWS_STORE)
  if(APPLE)
    add_custom_command(TARGET urde POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:visigen> $<TARGET_FILE_DIR:urde>)
    add_custom_command(TARGET urde POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:hecl> $<TARGET_FILE_DIR:urde>)
  endif()
  add_dependencies(urde visigen hecl)
else()
  set_property(TARGET urde PROPERTY VS_WINRT_COMPONENT TRUE)
  # This should match the Package.appxmanifest
  set_property(TARGET urde PROPERTY VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION "10.0.14393.0")
endif()
