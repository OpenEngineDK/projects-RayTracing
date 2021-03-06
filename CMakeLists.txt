# Project name. Should be the same as the project directory name and
# will become the name of the binary target.
SET( PROJECT_NAME "RayTracing")

# Project source code list
SET( PROJECT_SOURCES
  # Add all the cpp source files here
  main.cpp
  Handlers.cpp
  SceneCreator.cpp
  Display/BoundedCamera.cpp
)

# Project executable
ADD_EXECUTABLE(${PROJECT_NAME}
  ${PROJECT_SOURCES}
)

# Project dependencies
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
  # Core library dependencies
  OpenEngine_Core
  OpenEngine_Logging
  OpenEngine_Display
  OpenEngine_Devices
  OpenEngine_Scene
  # Extension dependencies
  Extensions_SDL
  Extensions_OpenGLRenderer
  Extensions_PhotonMapping
  Extensions_GenericHandlers
  Extensions_AssimpResource
  Extensions_FreeImage
  Extensions_MeshUtils
  Extensions_AntTweakBar
  Extensions_Inspection
  Extensions_InspectionBar
)
