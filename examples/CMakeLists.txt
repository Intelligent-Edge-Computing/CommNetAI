build_lib_example(
  NAME interference-pattern
  SOURCE_FILES interference-pattern/mygym.cc
               interference-pattern/sim.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${libinternet}
    ${libopengym}
    ${libwifi}
)

build_lib_example(
  NAME linear-mesh
  SOURCE_FILES linear-mesh/sim.cc
  LIBRARIES_TO_LINK
    ${libapplications}
    ${libcore}
    ${libinternet}
    ${libopengym}
    ${libwifi}
)

build_lib_example(
  NAME linear-mesh-2
  SOURCE_FILES linear-mesh-2/sim.cc
               linear-mesh-2/mygym.cc
  LIBRARIES_TO_LINK
    ${libapplications}
    ${libcore}
    ${libinternet}
    ${libopengym}
    ${libwifi}
)

build_lib_example(
  NAME opengym
  SOURCE_FILES opengym/sim.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${libopengym}
)

build_lib_example(
  NAME opengym-2
  SOURCE_FILES opengym-2/sim.cc
               opengym-2/mygym.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${libopengym}
)

# 将 settings.json 复制到构建目录
configure_file(${CMAKE_SOURCE_DIR}/contrib/opengym/examples/aoi-agent/settings.json ${CMAKE_BINARY_DIR}/settings.json COPYONLY)
set(BUILD_DIR "${CMAKE_BINARY_DIR}")
# 生成 config.h 文件，并将构建目录路径写入其中
configure_file(
        ${CMAKE_SOURCE_DIR}/contrib/opengym/examples/aoi-agent/agent-config.h.in
        ${CMAKE_BINARY_DIR}/contrib/opengym/examples/aoi-agent/agent-config.h
)
build_lib_example(
  NAME aoi-agent
  SOURCE_FILES aoi-agent/sim.cc
        aoi-agent/ap/ap-app.cc
        aoi-agent/server/server-app.cc
        aoi-agent/user/user-app.cc
        aoi-agent/status-util.cc
        aoi-agent/logger.cc
        aoi-agent/fwd/fwd-service.cc
        aoi-agent/server/server-queue.cc
        aoi-agent/ap/ap-app.h
        aoi-agent/server/server-app.h
        aoi-agent/user/user-app.h
        aoi-agent/status-util.h
        aoi-agent/logger.h
        aoi-agent/json.hpp
        aoi-agent/fwd/fwd-service.h
        aoi-agent/event-time-tag.h
        aoi-agent/packet-type-tag.h
        aoi-agent/server/server-queue.h
  LIBRARIES_TO_LINK
    ${libcore}
    ${ns3-libs}
    ${ns3-contrib-libs}
    ${libopengym}
    ${libflow-monitor}
)
target_include_directories(aoi-agent PRIVATE ${CMAKE_BINARY_DIR}/contrib/opengym/examples/aoi-agent/)

build_lib_example(
  NAME rl-tcp
  SOURCE_FILES rl-tcp/sim.cc
               rl-tcp/tcp-rl-env.cc
               rl-tcp/tcp-rl.cc
  LIBRARIES_TO_LINK
    ${libapplications}
    ${libcore}
    ${libflow-monitor}
    ${libinternet}
    ${libopengym}
    ${libpoint-to-point-layout}
    ${libptp}
)
